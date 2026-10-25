$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$destination = Join-Path $root 'dist/CompGod-1.0.0-Windows-x64'
New-Item -ItemType Directory -Force $destination | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'build/CompGod_artefacts/Release/VST3/CompGod.vst3') -Destination $destination -Recurse -Force
Copy-Item -LiteralPath (Join-Path $root 'build/CompGod_artefacts/Release/Standalone/CompGod.exe') -Destination $destination -Force
foreach ($name in @('README.md', 'INSTALL.txt', 'CMakeLists.txt', 'Source', 'Tests', 'scripts', 'docs')) {
    Copy-Item -LiteralPath (Join-Path $root $name) -Destination $destination -Recurse -Force
}
$juceLicense = Join-Path $root 'build/_deps/juce-src/LICENSE.md'
if (!(Test-Path -LiteralPath $juceLicense)) {
    $cache = Get-Content -LiteralPath (Join-Path $root 'build/CMakeCache.txt')
    $entry = $cache | Where-Object { $_ -match '^FETCHCONTENT_SOURCE_DIR_JUCE:PATH=' } | Select-Object -First 1
    if ($entry) { $juceLicense = Join-Path ($entry -replace '^FETCHCONTENT_SOURCE_DIR_JUCE:PATH=', '') 'LICENSE.md' }
}
if (Test-Path -LiteralPath $juceLicense) { Copy-Item -LiteralPath $juceLicense -Destination (Join-Path $destination 'JUCE-LICENSE.md') -Force }
Compress-Archive -LiteralPath $destination -DestinationPath (Join-Path $root 'dist/CompGod-1.0.0-Windows-x64.zip') -Force
Get-Item -LiteralPath (Join-Path $root 'dist/CompGod-1.0.0-Windows-x64.zip') | Select-Object FullName, Length
