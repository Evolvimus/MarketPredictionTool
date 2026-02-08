#pragma once
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

struct Candle {
  std::string timestamp; // Keep as string for simplicity initially, or time_t
  double open;
  double high;
  double low;
  double close;
  long long volume;
};

class MarketData {
public:
  static std::vector<Candle> fetch_history(const std::string &ticker,
                                           const std::string &interval = "1d");
};
