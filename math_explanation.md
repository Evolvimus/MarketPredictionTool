# Mathematische und Logische Erklärung der Handels-Analyse (Refactored)

Diese Datei beschreibt die **optimierte** Logik in `src/analysis.cpp`. Wir haben rohe Indikatoren durch normalisierte State-Vektoren ersetzt, um Korrelationen zu reduzieren und die ML-Performance zu steigern.

## 1. Feature Engineering: Normalisierung & States

Wir füttern das ML-Modell nicht mehr mit Indikator-"Wald", sondern mit kompakten Zustandsvariablen im Bereich $[-1, 1]$.

### A. Normalisierung
Jeder Indikator wird skaliert:
- **RSI Norm:** `(RSI - 50) / 50`  $\rightarrow \in [-1, 1]$
- **MACD Hist Norm:** `clamp( (MACD - Signal) / Close * 100, -1, 1 )`
- **ROC Norm:** `clamp( ROC_20 / 5, -1, 1 )`
- **SMA Dist Norm:** `clamp( SMA_Distance% / 10, -1, 1 )`

### B. Market State Vectors (Zustandsvektoren)
Indikatoren werden logisch gruppiert:

1.  **Momentum State** (Stärke der Bewegung)
    $$S_{mom} = \frac{RSI_{norm} + ROC_{norm} + MACD_{HistNorm}}{3}$$
    - $> 0.5$: Starker Bullischer Druck
    - $< -0.5$: Starker Bärischer Druck

2.  **Trend State** (Richtung & Stabilität)
    $$S_{trend} = \frac{SMA_{DistNorm} + (ADX_{norm} \times \text{sign}(S_{mom})) + HTF_{Align}}{3}$$
    - Kombiniert Abstand zum SMA, Trendstärke (ADX) und Multi-Timeframe-Richtung.

3.  **Volatility State** (Marktruhe vs. Chaos)
    $$S_{vol} = \frac{ATR_{rel\_norm} + BollingerWidth_{norm}}{2}$$
    - Hohe Werte (> 0.7) deuten auf Ausbruch oder Crash-Gefahr hin.

## 2. MACD Korrektur
Der MACD besitzt nun eine korrekte Signal-Linie (9-Perioden EMA des MACD):
$$MACD = EMA_{12}(Close) - EMA_{26}(Close)$$
$$Signal = EMA_{9}(MACD)$$
$$Hist = MACD - Signal$$

## 3. Machine Learning: Expected Value (EV)

Das Python-Skript (`ml_predict.py`) berechnet nun den **Erwartungswert (EV)** eines Trades, nicht nur die Wahrscheinlichkeit.

1.  **Probability ($P_{win}$):** Kalibrierte Wahrscheinlichkeit aus dem Richtungs-Score (basierend auf Momentum & Trend State).
2.  **Reward:Risk ($R$):** Abhängig vom Regime:
    - *Trend:* R = 2.5
    - *Range:* R = 1.5
    - *High Vol:* R = 3.0
3.  **Expected Value ($EV$):**
    $$EV = (P_{win} \times R) - ((1 - P_{win}) \times 1)$$

**Filter-Logik:**
Ein Trade wird nur akzeptiert, wenn $EV > 0.2$ (in `src/analysis.cpp` gefiltert). Dies filtert mathematisch unrentable Trades trotz "guter" Richtung aus.

## 4. Risiko-Management: Volatilitäts-Bias

Die Stop-Loss (SL) und Take-Profit (TP) Levels passen sich der Volatilität (`S_vol`) an:

- **Hohe Volatilität ($S_{vol} > 0.7$):**
  - SL weiter weg ($1.5 \times ATR$) um Rauschen zu überleben.
  - TP aggressiver ($2.0-3.0 \times ATR$).
- **Niedrige Volatilität ($S_{vol} < 0.3$):**
  - SL enger ($0.8 \times ATR$).
  - TP optimiert für Range-Ausbruch.

## 5. Markt-Regime Klassifizierung

1.  **High Volatility:** Wenn $S_{vol} > 0.7$
2.  **Trend:** Wenn $|S_{trend}| > 0.4$
3.  **Range:** Sonst.
