#!/bin/bash
set -e

echo "Building predict_app..."

clang++ -std=c++20 \
    src/main.cpp \
    src/market_data.cpp \
    src/analysis.cpp \
    src/ollama_client.cpp \
    -o predict_app \
    -I src \
    -lcurl

if [ $? -eq 0 ]; then
    echo "Build successful! Run ./predict_app to start."
else
    echo "Build failed."
fi
