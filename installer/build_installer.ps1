param(
    [switch]$SkipBuild,
    [switch]$SkipZip
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root "build-release"
$distRoot = Join-Path $root "dist"
$appDir = Join-Path $distRoot "RawImageProcess"
$qtBin = "C:\Qt\6.7.3\msvc2019_64\bin"
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
$toolchain = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
$cudaArchitectures = "75;80;86;89;120"

$cmake = "cmake"
if (Test-Path "C:\Program Files\CMake\bin\cmake.exe") { $cmake = "C:\Program Files\CMake\bin\cmake.exe" }

$appVersion = "1.0"
$versionMatch = Select-String -Path (Join-Path $root "gui.cpp") -Pattern 'kAppVersion = "([^"]+)"' | Select-Object -First 1
if ($versionMatch) { $appVersion = $versionMatch.Matches[0].Groups[1].Value }

Write-Host "== Raw Image Process $appVersion =="

function Stop-PackagedApp {
    Get-Process cuda_raw_gui -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -and ($_.Path.StartsWith($appDir) -or $_.Path.StartsWith($buildDir)) } |
        Stop-Process -Force
}

if (-not $SkipBuild) {

    Stop-PackagedApp
    New-Item -ItemType Directory -Force $buildDir | Out-Null

    $batch = Join-Path $buildDir "build_release.bat"

    @(
        "@echo off",
        "call `"$vcvars`" >nul",
        "`"$cmake`" -S `"$root`" -B `"$buildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$toolchain `"-DCMAKE_CUDA_ARCHITECTURES=$cudaArchitectures`" || exit /b 1",
        "`"$cmake`" --build `"$buildDir`" --target cuda_raw_gui || exit /b 1"
    ) | Set-Content -Path $batch -Encoding Default

    & cmd /c "`"$batch`""
    if ($LASTEXITCODE -ne 0) { throw "Release build failed" }
}

$exe = Join-Path $buildDir "cuda_raw_gui.exe"
if (-not (Test-Path $exe)) { throw "cuda_raw_gui.exe not found in $buildDir" }

Write-Host "== Collecting files into $appDir =="

Stop-PackagedApp
if (Test-Path $appDir) { Remove-Item $appDir -Recurse -Force }
New-Item -ItemType Directory -Force $appDir | Out-Null

Copy-Item $exe $appDir
Get-ChildItem $buildDir -Filter *.dll | Copy-Item -Destination $appDir

& (Join-Path $qtBin "windeployqt.exe") --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime --dir $appDir (Join-Path $appDir "cuda_raw_gui.exe")
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }

$nvccLine = Select-String -Path (Join-Path $buildDir "CMakeCache.txt") -Pattern '^CMAKE_CUDA_COMPILER:FILEPATH=(.+)$' | Select-Object -First 1
$cudaRoot = Split-Path -Parent (Split-Path -Parent $nvccLine.Matches[0].Groups[1].Value)
$cudart = Get-ChildItem (Join-Path $cudaRoot "bin") -Recurse -Filter "cudart64_*.dll" | Select-Object -First 1
if (-not $cudart) { throw "cudart64_*.dll not found under $cudaRoot" }
Copy-Item $cudart.FullName $appDir

$crt = Get-Item "C:\Program Files*\Microsoft Visual Studio\2022\*\VC\Redist\MSVC\*\x64\Microsoft.VC143.CRT" -ErrorAction SilentlyContinue |
    Where-Object { $_.Parent.Parent.Name -match '^\d+(\.\d+)+$' } |
    Sort-Object { [version]$_.Parent.Parent.Name } -Descending |
    Select-Object -First 1
if (-not $crt) { throw "Microsoft.VC143.CRT folder not found" }
Copy-Item (Join-Path $crt.FullName "*.dll") $appDir

$sizeMb = [math]::Round(((Get-ChildItem $appDir -Recurse | Measure-Object Length -Sum).Sum / 1MB), 1)
Write-Host "Program folder ready: $appDir ($sizeMb MB)"

if (-not $SkipZip) {
    $zip = Join-Path $distRoot "RawImageProcess-$appVersion-portable.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path $appDir -DestinationPath $zip
    Write-Host "Portable zip: $zip"
}

$iscc = $null
$command = Get-Command iscc.exe -ErrorAction SilentlyContinue
if ($command) { $iscc = $command.Source }

foreach ($candidate in @("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe", "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe")) {
    if (-not $iscc -and (Test-Path $candidate)) { $iscc = $candidate }
}

if (-not $iscc) {
    Write-Host ""
    Write-Host "Inno Setup not found, Setup.exe was not built. Install it and run this script again:"
    Write-Host "    winget install --id JRSoftware.InnoSetup -e"
    return
}

Write-Host "== Building installer with $iscc =="

& $iscc "/DAppVersion=$appVersion" "/DSourceDir=$appDir" "/DOutputDir=$distRoot" (Join-Path $PSScriptRoot "RawImageProcess.iss")
if ($LASTEXITCODE -ne 0) { throw "Inno Setup compilation failed" }

Write-Host ""
Write-Host "Installer ready: $(Join-Path $distRoot "RawImageProcess-$appVersion-Setup.exe")"
