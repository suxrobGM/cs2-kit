#!/usr/bin/env bash
set -euo pipefail

ScriptDir="$(cd "$(dirname "$0")" && pwd)"
RepoRoot="$(cd "$ScriptDir/.." && pwd)"
cd "$RepoRoot"

Preset="${1:-}"
if [[ -z "$Preset" ]]; then
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*) Preset="windows-msvc-release" ;;
        *) Preset="linux-steamrt-release" ;;
    esac
fi

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

version_ge() {
    local actual="$1"
    local minimum="$2"
    [[ "$(printf '%s\n%s\n' "$minimum" "$actual" | sort -V | head -n 1)" == "$minimum" ]]
}

require_min_tool_version() {
    local tool="$1"
    local minimum="$2"
    local actual="$3"

    if [[ -z "$actual" ]] || ! version_ge "$actual" "$minimum"; then
        echo "ERROR: $tool $minimum or newer is required, found ${actual:-unknown}." >&2
        exit 1
    fi
}

run_tool ninja --version >/dev/null

require_min_tool_version "CMake" "4.3.4" "$(run_tool cmake --version | sed -n 's/cmake version \([0-9.]*\).*/\1/p' | head -n 1)"
require_min_tool_version "Conan" "2.29.1" "$(run_tool conan --version | sed -n 's/Conan version \([0-9.]*\).*/\1/p' | head -n 1)"

case "$Preset" in
    *debug*) BuildType="Debug" ;;
    *) BuildType="Release" ;;
esac

ProfileDir="$RepoRoot/build/conan-profiles"
Profile="$ProfileDir/$Preset"
mkdir -p "$ProfileDir"

write_linux_profile() {
    local clang_bin="${CC:-clang}"
    local clangxx_bin="${CXX:-clang++}"
    local clang_version
    clang_version="$("$clang_bin" --version | sed -n 's/.*version \([0-9][0-9]*\).*/\1/p' | head -n 1)"
    [[ -n "$clang_version" ]] || clang_version="17"

    cat > "$Profile" <<EOF
[settings]
os=Linux
arch=x86_64
compiler=clang
compiler.version=$clang_version
compiler.libcxx=libstdc++
compiler.cppstd=23
build_type=$BuildType

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
tools.build:compiler_executables={"c": "$clang_bin", "cpp": "$clangxx_bin"}
EOF
}

write_windows_profile() {
    local msvc_version="193"
    if command -v cl >/dev/null 2>&1; then
        local cl_version
        cl_version="$(cl 2>&1 | sed -n 's/.*Version 19\.\([0-9][0-9]\).*/\1/p' | head -n 1)"
        if [[ -n "$cl_version" ]]; then
            msvc_version="19${cl_version:0:1}"
        fi
    fi

    local runtime_type="$BuildType"
    cat > "$Profile" <<EOF
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=$msvc_version
compiler.runtime=static
compiler.runtime_type=$runtime_type
compiler.cppstd=23
build_type=$BuildType

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
EOF
}

case "$Preset" in
    linux-*) write_linux_profile ;;
    windows-*) write_windows_profile ;;
    *) echo "Unknown preset: $Preset" >&2; exit 1 ;;
esac

BuildDir="$RepoRoot/build/$Preset"

run_tool conan install "$RepoRoot" \
    --output-folder "$BuildDir/generators" \
    --build=missing \
    --profile:host "$Profile" \
    --profile:build "$Profile"

run_tool cmake --preset "$Preset"
run_tool cmake --build --preset "$Preset"

echo
echo "=== Build Complete ==="
echo "Preset: $Preset"
echo "Build directory: build/$Preset"
