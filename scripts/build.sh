#!/usr/bin/env bash
set -euo pipefail

ScriptDir="$(cd "$(dirname "$0")" && pwd)"
RepoRoot="$(cd "$ScriptDir/.." && pwd)"
cd "$RepoRoot"

source "$ScriptDir/lib/common.sh"

Preset="${1:-}"
if [[ -z "$Preset" ]]; then
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*) Preset="windows-msvc-release" ;;
        *) Preset="linux-steamrt-release" ;;
    esac
fi

require_build_tools

case "$Preset" in
    *debug*) BuildType="Debug" ;;
    *) BuildType="Release" ;;
esac

# Static profiles (conan/profiles/); build_type and MSVC runtime_type are passed
# per build below, so one profile per OS serves Release+Debug.
ProfileDir="$RepoRoot/conan/profiles"
ExtraSettings=(-s "build_type=$BuildType")
case "$Preset" in
    linux-*)   Profile="$ProfileDir/linux-steamrt.txt" ;;
    windows-*)
        Profile="$ProfileDir/windows-msvc.txt"
        ExtraSettings+=(-s "compiler.runtime_type=$BuildType")
        ;;
    *) echo "Unknown preset: $Preset" >&2; exit 1 ;;
esac

BuildDir="$RepoRoot/build/$Preset"

run_tool conan install "$RepoRoot" \
    --output-folder "$BuildDir/generators" \
    --build=missing \
    --lockfile "$RepoRoot/conan.lock" \
    --profile:host "$Profile" \
    --profile:build "$Profile" \
    "${ExtraSettings[@]}"

run_tool cmake --preset "$Preset"
run_tool cmake --build --preset "$Preset"

echo
echo "=== Build Complete ==="
echo "Preset: $Preset"
echo "Build directory: build/$Preset"
