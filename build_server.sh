#!/bin/bash
set -e

echo "🔨 Building Trading Analysis Server..."

g++ -std=c++20 \
    src/server.cpp \
    src/market_data.cpp \
    src/analysis.cpp \
    src/ollama_client.cpp \
    src/analysis_storage.cpp \
    src/news_fetcher.cpp \
    src/settings_storage.cpp \
    -o predict_server \
    -I src \
    -lcurl \
    -lpthread

if [ $? -eq 0 ]; then
    echo "✅ Build complete! Run with: ./predict_server"
else
    echo "❌ Build failed."
fi
