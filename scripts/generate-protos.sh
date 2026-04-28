#!/usr/bin/env bash
set -euo pipefail

# This script generates protobuf files for the HL2SDK
cd "$(dirname "$0")/.."

HL2SDKCS2="$(pwd)/vendor/hl2sdk-cs2"

if [[ ! -d "$HL2SDKCS2" ]]; then
    echo "HL2SDK not found at $HL2SDKCS2. Run 'git submodule update --init --recursive' first." >&2
    exit 1
fi

# Use protoc.exe under Git Bash / WSL on Windows; fall back to protoc on Linux
Protoc="$HL2SDKCS2/devtools/bin/protoc.exe"
if [[ ! -f "$Protoc" ]]; then
    Protoc="$HL2SDKCS2/devtools/bin/protoc"
fi

ProtoPathCommon="$HL2SDKCS2/common"
ProtoPathGameShared="$HL2SDKCS2/game/shared"
ProtoPathGcSdk="$HL2SDKCS2/gcsdk"
ProtoPathGoogle="$HL2SDKCS2/thirdparty/protobuf-3.21.8/src"
OutputDir="$HL2SDKCS2/public"

if [[ ! -f "$Protoc" ]]; then
    echo "protoc not found at $Protoc" >&2
    exit 1
fi

echo "Generating protobuf files for hl2sdk-cs2..."
echo "SDK path: $HL2SDKCS2"

# Generate common/*.proto -> public/
echo "Generating common/*.proto..."
for ProtoFile in "$ProtoPathCommon"/*.proto; do
    [[ -f "$ProtoFile" ]] || continue
    echo "  $(basename "$ProtoFile")"
    "$Protoc" \
        --proto_path="$ProtoPathCommon" \
        --proto_path="$ProtoPathGoogle" \
        --cpp_out="$OutputDir" \
        "$ProtoFile"
done

# Generate game/shared/*.proto -> game/shared/
# game/shared/base_gcmessages.proto pulls in gcsdk/steammessages.proto, so include gcsdk on the path.
echo "Generating game/shared/*.proto..."
for ProtoFile in "$ProtoPathGameShared"/*.proto; do
    [[ -f "$ProtoFile" ]] || continue
    echo "  $(basename "$ProtoFile")"
    "$Protoc" \
        --proto_path="$ProtoPathCommon" \
        --proto_path="$ProtoPathGameShared" \
        --proto_path="$ProtoPathGcSdk" \
        --proto_path="$ProtoPathGoogle" \
        --cpp_out="$ProtoPathGameShared" \
        "$ProtoFile"
done

echo "Done! Generated files are in $OutputDir and $ProtoPathGameShared"
