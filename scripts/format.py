#!/usr/bin/env python3
"""Format C++ sources with clang-format. Usage: format.py [--check] [dirs...]

Targets the working directory. Default dirs are cs2-kit's own sources when run
from the kit root, else plugins/ (the consumer-repo layout); pass explicit dirs
to override.
"""

import sys
from pathlib import Path

import buildtools

ROOT = Path.cwd()
KIT_ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    args = sys.argv[1:]
    check = "--check" in args
    dirs = [a for a in args if a != "--check"]
    if not dirs:
        dirs = ["src", "include", "tests"] if ROOT == KIT_ROOT else ["plugins"]
    buildtools.format_sources(ROOT, dirs, check=check)


if __name__ == "__main__":
    main()
