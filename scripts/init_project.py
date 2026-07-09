#!/usr/bin/env python3
"""Stamp a complete, buildable plugin project around a vendored cs2-kit.

Run from an empty repo root that vendors the kit at vendor/cs2-kit:

    mkdir my-cs2-plugins && cd my-cs2-plugins
    git init
    git submodule add https://github.com/suxrobGM/cs2-kit.git vendor/cs2-kit
    git submodule update --init --recursive
    python vendor/cs2-kit/scripts/init_project.py --plugin my-plugin
    uv sync
    uv run poe build

Generates the root CMakeLists.txt, CMakePresets.json (includes the kit's presets),
conanfile.py, pyproject.toml (poe tasks call the kit's scripts directly),
.gitignore, .clang-format, and README.md from templates/project/, rendered with
string.Template.safe_substitute ($project only, so CMake/preset `${...}` syntax
passes through). The first plugin is then scaffolded via new_plugin.py.
"""

import argparse
import string
from pathlib import Path

import new_plugin

REPO_ROOT = Path.cwd()
TEMPLATE_DIR = Path(__file__).resolve().parent.parent / "templates" / "project"
SUBMODULE_HINT = (
    "    git submodule add https://github.com/suxrobGM/cs2-kit.git vendor/cs2-kit\n"
    "    git submodule update --init --recursive"
)


def render_project(name: str) -> None:
    for template in sorted(TEMPLATE_DIR.rglob("*")):
        if not template.is_file():
            continue
        rel = template.relative_to(TEMPLATE_DIR)
        content = string.Template(template.read_text(encoding="utf-8")).safe_substitute(
            {"project": name}
        )
        out = REPO_ROOT / rel
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(content, encoding="utf-8", newline="\n")
        print(f"  created {rel.as_posix()}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Initialize a plugin project around a vendored cs2-kit."
    )
    parser.add_argument(
        "--name",
        default=REPO_ROOT.name,
        help="kebab-case project name (default: current directory name)",
    )
    parser.add_argument(
        "--plugin",
        default="my-plugin",
        help="kebab-case name for the first plugin (default: my-plugin)",
    )
    args = parser.parse_args()

    for flag, value in (("--name", args.name), ("--plugin", args.plugin)):
        if not new_plugin.NAME_RE.match(value):
            print(f"error: {flag} '{value}' is not kebab-case (expected e.g. 'fun-votes').")
            return 2

    if Path(__file__).resolve() != (REPO_ROOT / "vendor/cs2-kit/scripts/init_project.py").resolve():
        print(
            "error: cs2-kit must be vendored at vendor/cs2-kit under the current\n"
            "directory. From your project root, run:\n" + SUBMODULE_HINT
        )
        return 1

    for existing in ("CMakeLists.txt", "pyproject.toml"):
        if (REPO_ROOT / existing).exists():
            print(f"error: {existing} already exists in {REPO_ROOT}; refusing to overwrite.")
            return 1

    plugin_dir = REPO_ROOT / "plugins" / args.plugin
    if plugin_dir.exists():
        print(f"error: {plugin_dir} already exists; refusing to overwrite.")
        return 1

    if not TEMPLATE_DIR.is_dir():
        print(f"error: template tree missing at {TEMPLATE_DIR}.")
        return 1

    sdk_dir = REPO_ROOT / "vendor/cs2-kit/vendor/hl2sdk-cs2"
    if not sdk_dir.is_dir() or not any(sdk_dir.iterdir()):
        print(
            "note: the kit's SDK submodules are not initialized yet; the build (or\n"
            "`uv run poe bootstrap`) needs: git submodule update --init --recursive"
        )

    render_project(args.name)
    new_plugin.render_tree(args.plugin, plugin_dir)
    if new_plugin.insert_subdirectory(REPO_ROOT / "CMakeLists.txt", args.plugin):
        print(f"  registered add_subdirectory(plugins/{args.plugin}) in CMakeLists.txt")

    print(
        "\nDone. Next steps:\n"
        "  uv sync           # provision CMake/Conan/Ninja (https://docs.astral.sh/uv)\n"
        "  uv run poe build  # conan install + cmake workflow preset\n"
        "Without uv: python vendor/cs2-kit/scripts/bootstrap.py"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
