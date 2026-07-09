#!/usr/bin/env python3
"""One-command setup: fetch submodules, check build tools, then build.

Targets the working directory: python vendor/cs2-kit/scripts/bootstrap.py
"""

import subprocess
from pathlib import Path

import buildtools

ROOT = Path.cwd()


def main() -> None:
    """Fetch submodules (cs2-kit + nested SDKs), verify tools, build."""
    print("==> [1/3] Fetching submodules (cs2-kit + nested SDKs)")
    subprocess.run(
        ["git", "submodule", "update", "--init", "--recursive", "--depth", "1"],
        cwd=ROOT,
        check=True,
    )

    print("==> [2/3] Checking CMake + Conan build tools")
    buildtools.require_build_tools()

    print("==> [3/3] Building with Conan + CMake")
    buildtools.build(ROOT, buildtools.default_preset())

    print(
        "\n============================================================\n"
        "  Bootstrap complete - the plugins built successfully.\n"
        "  Output: build/<preset>/plugins/\n"
        "============================================================"
    )


if __name__ == "__main__":
    main()
