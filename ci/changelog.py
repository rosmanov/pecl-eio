#!/usr/bin/env python3

from pathlib import Path
import re
import sys


def section(version, path="CHANGELOG.md"):
    """Returns the stripped body of the "## <version>" section, or None if it is missing or empty."""
    changelog = Path(path).read_text()

    pattern = (
        rf"^## {re.escape(version)}(?:[ \t][^\n]*)?\r?\n"
        rf"(.*?)(?=^## |\Z)"
    )

    match = re.search(pattern, changelog, re.MULTILINE | re.DOTALL)

    if not match or not match.group(1).strip():
        return None

    return match.group(1).strip()


def main():
    if len(sys.argv) != 3:
        raise SystemExit("Usage: check-changelog.py <check|extract> <version>")

    action, version = sys.argv[1:]
    if action not in ("check", "extract"):
        raise SystemExit(f"Unknown action: {action}")

    body = section(version)
    if body is None:
        raise SystemExit(f"Missing or empty changelog entry for {version}")

    if action == "extract":
        print(body)


if __name__ == "__main__":
    main()
