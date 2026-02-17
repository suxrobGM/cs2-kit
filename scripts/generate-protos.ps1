#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"

# This script generates protobuf files for the HL2SDK
Set-Location -Path (Split-Path -Parent $PSScriptRoot)

$HL2SDKCS2 = Join-Path (Get-Location) "vendor\hl2sdk-cs2"

if (-not (Test-Path $HL2SDKCS2)) {
    Write-Error "HL2SDK not found at $HL2SDKCS2. Run 'git submodule update --init --recursive' first."
    exit 1
}

$Protoc = Join-Path $HL2SDKCS2 "devtools\bin\protoc.exe"
$ProtoPathCommon = Join-Path $HL2SDKCS2 "common"
$ProtoPathGameShared = Join-Path $HL2SDKCS2 "game\shared"
$ProtoPathGoogle = Join-Path $HL2SDKCS2 "thirdparty\protobuf-3.21.8\src"
$OutputDir = Join-Path $HL2SDKCS2 "public"

if (-not (Test-Path $Protoc)) {
    Write-Error "protoc not found at $Protoc"
    exit 1
}

Write-Host "Generating protobuf files for hl2sdk-cs2..."
Write-Host "SDK path: $HL2SDKCS2"

# Generate common/*.proto -> public/
Write-Host "Generating common/*.proto..."
$ProtoFiles = Get-ChildItem -Path $ProtoPathCommon -Filter "*.proto"

foreach ($ProtoFile in $ProtoFiles) {
    Write-Host "  $($ProtoFile.Name)"
    & $Protoc `
        --proto_path="$ProtoPathCommon" `
        --proto_path="$ProtoPathGoogle" `
        --cpp_out="$OutputDir" `
        $ProtoFile.FullName
}

# Generate game/shared/*.proto -> game/shared/
Write-Host "Generating game/shared/*.proto..."
$GameProtoFiles = Get-ChildItem -Path $ProtoPathGameShared -Filter "*.proto"

foreach ($ProtoFile in $GameProtoFiles) {
    Write-Host "  $($ProtoFile.Name)"
    & $Protoc `
        --proto_path="$ProtoPathCommon" `
        --proto_path="$ProtoPathGameShared" `
        --proto_path="$ProtoPathGoogle" `
        --cpp_out="$ProtoPathGameShared" `
        $ProtoFile.FullName
}

Write-Host "Done! Generated files are in $OutputDir and $ProtoPathGameShared"
