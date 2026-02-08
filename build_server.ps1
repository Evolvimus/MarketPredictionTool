# Windows PowerShell Build Script
Write-Host "Building predict_server for Windows..." -ForegroundColor Cyan

# Check if using MSVC or MinGW
$hasMSVC = Get-Command cl -ErrorAction SilentlyContinue
$hasMinGW = Get-Command g++ -ErrorAction SilentlyContinue

if ($hasMSVC) {
    Write-Host "Using MSVC compiler..." -ForegroundColor Green
    cl /std:c++20 /EHsc /I src /I vcpkg\installed\x64-windows\include `
       src\server.cpp src\market_data.cpp src\analysis.cpp src\ollama_client.cpp `
       /Fe:predict_server.exe `
       /link vcpkg\installed\x64-windows\lib\libcurl.lib Ws2_32.lib
} elseif ($hasMinGW) {
    Write-Host "Using MinGW compiler..." -ForegroundColor Green
    g++ -std=c++20 `
        src/server.cpp src/market_data.cpp src/analysis.cpp src/ollama_client.cpp `
        -o predict_server.exe `
        -I src -lcurl -lws2_32 -lpthread
} else {
    Write-Host "Error: No C++ compiler found (MSVC or MinGW required)" -ForegroundColor Red
    exit 1
}

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Build successful! Run .\predict_server.exe to start." -ForegroundColor Green
} else {
    Write-Host "✗ Build failed." -ForegroundColor Red
    exit 1
}
