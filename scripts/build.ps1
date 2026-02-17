#!/usr/bin/env pwsh
Set-Location -Path (Split-Path -Parent $PSScriptRoot)

# Configure with AMBuild (creates objdir folder)
pdm run python configure.py

# Build from the objdir folder
Set-Location -Path "objdir"
pdm run ambuild

Set-Location -Path (Split-Path -Parent $PSScriptRoot)

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "=== Build Complete ===" -ForegroundColor Green
Write-Host "Static library: objdir/src/cs2-kit.lib"
