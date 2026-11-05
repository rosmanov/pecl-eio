#!/usr/bin/env python3

from pathlib import Path
import re
import sys

if len(sys.argv) != 3:
    raise SystemExit("Usage: check-changelog.py <check|extract> <version>")

action, version = sys.argv[1:]
if action not in ("check", "extract"):
    raise SystemExit(f"Unknown action: {action}")

changelog = Path("CHANGELOG.md").read_text()

pattern = (
    rf"^## {re.escape(version)}(?:[ \t][^\n]*)?\r?\n"
    rf"(.*?)(?=^## |\Z)"
)

match = re.search(pattern, changelog, re.MULTILINE | re.DOTALL)

if not match or not match.group(1).strip():
    raise SystemExit(f"Missing or empty changelog entry for {version}")

if action == "extract":
    print(match.group(1).strip())
