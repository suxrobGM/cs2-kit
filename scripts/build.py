#!/usr/bin/env python3
"""Build the current repo with Conan + CMake. Usage: build.py [preset]

Targets the working directory, so it serves cs2-kit itself and any repo that
vendors it: python vendor/cs2-kit/scripts/build.py [preset]
"""

import sys
from pathlib import Path

import buildtools

ROOT = Path.cwd()


def main(argv: list[str] | None = None) -> None:
    """Build the requested preset (default: release for this OS)."""
    args = sys.argv[1:] if argv is None else argv
    preset = args[0] if args else buildtools.default_preset()
    buildtools.build(ROOT, preset, workflow=True)


if __name__ == "__main__":
    main()
