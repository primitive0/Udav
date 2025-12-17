#!/usr/bin/env python3
import sys
import argparse
import re


# Allowed case name: [A-Za-z0-9_]+
CASE_RE = re.compile(r"^#=== CASE:\s*([A-Za-z0-9_]+)\s*===$")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Select a named CASE block from a Udav lit test file"
    )
    parser.add_argument("file", help="Input file ('-' for stdin)")
    parser.add_argument(
        "-c", "--case",
        required=True,
        help="Case name to select (alnum + underscore)"
    )
    args = parser.parse_args()

    if args.file == "-":
        lines = sys.stdin.readlines()
    else:
        with open(args.file, "r", encoding="utf-8") as f:
            lines = f.readlines()

    collecting = False
    found = False

    for line in lines:
        m = CASE_RE.match(line)
        if m:
            case_name = m.group(1)
            if collecting:
                break
            collecting = (case_name == args.case)
            found |= collecting
            continue

        if collecting:
            sys.stdout.write(line)

    if not found:
        sys.stderr.write(f"error: case '{args.case}' not found\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
