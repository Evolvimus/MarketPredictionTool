#include "analysis.hpp"
#include "market_data.hpp"
#include "ollama_client.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  std::string ticker = "AAPL";
  if (argc > 1) {
    ticker = argv[1];
  }

  std::string model = "llama3";
  if (argc > 2) {
    model = argv[2];
  }

  std::cout << "=== AI Trading Analyst C++ ===" << std::endl;
  std::cout << "Target: " << ticker << std::endl;

  // 1. Fetch Data
  auto candles = MarketData::fetch_history(ticker);
  if (candles.empty()) {
    std::cerr << "No data found for " << ticker << std::endl;
    return 1;
  }
  std::cout << "Fetched " << candles.size() << " candles." << std::endl;

  // 2. Technical Analysis
  auto indicators = TechnicalAnalysis::calculate_indicators(candles);
  std::string summary =
      TechnicalAnalysis::get_market_summary(candles, indicators);

  std::cout << "\n--- Market Summary ---\n" << summary << std::endl;

  // 3. AI Prediction
  OllamaClient ai(model);
  std::string prediction = ai.get_market_analysis(ticker, summary);

  std::cout << "\n--- AI Prediction ---\n" << prediction << std::endl;

  return 0;
}
