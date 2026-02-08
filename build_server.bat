@echo off
echo Building predict_server for Windows...

REM Check if using MSVC or MinGW
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using MSVC compiler...
    cl /std:c++20 /EHsc /I src /I vcpkg\installed\x64-windows\include ^
       src\server.cpp src\market_data.cpp src\analysis.cpp src\ollama_client.cpp ^
       /Fe:predict_server.exe ^
       /link vcpkg\installed\x64-windows\lib\libcurl.lib Ws2_32.lib
) else (
    echo Using MinGW compiler...
    g++ -std=c++20 ^
        src/server.cpp src/market_data.cpp src/analysis.cpp src/ollama_client.cpp ^
        -o predict_server.exe ^
        -I src -lcurl -lws2_32 -lpthread
)

if %ERRORLEVEL% EQU 0 (
    echo ✓ Build successful! Run predict_server.exe to start.
) else (
    echo ✗ Build failed.
    exit /b 1
)
