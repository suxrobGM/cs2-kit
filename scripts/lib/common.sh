#!/usr/bin/env bash

# Canonical shared build helpers (cs2-kit + projects that vendor it, e.g.
# cs2-plugins). Source, don't execute. Callers must set RepoRoot before run_tool.

# Run a build tool, preferring the project virtualenv, then `uv run`, then PATH.
run_tool() {
    local tool="$1"
    shift
    local venv
    local candidate
    local venv_roots=()

    if [[ -n "${VIRTUAL_ENV:-}" ]]; then
        venv_roots+=("$VIRTUAL_ENV")
    fi
    venv_roots+=("$RepoRoot/.venv")

    for venv in "${venv_roots[@]}"; do
        case "$(uname -s)" in
            MINGW*|MSYS*|CYGWIN*)
                for candidate in "$venv/Scripts/${tool}.exe" "$venv/Scripts/$tool"; do
                    if [[ -x "$candidate" ]]; then
                        PATH="$venv/Scripts:$PATH" "$candidate" "$@"
                        return
                    fi
                done
                ;;
            *)
                candidate="$venv/bin/$tool"
                if [[ -x "$candidate" ]]; then
                    PATH="$venv/bin:$PATH" "$candidate" "$@"
                    return
                fi
                ;;
        esac
    done

    if command -v uv >/dev/null 2>&1; then
        uv run --project "$RepoRoot" "$tool" "$@"
    elif command -v "$tool" >/dev/null 2>&1; then
        "$tool" "$@"
    else
        echo "ERROR: '$tool' was not found on PATH." >&2
        echo "Install CMake 4.3.4+, Conan 2.29.1+, and Ninja, or install uv and run uv sync." >&2
        exit 1
    fi
}

# True when $1 (actual version) is >= $2 (minimum version).
version_ge() {
    local actual="$1"
    local minimum="$2"
    [[ "$(printf '%s\n%s\n' "$minimum" "$actual" | sort -V | head -n 1)" == "$minimum" ]]
}

# Exit with an error unless $3 (actual version) satisfies the $2 minimum for tool $1.
require_min_tool_version() {
    local tool="$1"
    local minimum="$2"
    local actual="$3"

    if [[ -z "$actual" ]] || ! version_ge "$actual" "$minimum"; then
        echo "ERROR: $tool $minimum or newer is required, found ${actual:-unknown}." >&2
        exit 1
    fi
}

# Verify Ninja is present and CMake/Conan meet the project minimums.
require_build_tools() {
    run_tool ninja --version >/dev/null
    require_min_tool_version "CMake" "4.3.4" \
        "$(run_tool cmake --version | sed -n 's/cmake version \([0-9.]*\).*/\1/p' | head -n 1)"
    require_min_tool_version "Conan" "2.29.1" \
        "$(run_tool conan --version | sed -n 's/Conan version \([0-9.]*\).*/\1/p' | head -n 1)"
}
