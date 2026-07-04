#!/usr/bin/env python3
"""Format cs2-kit C++ sources with clang-format. Usage: format.py [--check]"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import buildtools  # noqa: E402

DIRS = ["src", "include", "tests"]


def main() -> None:
    buildtools.format_sources(ROOT, DIRS, check="--check" in sys.argv[1:])


if __name__ == "__main__":
    main()
