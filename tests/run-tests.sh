#!/usr/bin/env bash
#
# Standalone unit-test runner for the SDK-free CS2Kit utilities.
#
# This build is fully independent of CMake / Conan / the plugin build.
# It compiles only the pure-logic utilities (StringUtils, SteamId, TimeUtils) plus
# the test cases - NO HL2SDK, NO Metamod, NO libpqxx, NO curl.
#
# IMPORTANT: with MSVC (cl) this script assumes it is run from an
# "x64 Native Tools Command Prompt for VS" / vcvars environment (run via Git Bash
# launched from that shell, or after sourcing vcvars). If `cl` is not on PATH it
# falls back to g++.
#
# Usage:
#   tests/run-tests.sh
#
set -euo pipefail

ScriptDir="$(cd "$(dirname "$0")" && pwd)"
RepoRoot="$(cd "$ScriptDir/.." && pwd)"
IncludeDir="$RepoRoot/include"
KitSrc="$RepoRoot/src/Utils"
OutDir="$ScriptDir/build"
mkdir -p "$OutDir"

# Test cases + the harness main.
TestSources=(
    "$ScriptDir/MicroTestMain.cpp"
    "$ScriptDir/ParseDurationTests.cpp"
    "$ScriptDir/StringUtilsTests.cpp"
    "$ScriptDir/SteamIdTests.cpp"
    "$ScriptDir/TimeUtilsTests.cpp"
)

# Only the SDK-free CS2Kit translation units.
KitSources=(
    "$KitSrc/StringUtils.cpp"
    "$KitSrc/SteamId.cpp"
    "$KitSrc/TimeUtils.cpp"
)

if command -v cl >/dev/null 2>&1; then
    echo ">> Compiling with MSVC cl"
    ExeOut="$OutDir/run_tests.exe"
    # When invoked from Git Bash (MSYS), file/dir args need Windows-style paths
    # (cygpath -w), while cl's slash-style flags (/nologo, /I..., /Fe...) must be
    # left alone. MSYS2_ARG_CONV_EXCL='/' tells MSYS not to rewrite tokens that
    # start with a single '/', i.e. the flags, while we pre-convert the paths.
    WinInclude="$(cygpath -w "$IncludeDir")"
    WinScriptDir="$(cygpath -w "$ScriptDir")"
    WinExeOut="$(cygpath -w "$ExeOut")"
    WinOutDir="$(cygpath -w "$OutDir")\\"
    WinSources=()
    for s in "${TestSources[@]}" "${KitSources[@]}"; do
        WinSources+=("$(cygpath -w "$s")")
    done
    # /EHsc for C++ exceptions, /std:c++latest for C++23, /I for the CS2Kit headers.
    MSYS2_ARG_CONV_EXCL='/' \
        cl /nologo /std:c++latest /EHsc /W3 /utf-8 \
        "/I${WinInclude}" "/I${WinScriptDir}" \
        "${WinSources[@]}" \
        "/Fe${WinExeOut}" "/Fo${WinOutDir}"
    echo ">> Running $ExeOut"
    "$ExeOut"
elif command -v g++ >/dev/null 2>&1; then
    echo ">> Compiling with g++"
    ExeOut="$OutDir/run_tests"
    g++ -std=c++23 -Wall -I"$IncludeDir" -I"$ScriptDir" \
        "${TestSources[@]}" "${KitSources[@]}" -o "$ExeOut"
    echo ">> Running $ExeOut"
    "$ExeOut"
else
    echo "ERROR: no C++ compiler found (need cl or g++)." >&2
    echo "       For cl, run from an x64 Native Tools / vcvars environment." >&2
    exit 1
fi
