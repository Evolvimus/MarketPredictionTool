#include "analysis.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
  std::cout << "Starting Test Analysis..." << std::endl;

  // Generate dummy data (Sine wave + noise to simulate market)
  std::vector<Candle> candles;
  int data_points = 200;
  double price = 100.0;
  for (int i = 0; i < data_points; ++i) {
    Candle c;
    c.timestamp = "2023-01-01 10:00:00";
    double trend = i * 0.05; // upward trend
    double noise = std::sin(i * 0.1) * 2.0;

    c.open = price + noise + trend;
    c.close =
        price + noise + trend + ((rand() % 100) / 100.0 - 0.5); // small move
    c.high = std::max(c.open, c.close) + 1.0;
    c.low = std::min(c.open, c.close) - 1.0;
    c.volume = 1000 + (rand() % 500);

    candles.push_back(c);
    price = c.close; // next open approx close
  }

  // Call Analysis
  std::cout << "Data generated. Running analysis..." << std::endl;
  AnalysisResult res = TechnicalAnalysis::calculate_indicators(candles);

  // Print Results
  std::cout << "--- Analysis Results ---" << std::endl;
  std::cout << "Regime: " << res.regime_info.regime
            << " (Conf: " << res.regime_info.confidence << ")" << std::endl;
  std::cout << "Direction: " << res.ml_info.direction
            << " (Prob: " << res.ml_info.probability << ")" << std::endl;
  std::cout << "Expected Value: " << res.expected_value << std::endl;
  std::cout << "Signal Strength: " << res.signal_strength << std::endl;

  std::cout << "--- State Vectors ---" << std::endl;
  std::cout << "Momentum: " << res.momentum_state << std::endl;
  std::cout << "Trend: " << res.trend_state << std::endl;
  std::cout << "Volatility: " << res.volatility_state << std::endl;

  if (res.regime_info.regime == "unknown") {
    std::cerr << "Test Failed: ML output parsing failed or script error."
              << std::endl;
    return 1;
  }

  std::cout << "Test Passed!" << std::endl;
  return 0;
}
