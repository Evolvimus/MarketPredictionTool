#include "analysis.hpp"
#include "analysis_storage.hpp"
#include "httplib.h"
#include "market_data.hpp"
#include "news_fetcher.hpp"
#include "nlohmann/json.hpp"
#include "ollama_client.hpp"
#include "settings_storage.hpp"
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using json = nlohmann::json;

// Global storage instances
AnalysisStorage storage("analyses.json");
SettingsStorage settings_storage("settings.json");

// Metrics globals
std::atomic<uint64_t> total_processed_candles{0};
std::chrono::steady_clock::time_point server_start_time;

int main() {
  server_start_time = std::chrono::steady_clock::now();
  httplib::Server svr;

  // Serve static files from public directory
  svr.set_mount_point("/", "./public");

  // API endpoint for analysis
  svr.Post("/api/analyze", [](const httplib::Request &req,
                              httplib::Response &res) {
    try {
      auto body = json::parse(req.body);
      std::string ticker = body.value("ticker", "AAPL");
      std::string model = body.value("model", "deepseek-v3.1:671b-cloud");

      std::cout << "API Request: ticker=" << ticker << ", model=" << model
                << std::endl;

      // Fetch market data
      auto request_start = std::chrono::steady_clock::now();

      // Fetch market data
      auto candles = MarketData::fetch_history(ticker, "1d");
      auto htf_candles = MarketData::fetch_history(ticker, "1wk");

      if (candles.empty()) {
        res.set_content("{\"error\": \"No data found for ticker\"}",
                        "application/json");
        res.status = 404;
        return;
      }

      // Determine asset type
      bool is_stock = true;
      if (ticker.find("=F") != std::string::npos || ticker == "BTC-USD") {
        is_stock = false;
      }

      // Calculate indicators with 3-model setup
      auto indicators = TechnicalAnalysis::calculate_indicators(
          candles, htf_candles, is_stock);

      // Metrics Update
      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> distinct_elapsed = now - request_start;
      double duration_seconds = distinct_elapsed.count();

      size_t batch_size = candles.size();
      total_processed_candles += batch_size;

      double rate =
          (duration_seconds > 0) ? (double)batch_size / duration_seconds : 0.0;

      std::cout << "[Metrics] Analysis: " << ticker
                << " | Batch: " << batch_size << " | Duration: " << std::fixed
                << std::setprecision(3) << duration_seconds << "s"
                << " | Speed: " << std::fixed << std::setprecision(1) << rate
                << " pts/sec"
                << " | Total: " << total_processed_candles << std::endl;
      std::string summary =
          TechnicalAnalysis::get_market_summary(candles, indicators);

      // Fetch news and economic calendar events
      auto news = fetchTickerNews(ticker);
      auto events = fetchEconomicCalendar();

      // Build context (Events, News, Signals)
      nlohmann::json context_data;
      context_data["regime"] = indicators.regime_info.regime;
      context_data["directional_model"] = {
          {"direction", indicators.ml_info.direction},
          {"probability", indicators.ml_info.probability}};
      context_data["indicators"] = {{"RSI", indicators.current_rsi},
                                    {"HTF_RSI", indicators.htf_rsi},
                                    {"ADX", indicators.adx},
                                    {"SMA_Dist", indicators.sma_distance_pct},
                                    {"ROC_20", indicators.roc_20},
                                    {"VWAP_Dist", indicators.vwap_dist}};

      std::vector<std::string> event_flags;
      for (const auto &e : events) {
        if (e.impact == "high")
          event_flags.push_back(e.event);
      }
      context_data["events"] = event_flags;

      // Get AI Meta-Analysis
      OllamaClient ai(model);
      std::string ai_response_str =
          ai.get_market_analysis(ticker, context_data.dump(), "");

      // Save to storage
      AnalysisRecord record;
      record.ticker = ticker;
      record.model = model;
      record.rsi = indicators.current_rsi;
      record.macd = indicators.macd;
      record.macd_signal = indicators.macd_signal;
      record.sma_50 = indicators.sma_50;
      record.sma_200 = indicators.sma_200;
      record.current_price = candles.back().close;
      record.adx = indicators.adx;
      record.boll_width = indicators.boll_width;
      record.atr_median = indicators.atr_median;
      record.volume_z_score = indicators.volume_z_score;
      record.roc_5 = indicators.roc_5;
      record.roc_10 = indicators.roc_10;
      record.roc_20 = indicators.roc_20;
      record.obv = indicators.obv;
      record.vwap_dist = indicators.vwap_dist;
      record.sma_distance_pct = indicators.sma_distance_pct;
      record.range_pos = indicators.range_pos;
      record.is_stock = indicators.is_stock;
      record.htf_rsi = indicators.htf_rsi;
      record.htf_sma_50 = indicators.htf_sma_50;
      record.htf_sma_200 = indicators.htf_sma_200;
      record.entry_price = indicators.entry_price;
      record.take_profit = indicators.take_profit;
      record.stop_loss = indicators.stop_loss;
      record.trailing_sl = indicators.trailing_sl;
      record.partial_tp = indicators.partial_tp;
      record.ai_prediction = ai_response_str;

      // Store state history
      for (const auto &s : indicators.state_history) {
        record.state_history.push_back({s.x, s.y, s.z, s.timestamp});
      }

      std::string analysis_id = storage.save_analysis(record);

      // Build response JSON
      json response;
      response["ticker"] = ticker;
      response["candleCount"] = candles.size();
      response["analysis_id"] = analysis_id;

      // Quantum State Data
      json state_vecs = json::array();
      for (const auto &s : indicators.state_history) {
        state_vecs.push_back(
            {{"x", s.x}, {"y", s.y}, {"z", s.z}, {"t", s.timestamp}});
      }
      response["quantum_state"] = {{"current",
                                    {{"x", indicators.momentum_state},
                                     {"y", indicators.trend_state},
                                     {"z", indicators.volatility_state}}},
                                   {"history", state_vecs}};

      response["regime"] = indicators.regime_info.regime;
      response["regime_confidence"] = indicators.regime_info.confidence;
      response["ml_direction"] = indicators.ml_info.direction;
      response["ml_probability"] = indicators.ml_info.probability;
      response["ml_expected_r"] = indicators.ml_info.expected_r;

      // Convert candles to JSON array (last 100 for performance)
      json candles_array = json::array();
      size_t start = candles.size() > 100 ? candles.size() - 100 : 0;
      for (size_t i = start; i < candles.size(); i++) {
        candles_array.push_back({{"timestamp", candles[i].timestamp},
                                 {"open", candles[i].open},
                                 {"high", candles[i].high},
                                 {"low", candles[i].low},
                                 {"close", candles[i].close},
                                 {"volume", candles[i].volume}});
      }
      response["candles"] = candles_array;

      // Add indicators
      response["indicators"] = {{"rsi", indicators.current_rsi},
                                {"htf_rsi", indicators.htf_rsi},
                                {"macd", indicators.macd},
                                {"adx", indicators.adx},
                                {"boll_width", indicators.boll_width},
                                {"atr_median", indicators.atr_median},
                                {"vol_z", indicators.volume_z_score},
                                {"roc_20", indicators.roc_20},
                                {"vwap_dist", indicators.vwap_dist},
                                {"sma_50", indicators.sma_50},
                                {"sma_200", indicators.sma_200}};

      // Add trading levels
      response["trading_levels"] = {{"entry", indicators.entry_price},
                                    {"take_profit", indicators.take_profit},
                                    {"stop_loss", indicators.stop_loss},
                                    {"trailing_sl", indicators.trailing_sl},
                                    {"partial_tp", indicators.partial_tp}};

      // Add news and economic events
      response["news"] = newsToJson(news);
      response["economic_events"] = eventsToJson(events);

      // Add AI prediction
      response["ai_prediction"] = ai_response_str;

      // --- Risk Management Calculation ---
      auto user_settings = settings_storage.get_settings();
      double balance = user_settings.account_balance;
      double risk_pct = user_settings.risk_per_trade_pct;
      double risk_amount = balance * (risk_pct / 100.0);

      double stop_loss = indicators.stop_loss;
      double entry = indicators.entry_price;
      double risk_per_unit = std::abs(entry - stop_loss);

      double units = 0;
      double notional = 0;
      double leverage = 1.0;

      if (risk_per_unit > 0) {
        units = risk_amount / risk_per_unit;
        notional = units * entry;
        leverage = notional / balance;
      }

      response["risk_management"] = {{"balance", balance},
                                     {"risk_amount", risk_amount},
                                     {"recommended_units", units},
                                     {"notional_value", notional},
                                     {"suggested_leverage", leverage},
                                     {"risk_pct", risk_pct}};

      res.set_content(response.dump(), "application/json");

    } catch (const std::exception &e) {
      json error = {{"error", e.what()}};
      res.set_content(error.dump(), "application/json");
      res.status = 500;
    }
  });

  // GET endpoint: Retrieve recent analyses
  svr.Get("/api/recent-analyses",
          [](const httplib::Request &req, httplib::Response &res) {
            try {
              auto recent = storage.get_recent_analyses(50);

              json response = json::array();
              for (const auto &record : recent) {
                response.push_back(record.to_json());
              }

              res.set_content(response.dump(), "application/json");

            } catch (const std::exception &e) {
              json error = {{"error", e.what()}};
              res.set_content(error.dump(), "application/json");
              res.status = 500;
            }
          });

  // POST endpoint: Submit feedback for an analysis
  svr.Post(
      "/api/feedback", [](const httplib::Request &req, httplib::Response &res) {
        try {
          auto body = json::parse(req.body);
          std::string analysis_id = body.value("analysis_id", "");
          bool success = body.value("success", false);
          std::string remark = body.value("remark", "");

          if (analysis_id.empty()) {
            res.set_content("{\"error\": \"Missing analysis_id\"}",
                            "application/json");
            res.status = 400;
            return;
          }

          bool updated = storage.update_feedback(analysis_id, success, remark);

          if (updated) {
            json response = {{"success", true},
                             {"message", "Feedback gespeichert"}};
            res.set_content(response.dump(), "application/json");
          } else {
            json error = {{"error", "Analysis not found"}};
            res.set_content(error.dump(), "application/json");
            res.status = 404;
          }

        } catch (const std::exception &e) {
          json error = {{"error", e.what()}};
          res.set_content(error.dump(), "application/json");
          res.status = 500;
        }
      });

  // DELETE endpoint: Delete an analysis
  svr.Delete(R"(/api/analysis/(.+))", [](const httplib::Request &req,
                                         httplib::Response &res) {
    try {
      std::string analysis_id = req.matches[1];

      if (analysis_id.empty()) {
        res.set_content("{\"error\": \"Missing analysis_id\"}",
                        "application/json");
        res.status = 400;
        return;
      }

      bool deleted = storage.delete_analysis(analysis_id);

      if (deleted) {
        json response = {{"success", true}, {"message", "Analyse gelöscht"}};
        res.set_content(response.dump(), "application/json");
      } else {
        json error = {{"error", "Analysis not found"}};
        res.set_content(error.dump(), "application/json");
        res.status = 404;
      }

    } catch (const std::exception &e) {
      json error = {{"error", e.what()}};
      res.set_content(error.dump(), "application/json");
      res.status = 500;
    }
  });

  // GET endpoint: Retrieve settings
  svr.Get("/api/settings",
          [](const httplib::Request &req, httplib::Response &res) {
            res.set_content(settings_storage.get_settings().to_json().dump(),
                            "application/json");
          });

  // POST endpoint: Update settings
  svr.Post("/api/settings",
           [](const httplib::Request &req, httplib::Response &res) {
             try {
               auto body = json::parse(req.body);
               settings_storage.save_settings(UserSettings::from_json(body));
               res.set_content("{\"success\": true}", "application/json");
             } catch (const std::exception &e) {
               res.status = 400;
               res.set_content("{\"error\": \"Invalid settings data\"}",
                               "application/json");
             }
           });

  // POST endpoint: Quantum Chat
  svr.Post("/api/chat_quantum", [](const httplib::Request &req,
                                   httplib::Response &res) {
    try {
      auto body = json::parse(req.body);
      std::string question = body.value("question", "");
      auto state =
          body["state"]; // Expect {momentum, trend, volatility, ticker}
      std::string ticker = state.value("ticker", "Unknown");

      double mom = state.value("momentum", 0.0);
      double trend = state.value("trend", 0.0);
      double vol = state.value("volatility", 0.0);
      std::string regime = state.value("regime", "neutral");

      // Construct System Prompt
      std::stringstream sys_prompt;
      sys_prompt << "You are a Quantum Market Physicist. You explain market "
                    "states using the 'Market Potential Field' theory.\n"
                 << "Current State for " << ticker << ":\n"
                 << "- Momentum (Direction): " << mom << " (range -1 to 1)\n"
                 << "- Trend Energy (Depth): " << trend << " (range -1 to 1)\n"
                 << "- Volatility (Entropy): " << vol << " (range 0 to 1)\n"
                 << "- Regime: " << regime << "\n\n"
                 << "The User is looking at a 3D visualization where:\n"
                 << "- X-Axis is Momentum (Tilt).\n"
                 << "- Y-Axis is Trend (Valleys=Stable, Bowls=Range).\n"
                 << "- Z-Axis is Volatility (Ripples).\n"
                 << "Explain the situation based on these physics metaphors. "
                    "Keep it short (max 3 sentences).";

      OllamaClient ai("deepseek-v3.1:671b-cloud");
      std::string answer = ai.ask_question(sys_prompt.str(), question);

      json response = {{"answer", answer}};
      res.set_content(response.dump(), "application/json");

    } catch (const std::exception &e) {
      json error = {{"error", e.what()}};
      res.set_content(error.dump(), "application/json");
      res.status = 500;
    }
  });

  std::cout << "🚀 Server starting on http://localhost:8080" << std::endl;
  std::cout << "📊 Open your browser and navigate to: http://localhost:8080"
            << std::endl;

  svr.listen("localhost", 8080);
  return 0;
}
