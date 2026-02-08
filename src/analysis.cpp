#include "analysis.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

// Helper functions
template <typename T> T clamp(T v, T lo, T hi) {
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

double calculate_sma(const std::vector<double> &prices, int period) {
  if (prices.size() < period)
    return 0.0;
  double sum = std::accumulate(prices.end() - period, prices.end(), 0.0);
  return sum / period;
}

double calculate_ema(double current_price, double prev_ema, int period) {
  double multiplier = 2.0 / (period + 1);
  return (current_price - prev_ema) * multiplier + prev_ema;
}

// Helper for RSI
double calculate_rsi(const std::vector<double> &prices, int period) {
  if (prices.size() <= period)
    return 50.0;
  double avg_gain = 0, avg_loss = 0;
  for (int i = 1; i <= period; ++i) {
    double change = prices[i] - prices[i - 1];
    if (change > 0)
      avg_gain += change;
    else
      avg_loss -= change;
  }
  avg_gain /= period;
  avg_loss /= period;
  for (size_t i = period + 1; i < prices.size(); ++i) {
    double change = prices[i] - prices[i - 1];
    double gain = (change > 0) ? change : 0;
    double loss = (change < 0) ? -change : 0;
    avg_gain = (avg_gain * (period - 1) + gain) / period;
    avg_loss = (avg_loss * (period - 1) + loss) / period;
  }
  return (avg_loss == 0) ? 100.0
                         : 100.0 - (100.0 / (1.0 + avg_gain / avg_loss));
}

// Helper for ROC
double calculate_roc(const std::vector<double> &prices, int period) {
  if (prices.size() <= period)
    return 0.0;
  return (prices.back() - prices[prices.size() - 1 - period]) /
         prices[prices.size() - 1 - period] * 100.0;
}

// Helper for OBV
double calculate_obv(const std::vector<Candle> &candles) {
  if (candles.size() < 2)
    return 0.0;
  double obv = 0;
  for (size_t i = 1; i < candles.size(); ++i) {
    if (candles[i].close > candles[i - 1].close)
      obv += candles[i].volume;
    else if (candles[i].close < candles[i - 1].close)
      obv -= candles[i].volume;
  }
  return obv;
}

// Helper for VWAP Distance
double calculate_vwap_dist(const std::vector<Candle> &candles, int period) {
  if (candles.size() < period)
    return 0.0;
  double pv_sum = 0, v_sum = 0;
  for (size_t i = candles.size() - period; i < candles.size(); ++i) {
    pv_sum += candles[i].close * candles[i].volume;
    v_sum += (double)candles[i].volume;
  }
  if (v_sum == 0)
    return 0.0;
  double vwap = pv_sum / v_sum;
  return (candles.back().close - vwap) / vwap * 100.0;
}

// Helper for Bollinger Bands
std::pair<double, double> calculate_bollinger(const std::vector<double> &prices,
                                              int period, double std_dev_mult) {
  if (prices.size() < period)
    return {0.0, 0.0};
  double sma = calculate_sma(prices, period);
  double sum_sq_diff = 0;
  for (size_t i = prices.size() - period; i < prices.size(); ++i) {
    sum_sq_diff += std::pow(prices[i] - sma, 2);
  }
  double std_dev = std::sqrt(sum_sq_diff / period);
  return {sma + (std_dev_mult * std_dev), sma - (std_dev_mult * std_dev)};
}

// Simplified ADX (Average Directional Index)
double calculate_adx(const std::vector<Candle> &candles, int period) {
  if (candles.size() < period * 2)
    return 0.0;

  std::vector<double> tr, plus_dm, minus_dm;
  for (size_t i = 1; i < candles.size(); ++i) {
    double h = candles[i].high;
    double l = candles[i].low;
    double ph = candles[i - 1].high;
    double pl = candles[i - 1].low;
    double pc = candles[i - 1].close;

    tr.push_back(std::max({h - l, std::abs(h - pc), std::abs(l - pc)}));

    double move_up = h - ph;
    double move_down = pl - l;

    if (move_up > move_down && move_up > 0)
      plus_dm.push_back(move_up);
    else
      plus_dm.push_back(0);

    if (move_down > move_up && move_down > 0)
      minus_dm.push_back(move_down);
    else
      minus_dm.push_back(0);
  }

  // Smooth using SMA for simplicity in MVP
  double smooth_tr = calculate_sma(tr, period);
  double smooth_plus = calculate_sma(plus_dm, period);
  double smooth_minus = calculate_sma(minus_dm, period);

  if (smooth_tr == 0)
    return 0;

  double plus_di = 100 * smooth_plus / smooth_tr;
  double minus_di = 100 * smooth_minus / smooth_tr;

  if (plus_di + minus_di == 0)
    return 0;
  return 100 * std::abs(plus_di - minus_di) / (plus_di + minus_di);
}

AnalysisResult
TechnicalAnalysis::calculate_indicators(const std::vector<Candle> &candles,
                                        const std::vector<Candle> &htf_candles,
                                        bool is_stock) {
  AnalysisResult res;
  res.is_stock = is_stock;
  if (candles.empty())
    return res;

  std::vector<double> closes;
  for (const auto &c : candles)
    closes.push_back(c.close);

  // 1. Basic Indicators
  res.sma_50 = calculate_sma(closes, 50);
  res.sma_200 = calculate_sma(closes, 200);
  res.current_rsi = calculate_rsi(closes, 14);

  // MACD with Signal Line
  if (closes.size() > 26) {
    double ema12 = closes[0], ema26 = closes[0];
    // Warmup EMAs
    for (size_t i = 1; i < closes.size(); ++i) {
      ema12 = calculate_ema(closes[i], ema12, 12);
      ema26 = calculate_ema(closes[i], ema26, 26);
    }
    res.macd = ema12 - ema26;

    // Calculate Signal Line (9 EMA of MACD) - simplified approximation for now
    // Ideally we'd keep a history of MACD values, but for this stateless
    // function: We assume current MACD is close to recent average or calculate
    // strictly if we had history. For MVP/Stateless: We will estimate Signal
    // based on recent price momentum or strictly, we need the MACD series.
    // Let's re-calculate MACD series for the last 9 periods to get a proper
    // signal line.
    std::vector<double> macd_history;
    double e12 = closes[std::max((int)0, (int)closes.size() - 50)];
    double e26 = e12;

    // Recalculate EMAs for a window to build valid MACD signal
    int start_idx = std::max((int)0, (int)closes.size() - 50);
    for (int i = start_idx; i < closes.size(); ++i) {
      e12 = calculate_ema(closes[i], e12, 12);
      e26 = calculate_ema(closes[i], e26, 26);
      if (i >= start_idx + 26) {
        macd_history.push_back(e12 - e26);
      }
    }

    if (!macd_history.empty()) {
      res.macd_signal = calculate_ema(res.macd, macd_history.front(), 9);
      // Correct signal line using the series
      double sig = macd_history[0];
      for (size_t i = 1; i < macd_history.size(); ++i) {
        sig = calculate_ema(macd_history[i], sig, 9);
      }
      res.macd_signal = sig;
    } else {
      res.macd_signal = res.macd; // Fallback
    }
  }

  // 2. Advanced Features for 3-Model Setup
  res.adx = calculate_adx(candles, 14);
  auto bb = calculate_bollinger(closes, 20, 2.0);
  res.boll_upper = bb.first;
  res.boll_lower = bb.second;
  res.boll_width = (res.boll_upper - res.boll_lower) / closes.back();

  // ATR and ATR Median
  double current_atr = 0;
  std::vector<double> atrs;
  if (candles.size() >= 14) {
    for (size_t i = std::max((size_t)1, candles.size() - 50);
         i < candles.size(); ++i) {
      double tr = std::max({candles[i].high - candles[i].low,
                            std::abs(candles[i].high - candles[i - 1].close),
                            std::abs(candles[i].low - candles[i - 1].close)});
      atrs.push_back(tr);
    }
    current_atr = std::accumulate(atrs.end() - 14, atrs.end(), 0.0) / 14.0;
    std::sort(atrs.begin(), atrs.end());
    res.atr_median = atrs[atrs.size() / 2];
  }

  // New Indicators
  res.roc_5 = calculate_roc(closes, 5);
  res.roc_10 = calculate_roc(closes, 10);
  res.roc_20 = calculate_roc(closes, 20);
  res.obv = calculate_obv(candles);
  res.vwap_dist = calculate_vwap_dist(candles, 20);

  res.sma_distance_pct =
      (res.sma_200 > 0) ? (closes.back() - res.sma_200) / res.sma_200 * 100 : 0;

  double low_50 = closes.back(), high_50 = closes.back();
  for (size_t i = std::max((size_t)0, closes.size() - 50); i < closes.size();
       ++i) {
    low_50 = std::min(low_50, closes[i]);
    high_50 = std::max(high_50, closes[i]);
  }
  res.range_pos =
      (high_50 == low_50) ? 0.5 : (closes.back() - low_50) / (high_50 - low_50);

  // MTF Features
  if (!htf_candles.empty()) {
    std::vector<double> htf_closes;
    for (const auto &c : htf_candles)
      htf_closes.push_back(c.close);
    res.htf_rsi = calculate_rsi(htf_closes, 14);
    res.htf_sma_50 = calculate_sma(htf_closes, 50);
    res.htf_sma_200 = calculate_sma(htf_closes, 200);
  } else {
    res.htf_rsi = 50.0;
    res.htf_sma_50 = 0.0;
    res.htf_sma_200 = 0.0;
  }

  // Volume Z-Score
  if (candles.size() >= 20) {
    double vol_sum = 0, vol_sq_sum = 0;
    for (size_t i = candles.size() - 20; i < candles.size(); ++i) {
      vol_sum += candles[i].volume;
      vol_sq_sum += (double)candles[i].volume * candles[i].volume;
    }
    double vol_mean = vol_sum / 20.0;
    double vol_std = std::sqrt(vol_sq_sum / 20.0 - vol_mean * vol_mean);
    res.volume_z_score =
        (vol_std == 0) ? 0 : (candles.back().volume - vol_mean) / vol_std;
  }

  // --- 3. Feature Normalization & State Vectors ---

  // Normalize RSI: (Val - 50) / 50 -> [-1, 1]
  res.rsi_norm = (res.current_rsi - 50.0) / 50.0;

  // Normalize MACD: Histogram / Close (approx %) -> scaled
  double macd_hist = res.macd - res.macd_signal;
  res.macd_hist_norm = clamp((macd_hist / closes.back()) * 100.0, -1.0, 1.0);
  res.macd_norm = clamp((res.macd / closes.back()) * 100.0, -1.0, 1.0);

  // Normalize ROC: clamp(roc / 5, -1, 1)
  res.roc_norm = clamp(res.roc_20 / 5.0, -1.0, 1.0);

  // Normalize ADX: Val / 50 -> [0, 1+]
  res.adx_norm = clamp(res.adx / 50.0, 0.0, 1.0);

  // Normalize SMA Distance: % dist clamp
  res.sma_dist_norm = clamp(res.sma_distance_pct / 10.0, -1.0, 1.0);

  // Normalize Bollinger Width (volatility): Relative to recent avg or just raw
  // scaled Heuristic: Width > 0.05 is high vol for many assets, but varies.
  // Using ATR/Close as proxy for "normal" vol.
  double rel_atr = (current_atr / closes.back()); // e.g. 0.01 for 1%
  res.bollinger_width_norm =
      clamp(res.boll_width / 0.10, 0.0, 1.0); // 10% width is huge

  // Normalize Volume: Z-score is already normalized-ish, clamp it
  res.volume_z_norm = clamp(res.volume_z_score / 3.0, -1.0, 1.0);

  // --- Market State Vectors ---

  // Momentum State: Avg of RSI, ROC, MACD Hist
  res.momentum_state = (res.rsi_norm + res.roc_norm + res.macd_hist_norm) / 3.0;

  // Trend State: Avg of SMA Dist, ADX (directional), HTF alignment
  double htf_align = 0.0;
  if (res.htf_sma_200 > 0) {
    htf_align = (closes.back() > res.htf_sma_200) ? 0.5 : -0.5;
  }
  // ADX is non-directional, multiply by trend sign (Momentum)
  double trend_dir = (res.momentum_state > 0) ? 1.0 : -1.0;
  res.trend_state =
      (res.sma_dist_norm + (res.adx_norm * trend_dir) + htf_align) / 3.0;

  // Volatility State: Combined ATR, Width
  // Normalize ATR Rel: 0.5% is low, 2% is high
  double atr_norm = clamp((rel_atr - 0.005) / 0.015, 0.0, 1.0);
  res.volatility_state = (atr_norm + res.bollinger_width_norm) / 2.0;

  res.volatility_state = (atr_norm + res.bollinger_width_norm) / 2.0;

  // --- Quantum Trajectory (State History) ---
  // Calculate states for the last 50 candles to show trajectory.
  // We use a simplified calculation for history to keep it fast (approximated).
  int history_len = 50;
  int start_idx = std::max((int)0, (int)closes.size() - history_len);

  for (int i = start_idx; i < closes.size(); ++i) {
    if (i < 50)
      continue; // Need some warmup

    // 1. Momentum Component (Approx RSI + ROC)
    // Recalculate RSI for this point (simplified lookback)
    // This is O(N*M) but N=50, M=14 so it's fine.
    double avg_gain = 0, avg_loss = 0;
    for (int k = 0; k < 14; ++k) {
      double chg = closes[i - k] - closes[i - k - 1];
      if (chg > 0)
        avg_gain += chg;
      else
        avg_loss -= chg;
    }
    avg_gain /= 14.0;
    avg_loss /= 14.0;
    double rs = (avg_loss == 0) ? 100 : avg_gain / avg_loss;
    double rsi = 100.0 - (100.0 / (1.0 + rs));
    double rsi_n = (rsi - 50.0) / 50.0;

    double roc_n = clamp(
        ((closes[i] - closes[i - 5]) / closes[i - 5] * 100.0) / 5.0, -1.0, 1.0);

    double mom_s = (rsi_n + roc_n) / 2.0;

    // 2. Trend Component (SMA Dist)
    double sma_50_i = 0; // Simple accumulation
    for (int k = 0; k < 50; ++k)
      sma_50_i += closes[i - k];
    sma_50_i /= 50.0;
    double sma_dist_n =
        clamp(((closes[i] - sma_50_i) / sma_50_i * 100.0) / 10.0, -1.0, 1.0);

    double trend_s = sma_dist_n; // Simplified

    // 3. Volatility Component (Range)
    double range_high = closes[i], range_low = closes[i];
    for (int k = 0; k < 20; ++k) {
      range_high = std::max(range_high, closes[i - k]);
      range_low = std::min(range_low, closes[i - k]);
    }
    double vol_measure = (range_high - range_low) / closes[i];
    double vol_s = clamp(vol_measure / 0.10, 0.0, 1.0);

    res.state_history.push_back({mom_s, trend_s, vol_s, candles[i].timestamp});
  }
  // Allow last point to overwrite with the "precise" calculations we just did
  // above
  if (!res.state_history.empty()) {
    res.state_history.back() = {res.momentum_state, res.trend_state,
                                res.volatility_state, candles.back().timestamp};
  }

  // --- 4. Call Python ML Bridge ---
  try {
    nlohmann::json ml_input;
    // Sending compact state vectors + raw essentials
    ml_input["features"] = {// State Vectors
                            {"momentum_state", res.momentum_state},
                            {"trend_state", res.trend_state},
                            {"volatility_state", res.volatility_state},

                            // Normalized bits if needed individually
                            {"rsi_norm", res.rsi_norm},
                            {"roc_norm", res.roc_norm},
                            {"vol_z_norm", res.volume_z_norm},
                            {"sma_dist_norm", res.sma_dist_norm},

                            // Raw needed for absolute levels
                            {"rsi", res.current_rsi},
                            {"adx", res.adx},
                            {"is_stock", is_stock}};

    std::string cmd =
        "echo '" + ml_input.dump() + "' | python3 scripts/ml_predict.py";
    FILE *pipe = popen(cmd.c_str(), "r");
    if (pipe) {
      char buffer[1024];
      std::string ml_output_str;
      while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        ml_output_str += buffer;
      pclose(pipe);

      auto ml_output = nlohmann::json::parse(ml_output_str);
      res.regime_info = {ml_output["regime_model"]["regime"],
                         ml_output["regime_model"]["confidence"]};

      // Parse detailed ML info
      auto dir_model = ml_output["directional_model"];
      res.ml_info = {dir_model["direction"], dir_model["probability"],
                     dir_model["expected_r"]};

      // Store new metrics
      res.expected_value = dir_model["expected_value"].get<double>();
      res.signal_strength = dir_model["signal_strength"].get<double>();
    }
  } catch (...) {
    res.regime_info = {"unknown", 0.0};
    res.ml_info = {"neutral", 0.0, 0.0};
    res.expected_value = 0.0;
    res.signal_strength = 0.0;
  }

  // 5. Final TP/SL & Risk Filtering
  double current_price = closes.back();
  double atr = current_atr;

  // Filter mostly based on Expected Value > 0.3 (as per expert review)
  if (res.expected_value < 0.2) { // Slightly lenient for MVP
    res.ml_info.direction = "neutral";
    res.ml_info.probability = 0.0;
  }

  // Volatility Adjusted TP/SL Multipliers
  // If Volatility is High -> Wider SL, tighter TP (mean reversion logic or
  // safety) Or: High Vol -> Wider SL, Wider TP (trend following) Let's use:
  // High Vol -> Wider stops to avoid noise.
  double sl_mult = 1.0;
  double tp_mult = 1.5;

  if (res.volatility_state > 0.7) {
    sl_mult = 1.5; // Wider stop in high vol
    tp_mult = 2.0;
  } else if (res.volatility_state < 0.3) {
    sl_mult = 0.8; // Tighter stop in low vol
    tp_mult = 2.5; // Target breakout
  }

  if (res.regime_info.regime == "trend") {
    res.stop_loss = (res.ml_info.direction == "long")
                        ? current_price - (atr * 1.0 * sl_mult)
                        : current_price + (atr * 1.0 * sl_mult);
    res.take_profit = (res.ml_info.direction == "long")
                          ? current_price + (atr * 3.0 * tp_mult)
                          : current_price - (atr * 3.0 * tp_mult);
  } else if (res.regime_info.regime == "range") {
    res.stop_loss = (res.ml_info.direction == "long")
                        ? current_price - (atr * 1.0 * sl_mult)
                        : current_price + (atr * 1.0 * sl_mult);
    res.take_profit = (res.ml_info.direction == "long")
                          ? current_price + (atr * 2.0) // Fixed 2R in range
                          : current_price - (atr * 2.0);
  } else {
    // Default
    res.stop_loss = (res.ml_info.direction == "long")
                        ? current_price - (atr * 1.5)
                        : current_price + (atr * 1.5);
    res.take_profit = (res.ml_info.direction == "long")
                          ? current_price + (atr * 2.0)
                          : current_price - (atr * 2.0);
  }

  // Advanced Risk Management
  res.trailing_sl = (res.ml_info.direction == "long") ? current_price + atr
                                                      : current_price - atr;
  res.partial_tp = (res.ml_info.direction == "long")
                       ? current_price + (atr * 1.0)
                       : current_price - (atr * 1.0);

  res.entry_price = current_price;

  return res;
}

std::string
TechnicalAnalysis::get_market_summary(const std::vector<Candle> &candles,
                                      const AnalysisResult &indicators) {
  if (candles.empty())
    return "No Data";
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  ss << "### Technical Summary ###\n";
  ss << "Regime: " << indicators.regime_info.regime << " ("
     << (int)(indicators.regime_info.confidence * 100) << "%)\n";
  ss << "ML Prediction: " << indicators.ml_info.direction
     << " (Prob: " << indicators.ml_info.probability << ")\n";
  ss << "RSI: " << indicators.current_rsi << " | MACD: " << indicators.macd
     << "\n";
  ss << "ADX: " << indicators.adx
     << " | Bollinger Width: " << indicators.boll_width << "\n";
  ss << "SMA 50/200: " << indicators.sma_50 << " / " << indicators.sma_200
     << "\n";
  ss << "Momentum State: " << indicators.momentum_state
     << " | Trend State: " << indicators.trend_state << "\n";
  ss << "Vol State: " << indicators.volatility_state
     << " | Exp. Value: " << indicators.expected_value << "\n";
  return ss.str();
}
