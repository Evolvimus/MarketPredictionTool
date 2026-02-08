#pragma once
#include "market_data.hpp"
#include "nlohmann/json.hpp"
#include <vector>

struct MarketRegime {
  std::string regime; // trend, range, high_vol, event_risk
  double confidence;
};

struct StateVector {
  double x; // Momentum/Direction [-1, 1]
  double y; // Trend/Regime [-1, 1]
  double z; // Volatility [0, 1]
  std::string timestamp;
};

struct MLPrediction {
  std::string direction; // long, short, neutral
  double probability;
  double expected_r;
};

struct AnalysisResult {
  double current_rsi;
  double macd;
  double macd_signal;
  double sma_50;
  double sma_200;
  double entry_price;
  double take_profit;
  double stop_loss;

  // New features for 3-model setup
  double adx;
  double boll_upper;
  double boll_lower;
  double boll_width;
  double atr_median;
  double volume_z_score;
  double roc_5;
  double roc_10;
  double roc_20;
  double obv;
  double vwap_dist;
  double sma_distance_pct;
  double range_pos; // 0-1
  bool is_stock;

  // Multi-Timeframe (MTF) features
  double htf_rsi;
  double htf_sma_50;
  double htf_sma_200;

  // Advanced Risk Management
  double trailing_sl;
  double partial_tp;

  MarketRegime regime_info;
  MLPrediction ml_info;

  // Normalized Features (ML-ready)
  double rsi_norm;
  double macd_norm;
  double macd_hist_norm;
  double roc_norm;
  double adx_norm;
  double sma_dist_norm;
  double bollinger_width_norm;
  double volume_z_norm;

  // Market State Vectors
  double momentum_state;   // Combined RSI, ROC, MACD
  double trend_state;      // Combined SMA dist, ADX
  double volatility_state; // Combined ATR, Bollinger Width

  // Outcome Metrics
  double expected_value;
  double signal_strength;

  // Quantum Trajectory
  std::vector<StateVector> state_history;
};

class TechnicalAnalysis {
public:
  static AnalysisResult
  calculate_indicators(const std::vector<Candle> &candles,
                       const std::vector<Candle> &htf_candles = {},
                       bool is_stock = true);
  static std::string get_market_summary(const std::vector<Candle> &candles,
                                        const AnalysisResult &indicators);
};
