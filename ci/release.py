#!/usr/bin/env python3
"""Keeps the release metadata in sync. CHANGELOG.md is the source of the release notes.

  release.py check [--tag <tag>]
      Verifies that package.xml, php8/php_eio.h and CHANGELOG.md agree with each other.

  release.py sync <version> [--date YYYY-MM-DD] [--api <version>] [--stability <name>]
      Writes <version> and the CHANGELOG.md notes into package.xml and php8/php_eio.h.
      The date is kept when <version> is already the current one, otherwise it is today.
"""

import argparse
import datetime
import re
import sys
from pathlib import Path
from xml.dom import Node, minidom

import changelog

ROOT = Path(__file__).resolve().parent.parent
PACKAGE_XML = ROOT / "package.xml"
HEADER = ROOT / "php8" / "php_eio.h"
CHANGELOG = ROOT / "CHANGELOG.md"

VERSION_RE = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+(RC[0-9]+)?$")
HEADER_RE = re.compile(r'^(#\s*define\s+PHP_EIO_VERSION\s+)"([^"]*)"', re.MULTILINE)


# FOLD_START Markdown notes

def bullets(text):
    """Splits a bullet list into whitespace-normalized items; continuation lines join the previous item."""
    items = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith("- "):
            items.append(stripped[2:].strip())
        elif items:
            items[-1] += " " + stripped
        else:
            raise ValueError(f"Expected a '- ' bullet, got: {stripped!r}")
    return [" ".join(item.split()) for item in items]


def plain(item):
    """Strips the Markdown inline syntax that package.xml notes can't render."""
    item = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r"\1 (\2)", item)
    return item.replace("`", "")


def changelog_notes(version):
    body = changelog.section(version, CHANGELOG)
    if body is None:
        raise SystemExit(f"Missing or empty changelog entry for {version} in CHANGELOG.md")
    return [plain(item) for item in bullets(body)]

# FOLD_END

# FOLD_START package.xml

def children(node, tag):
    return [c for c in node.childNodes if c.nodeType == Node.ELEMENT_NODE and c.tagName == tag]


def child(node, *path):
    for tag in path:
        found = children(node, tag)
        if len(found) != 1:
            raise SystemExit(f"package.xml: expected exactly one <{tag}> in <{node.tagName}>")
        node = found[0]
    return node


def text_of(node):
    return "".join(
        c.data for c in node.childNodes if c.nodeType in (Node.TEXT_NODE, Node.CDATA_SECTION_NODE)
    )


def set_text(node, value):
    for c in list(node.childNodes):
        node.removeChild(c)
    node.appendChild(node.ownerDocument.createTextNode(value))


def set_notes(notes, items, indent):
    pad = " " * indent
    lines = [f"{pad}    - {item}" for item in items]
    set_text(notes, "\n" + "\n".join(lines) + f"\n{pad}")


def load_package():
    return minidom.parse(str(PACKAGE_XML))


def save_package(doc):
    # The root element is serialized on its own to get a newline after the declaration,
    # and the nodes around it (e.g. the trailing modeline comment) are kept.
    parts = ['<?xml version="1.0" encoding="UTF-8"?>']
    parts += [n.toxml() for n in doc.childNodes]
    PACKAGE_XML.write_text("\n".join(parts) + "\n", encoding="utf-8")


def package_version(doc):
    return text_of(child(doc.documentElement, "version", "release")).strip()


def package_notes(doc):
    return bullets(text_of(child(doc.documentElement, "notes")))


def find_changelog_release(doc, version):
    for release in children(child(doc.documentElement, "changelog"), "release"):
        if text_of(child(release, "version", "release")).strip() == version:
            return release
    return None


def add_changelog_release(doc, version):
    """Clones the newest <changelog> entry (with its fold comments) in front of itself."""
    log = child(doc.documentElement, "changelog")
    first = next(c for c in log.childNodes if c.nodeType == Node.ELEMENT_NODE)
    release = first.cloneNode(True)
    ws = lambda: doc.createTextNode("\n    ")

    anchor = first
    before = first.previousSibling
    while before is not None and before.nodeType == Node.TEXT_NODE and not before.data.strip():
        before = before.previousSibling
    if before is not None and before.nodeType == Node.COMMENT_NODE:
        anchor = before  # the fold comment opening the newest entry

    for node in (
        doc.createComment(f"{{{{{{ {version} "),
        ws(),
        release,
        ws(),
        doc.createComment("}}}"),
        ws(),
    ):
        log.insertBefore(node, anchor)

    return release

# FOLD_END

# FOLD_START php_eio.h

def header_version():
    match = HEADER_RE.search(HEADER.read_text())
    return match.group(2) if match else None


def set_header_version(version):
    text, count = HEADER_RE.subn(lambda m: f'{m.group(1)}"{version}"', HEADER.read_text())
    if count != 1:
        raise SystemExit(f"{HEADER}: PHP_EIO_VERSION definition not found")
    HEADER.write_text(text)

# FOLD_END


# check action handler
def check(tag):
    doc = load_package()
    version = package_version(doc)
    errors = []

    if tag is not None and tag != version:
        errors.append(f"tag {tag} does not match the package.xml version {version}")

    if header_version() != version:
        errors.append(f"PHP_EIO_VERSION is {header_version()}, package.xml says {version}")

    expected = None
    try:
        expected = changelog_notes(version)
    except SystemExit as e:
        errors.append(str(e))

    if expected is not None:
        if package_notes(doc) != expected:
            errors.append("package.xml <notes> differ from CHANGELOG.md")

        release = find_changelog_release(doc, version)
        if release is None:
            errors.append(f"package.xml <changelog> has no entry for {version}")
        elif bullets(text_of(child(release, "notes"))) != expected:
            errors.append(f"package.xml <changelog> entry for {version} differs from CHANGELOG.md")

    for error in errors:
        print(f"error: {error}", file=sys.stderr)
    if errors:
        print("Run: ci/release.py sync " + version, file=sys.stderr)
        raise SystemExit(1)

    print(f"OK: {version}")

# sync action handler
def sync(version, date, api, stability):
    if not VERSION_RE.match(version):
        raise SystemExit(f"Invalid release version: {version}")

    items = changelog_notes(version)
    doc = load_package()
    root = doc.documentElement

    if date is None:
        if package_version(doc) == version:
            date = text_of(child(root, "date")).strip()
        else:
            date = datetime.date.today().isoformat()

    set_text(child(root, "date"), date)
    set_text(child(root, "version", "release"), version)
    if api:
        set_text(child(root, "version", "api"), api)
    if stability:
        set_text(child(root, "stability", "release"), stability)
    set_notes(child(root, "notes"), items, indent=2)

    release = find_changelog_release(doc, version) or add_changelog_release(doc, version)
    set_text(child(release, "version", "release"), version)
    set_text(child(release, "version", "api"), text_of(child(root, "version", "api")).strip())
    set_text(child(release, "stability", "release"), text_of(child(root, "stability", "release")).strip())
    set_text(child(release, "stability", "api"), text_of(child(root, "stability", "api")).strip())
    set_notes(child(release, "notes"), items, indent=6)

    save_package(doc)
    set_header_version(version)
    print(f"Synced {version} ({date}): package.xml, {HEADER.relative_to(ROOT)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="action", required=True)

    p = sub.add_parser("check")
    p.add_argument("--tag")

    p = sub.add_parser("sync")
    p.add_argument("version")
    p.add_argument("--date")
    p.add_argument("--api")
    p.add_argument("--stability")

    args = parser.parse_args()
    if args.action == "check":
        check(args.tag)
    else:
        sync(args.version, args.date, args.api, args.stability)


if __name__ == "__main__":
    main()

# vim: foldmethod=marker fmr=FOLD_START,FOLD_END:
# vim: ts=4 sw=4 et
