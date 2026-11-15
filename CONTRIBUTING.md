# Contributing

## Release metadata

The version and release notes live in three files:

- `CHANGELOG.md` is the source of truth for the release notes. Edit it by hand.
- `package.xml` holds the version, date, notes, and the `<changelog>` history.
- `php8/php_eio.h` holds `PHP_EIO_VERSION`.

Don't edit the version or notes in `package.xml` or `php_eio.h` directly. `ci/release.py` generates them from `CHANGELOG.md`:

```bash
ci/release.py sync <version> [--date YYYY-MM-DD] [--api <version>] [--stability <name>]
ci/release.py check [--tag <tag>]
```

- `sync` writes the version and the notes of the `## <version>` section of `CHANGELOG.md` into `package.xml` and `php_eio.h`. The date is kept if `<version>` is already the current one, otherwise it is today. Use `--date` to override it. Use `--api` if the API version changes, and `--stability` for pre-releases.
- `check` verifies that the three files agree. CI runs it on every push and pull request, and the release workflow runs it with `--tag` before publishing.

Notes in `CHANGELOG.md` are a Markdown bullet list (`- ...`). Backticks are stripped and `[text](url)` becomes `text (url)` in `package.xml`.

## Workflow

1. Commit changes to `master` as usual. Changes that don't touch the changelog don't need any metadata update.

2. To bump the version without releasing it yet:
   1. Add a `## <new_version>` section to `CHANGELOG.md`.
   2. Run `ci/release.py sync <new_version>`.
   3. Commit `CHANGELOG.md`, `package.xml`, and `php8/php_eio.h`, and push `master`.

   From then on, every bullet you add under that version's header needs another `ci/release.py sync <new_version>`, otherwise the CI check fails.

3. To publish a release:
   1. Add or update the `## <new_version>` section in `CHANGELOG.md` with the final list of changes.
   2. Run `ci/release.py sync <new_version> --date $(date +%F)`. Passing `--date` sets the release day, since `sync` keeps the date from an earlier bump.
   3. Commit the generated files and let CI pass on `master`.
   4. Create a tag that matches the version exactly, for example `4.1.0`, and push it together with `master`:

      ```
      git tag <new_version>
      git push origin master <new_version>
      ```

The `Release` workflow then runs `ci/release.py check --tag <new_version>` and publishes a GitHub release with the notes from `CHANGELOG.md`. It checks out `master`, so the commit with the generated files has to be on `master` before the tag is pushed. If the workflow fails because the changelog entry is missing, add it, merge to `master`, and re-run the workflow with `workflow_dispatch` and the version.
