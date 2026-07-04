"""Stamp a new plugin skeleton under plugins/<name>/ in the consuming repo.

Usage (from the consuming repo's root): uv run poe new-plugin <name>

<name> is kebab-case (e.g. "fun-votes"). The generated plugin builds as-is: it loads
settings.jsonc, installs a permissive policy, and answers !ping. The repo's root
CMakeLists.txt gets its add_subdirectory() line inserted automatically.

Templates live in cs2-kit's templates/plugin/ tree, mirroring the output layout;
file contents are rendered with string.Template ($name, $ns, $klass, $title, $tag).
The script targets the current working directory, so any repo that vendors the kit
can expose it as a task, e.g.:

    new-plugin = "python vendor/cs2-kit/scripts/new_plugin.py"
"""

import re
import string
import sys
from pathlib import Path

REPO_ROOT = Path.cwd()
TEMPLATE_DIR = Path(__file__).resolve().parent.parent / "templates" / "plugin"

NAME_RE = re.compile(r"^[a-z][a-z0-9]*(-[a-z0-9]+)*$")


def pascal_case(name: str) -> str:
    return "".join(part.capitalize() for part in name.split("-"))


def substitutions(name: str) -> dict[str, str]:
    pascal = pascal_case(name)
    return {
        "name": name,                                              # fun-votes
        "ns": pascal,                                              # FunVotes
        "klass": f"{pascal}Plugin",                                # FunVotesPlugin
        "title": " ".join(p.capitalize() for p in name.split("-")),  # Fun Votes
        "tag": pascal.upper()[:12],                                # FUNVOTES
    }


def render_tree(name: str, plugin_dir: Path) -> None:
    subs = substitutions(name)
    for template in sorted(TEMPLATE_DIR.rglob("*")):
        if not template.is_file():
            continue
        rel = template.relative_to(TEMPLATE_DIR)
        content = string.Template(template.read_text(encoding="utf-8")).substitute(subs)
        out = plugin_dir / rel
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(content, encoding="utf-8", newline="\n")
        print(f"  created plugins/{name}/{rel.as_posix()}")


def insert_subdirectory(root_cmake: Path, name: str) -> bool:
    """Add add_subdirectory(plugins/<name>) after the last plugin add_subdirectory."""
    line = f"add_subdirectory(plugins/{name})"
    text = root_cmake.read_text(encoding="utf-8")
    if line in text:
        return False

    lines = text.splitlines(keepends=True)
    last = max(
        (i for i, ln in enumerate(lines) if ln.strip().startswith("add_subdirectory(plugins/")),
        default=len(lines) - 1,
    )
    lines.insert(last + 1, line + "\n")
    root_cmake.write_text("".join(lines), encoding="utf-8", newline="\n")
    return True


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2

    name = sys.argv[1]
    if not NAME_RE.match(name):
        print(f"error: '{name}' is not kebab-case (expected e.g. 'fun-votes').")
        return 2

    if not TEMPLATE_DIR.is_dir():
        print(f"error: template tree missing at {TEMPLATE_DIR}.")
        return 1

    if not (REPO_ROOT / "CMakeLists.txt").is_file():
        print(f"error: no CMakeLists.txt in {REPO_ROOT}; run from your repo's root.")
        return 1

    plugin_dir = REPO_ROOT / "plugins" / name
    if plugin_dir.exists():
        print(f"error: {plugin_dir} already exists; refusing to overwrite.")
        return 1

    render_tree(name, plugin_dir)

    if insert_subdirectory(REPO_ROOT / "CMakeLists.txt", name):
        print(f"  registered add_subdirectory(plugins/{name}) in CMakeLists.txt")

    print("\nDone. Build it with: uv run poe build")
    return 0


if __name__ == "__main__":
    sys.exit(main())
