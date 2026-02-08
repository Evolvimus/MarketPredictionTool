# 📈 AI Trading Analyst

Eine moderne C++ Trading-Analyse-Anwendung mit AI-Integration (Ollama LLM) und Web-UI.

## 🎯 Features

- **Echtzeit-Marktdaten** via Yahoo Finance API
- **Technische Indikatoren**: RSI, MACD, SMA (50/200), ATR
- **AI-gestützte Prognosen** mit lokalen Ollama-Modellen
- **Automatische Trading-Level**: Entry, Take Profit, Stop Loss
- **Moderne Web-UI** mit Dark Theme & Glassmorphism
- **Asset-Schnellauswahl**: AAPL, Gold, EUR/USD, NASDAQ, S&P 500, Crude Oil

## 📋 Voraussetzungen

### macOS

#### 1. Ollama Installation
```bash
# macOS (Homebrew)
brew install ollama

# Oder von https://ollama.ai herunterladen
```

#### 2. Ollama-Modell herunterladen
```bash
# Llama 3 (empfohlen, ~2GB)
ollama pull llama3

# Optional: GPT-OSS 20B (~13GB)
ollama pull gpt-oss:20b
```

#### 3. System-Requirements
- **macOS** mit clang++ (bereits vorinstalliert)
- **libcurl** (bereits vorinstalliert auf macOS)

### Windows

#### 1. Ollama Installation
```powershell
# Windows: Download von https://ollama.ai/download/windows
# Installiere die .exe und starte Ollama Desktop App
```

#### 2. C++ Compiler & Dependencies

**Option A: Visual Studio (empfohlen)**
```powershell
# 1. Visual Studio 2022 Community installieren
# https://visualstudio.microsoft.com/downloads/
# Wähle "Desktop development with C++"

# 2. vcpkg installieren (für libcurl)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl:x64-windows
```

**Option B: MSYS2/MinGW (leichtgewichtig)**
```powershell
# 1. MSYS2 von https://www.msys2.org/ installieren
# 2. In MSYS2 Terminal:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-curl
```

#### 3. Ollama-Modell herunterladen
```powershell
# In PowerShell oder CMD
ollama pull llama3
ollama pull gpt-oss:20b  # optional
```


## 🚀 Schnellstart

### macOS

#### 1. Ollama Server starten
```bash
# In einem separaten Terminal
ollama serve
```

#### 2. Web-Server bauen und starten
```bash
# Im predict-Verzeichnis
./build_server.sh
./predict_server
```

#### 3. Browser öffnen
Navigiere zu: **http://localhost:8080**

### Windows

#### 1. Ollama starten
```powershell
# Ollama Desktop App ist bereits im Hintergrund aktiv
# Falls nicht: Starte die Ollama App aus dem Startmenü
```

#### 2. Build-Script erstellen & ausführen

**PowerShell-Script erstellen (`build_server.ps1`):**
```powershell
# Für MSVC (Visual Studio)
cl /std:c++20 /EHsc /I src /I vcpkg\installed\x64-windows\include `
   src\server.cpp src\market_data.cpp src\analysis.cpp src\ollama_client.cpp `
   /Fe:predict_server.exe `
   /link vcpkg\installed\x64-windows\lib\libcurl.lib Ws2_32.lib

# Für MinGW
g++ -std=c++20 `
    src/server.cpp src/market_data.cpp src/analysis.cpp src/ollama_client.cpp `
    -o predict_server.exe `
    -I src -lcurl -lws2_32 -lpthread
```

**Server starten:**
```powershell
.\predict_server.exe
```

#### 3. Browser öffnen
Navigiere zu: **http://localhost:8080**


## 💡 Nutzung

1. **Symbol wählen**:
   - Klicke auf einen der Schnellauswahl-Buttons (🥇 Gold, 💶 EUR/USD, etc.)
   - Oder gib ein eigenes Yahoo Finance Symbol ein

2. **AI-Modell wählen**:
   - `llama3` (schnell, ~5-10 Sek)
   - `gpt-oss:20b` (langsamer, detaillierter)

3. **"Analyse Starten" klicken**

4. **Ergebnisse ansehen**:
   - Entry, Take Profit, Stop Loss
   - RSI mit Signalen (Oversold/Overbought)
   - MACD mit Momentum-Interpretation
   - SMA 50/200 mit Golden/Death Cross
   - Interaktive Candlestick-Charts
   - AI-Prognose

## 🛠️ Manuelle Builds

### macOS

#### Web-Server neu kompilieren
```bash
./build_server.sh
```

#### CLI-Version (ohne UI)
```bash
./build.sh
./predict_app AAPL llama3
```

### Windows

#### Web-Server kompilieren

**MSVC (Visual Studio):**
```powershell
cl /std:c++20 /EHsc /I src /I vcpkg\installed\x64-windows\include `
   src\server.cpp src\market_data.cpp src\analysis.cpp src\ollama_client.cpp `
   /Fe:predict_server.exe `
   /link vcpkg\installed\x64-windows\lib\libcurl.lib Ws2_32.lib
```

**MinGW:**
```bash
g++ -std=c++20 src/server.cpp src/market_data.cpp src/analysis.cpp src/ollama_client.cpp -o predict_server.exe -I src -lcurl -lws2_32 -lpthread
```

#### CLI-Version (ohne UI)

**MSVC:**
```powershell
cl /std:c++20 /EHsc /I src /I vcpkg\installed\x64-windows\include `
   src\main.cpp src\market_data.cpp src\analysis.cpp src\ollama_client.cpp `
   /Fe:predict_app.exe `
   /link vcpkg\installed\x64-windows\lib\libcurl.lib Ws2_32.lib

.\predict_app.exe AAPL llama3
```

**MinGW:**
```bash
g++ -std=c++20 src/main.cpp src/market_data.cpp src/analysis.cpp src/ollama_client.cpp -o predict_app.exe -I src -lcurl -lws2_32 -lpthread
./predict_app.exe AAPL llama3
```


## 📊 Unterstützte Symbole (Yahoo Finance)

| Asset | Yahoo Symbol | Beschreibung |
|-------|--------------|--------------|
| 🍎 AAPL | `AAPL` | Apple Stock |
| 🥇 Gold | `GC=F` | Gold Futures (XAUUSD) |
| 💶 EUR/USD | `EURUSD=X` | Forex Pair |
| 📊 NASDAQ | `^IXIC` | NASDAQ Composite |
| 📈 S&P 500 | `^GSPC` | S&P 500 Index |
| 🛢️ Oil | `CL=F` | Crude Oil WTI |
| 🛡️ LMT | `LMT` | Lockheed Martin |
| 🚀 RTX | `RTX` | RTX Corporation |
| ✈️ NOC | `NOC` | Northrop Grumman |
| 🚜 GD | `GD` | General Dynamics |
| 📡 LHX | `LHX` | L3Harris Technologies |
| 🇩🇪 RHM | `RHM.DE` | Rheinmetall AG |
| ⚪ Platin | `PL=F` | Platinum Futures |

## 🔧 Troubleshooting

### "Model not found" Fehler
```bash
# Stelle sicher, dass Ollama läuft
ollama serve

# Prüfe installierte Modelle
ollama list

# Modell herunterladen
ollama pull llama3
```

### Port 8080 bereits belegt
```bash
# Finde laufende Prozesse auf Port 8080
lsof -i :8080

# Server stoppen mit Ctrl+C und neu starten
```

### Build-Fehler
```bash
# Stelle sicher, dass alle Source-Dateien vorhanden sind
ls src/

# Sollte zeigen:
# - server.cpp, main.cpp
# - market_data.cpp/hpp
# - analysis.cpp/hpp
# - ollama_client.cpp/hpp
# - httplib.h, nlohmann/json.hpp
```

## 📂 Projekt-Struktur

```
predict/
├── README.md
├── build.sh              # CLI Build
├── build_server.sh       # Web-Server Build
├── predict_app           # CLI Binary
├── predict_server        # Web-Server Binary
├── src/
│   ├── main.cpp          # CLI Entry
│   ├── server.cpp        # HTTP Server
│   ├── market_data.*     # Yahoo Finance Integration
│   ├── analysis.*        # Technical Indicators
│   ├── ollama_client.*   # AI Integration
│   ├── httplib.h         # HTTP Library
│   └── nlohmann/json.hpp # JSON Parser
└── public/
    ├── index.html        # Web UI
    ├── style.css         # Styling
    └── app.js            # Frontend Logic
```

## 🤖 Technische Details

### Trading-Level-Berechnung
- **ATR (Average True Range)**: 14-Perioden für Volatilität
- **Take Profit**: 2x ATR vom Entry
- **Stop Loss**: 1x ATR vom Entry
- **Richtung**: Basierend auf RSI (Oversold/Overbought) oder MACD

### Indikatoren
- **RSI**: 14-Periode, Oversold < 30, Overbought > 70
- **MACD**: EMA 12/26, Signal Line approximiert
- **SMA**: 50 & 200 Tage für Trend-Erkennung
- **Golden Cross**: SMA 50 > SMA 200 (Bullish)
- **Death Cross**: SMA 50 < SMA 200 (Bearish)

## 📝 Lizenz

Dieses Projekt dient zu Bildungszwecken. Keine Anlageberatung!

---

**Made with C++20, libcurl, Ollama & ❤️**
