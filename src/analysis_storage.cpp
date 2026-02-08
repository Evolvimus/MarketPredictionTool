#include "analysis_storage.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

// AnalysisRecord methods
json AnalysisRecord::to_json() const {
  return json{{"id", id},
              {"timestamp", timestamp},
              {"ticker", ticker},
              {"model", model},
              {"indicators",
               {{"rsi", rsi},
                {"macd", macd},
                {"macd_signal", macd_signal},
                {"sma_50", sma_50},
                {"sma_200", sma_200},
                {"current_price", current_price},
                {"adx", adx},
                {"boll_width", boll_width},
                {"atr_median", atr_median},
                {"volume_z_score", volume_z_score},
                {"roc_5", roc_5},
                {"roc_10", roc_10},
                {"roc_20", roc_20},
                {"obv", obv},
                {"vwap_dist", vwap_dist},
                {"sma_distance_pct", sma_distance_pct},
                {"range_pos", range_pos},
                {"is_stock", is_stock},
                {"htf_rsi", htf_rsi},
                {"htf_sma_50", htf_sma_50},
                {"htf_sma_200", htf_sma_200}}},
              {"trading_levels",
               {{"entry", entry_price},
                {"take_profit", take_profit},
                {"stop_loss", stop_loss},
                {"trailing_sl", trailing_sl},
                {"partial_tp", partial_tp}}},
              {"ai_prediction", ai_prediction},
              {"state_history",
               [this]() {
                 json ja = json::array();
                 for (const auto &s : state_history)
                   ja.push_back(s.to_json());
                 return ja;
               }()},
              {"feedback",
               {{"submitted", feedback.submitted},
                {"success", feedback.success},
                {"remark", feedback.remark}}}};
}

AnalysisRecord AnalysisRecord::from_json(const json &j) {
  AnalysisRecord record;
  record.id = j.value("id", "");
  record.timestamp = j.value("timestamp", "");
  record.ticker = j.value("ticker", "");
  record.model = j.value("model", "");

  if (j.contains("indicators")) {
    auto ind = j["indicators"];
    record.rsi = ind.value("rsi", 0.0);
    record.macd = ind.value("macd", 0.0);
    record.macd_signal = ind.value("macd_signal", 0.0);
    record.sma_50 = ind.value("sma_50", 0.0);
    record.sma_200 = ind.value("sma_200", 0.0);
    record.current_price = ind.value("current_price", 0.0);
    record.adx = ind.value("adx", 0.0);
    record.boll_width = ind.value("boll_width", 0.0);
    record.atr_median = ind.value("atr_median", 0.0);
    record.volume_z_score = ind.value("volume_z_score", 0.0);
    record.roc_5 = ind.value("roc_5", 0.0);
    record.roc_10 = ind.value("roc_10", 0.0);
    record.roc_20 = ind.value("roc_20", 0.0);
    record.obv = ind.value("obv", 0.0);
    record.vwap_dist = ind.value("vwap_dist", 0.0);
    record.sma_distance_pct = ind.value("sma_distance_pct", 0.0);
    record.range_pos = ind.value("range_pos", 0.0);
    record.is_stock = ind.value("is_stock", true);
    record.htf_rsi = ind.value("htf_rsi", 50.0);
    record.htf_sma_50 = ind.value("htf_sma_50", 0.0);
    record.htf_sma_200 = ind.value("htf_sma_200", 0.0);
  }

  if (j.contains("trading_levels")) {
    auto levels = j["trading_levels"];
    record.entry_price = levels.value("entry", 0.0);
    record.take_profit = levels.value("take_profit", 0.0);
    record.stop_loss = levels.value("stop_loss", 0.0);
    record.trailing_sl = levels.value("trailing_sl", 0.0);
    record.partial_tp = levels.value("partial_tp", 0.0);
  }

  record.ai_prediction = j.value("ai_prediction", "");

  if (j.contains("state_history")) {
    for (const auto &item : j["state_history"]) {
      record.state_history.push_back(
          {item.value("x", 0.0), item.value("y", 0.0), item.value("z", 0.0),
           item.value("t", "")});
    }
  }

  if (j.contains("feedback")) {
    auto fb = j["feedback"];
    record.feedback.submitted = fb.value("submitted", false);
    record.feedback.success = fb.value("success", false);
    record.feedback.remark = fb.value("remark", "");
  }

  return record;
}

// AnalysisStorage methods
AnalysisStorage::AnalysisStorage(const std::string &filename)
    : filename_(filename) {
  // Ensure file exists
  std::ifstream test(filename_);
  if (!test.good()) {
    save_to_file(json{{"analyses", json::array()}});
  }
}

std::string AnalysisStorage::save_analysis(const AnalysisRecord &record) {
  auto data = load_from_file();

  // Create a copy with generated ID and timestamp
  AnalysisRecord new_record = record;
  new_record.id = generate_id();
  new_record.timestamp = get_timestamp();

  // Add to analyses array
  data["analyses"].push_back(new_record.to_json());

  // Save to file
  save_to_file(data);

  std::cout << "💾 Saved analysis: " << new_record.id << " ("
            << new_record.ticker << ")" << std::endl;

  return new_record.id;
}

std::vector<AnalysisRecord> AnalysisStorage::get_recent_analyses(int limit) {
  auto data = load_from_file();
  std::vector<AnalysisRecord> results;

  if (!data.contains("analyses")) {
    return results;
  }

  auto analyses = data["analyses"];

  // Get last N analyses (newest first)
  int start = std::max(0, (int)analyses.size() - limit);
  for (int i = analyses.size() - 1; i >= start; i--) {
    results.push_back(AnalysisRecord::from_json(analyses[i]));
  }

  return results;
}

bool AnalysisStorage::update_feedback(const std::string &analysis_id,
                                      bool success, const std::string &remark) {
  auto data = load_from_file();

  if (!data.contains("analyses")) {
    return false;
  }

  // Find and update the analysis
  bool found = false;
  for (auto &analysis : data["analyses"]) {
    if (analysis["id"] == analysis_id) {
      analysis["feedback"]["submitted"] = true;
      analysis["feedback"]["success"] = success;
      analysis["feedback"]["remark"] = remark;
      found = true;
      break;
    }
  }

  if (found) {
    save_to_file(data);
    std::cout << "✅ Updated feedback for: " << analysis_id
              << (success ? " (SUCCESS)" : " (FAILED)") << std::endl;
  }

  return found;
}

std::vector<AnalysisRecord>
AnalysisStorage::get_successful_analyses(int limit) {
  auto data = load_from_file();
  std::vector<AnalysisRecord> results;

  if (!data.contains("analyses")) {
    return results;
  }

  // Filter for successful analyses
  for (const auto &analysis : data["analyses"]) {
    if (analysis.contains("feedback") &&
        analysis["feedback"]["submitted"] == true &&
        analysis["feedback"]["success"] == true) {
      results.push_back(AnalysisRecord::from_json(analysis));
      if (results.size() >= (size_t)limit)
        break;
    }
  }

  return results;
}

std::vector<AnalysisRecord> AnalysisStorage::get_failed_analyses(int limit) {
  auto data = load_from_file();
  std::vector<AnalysisRecord> results;

  if (!data.contains("analyses")) {
    return results;
  }

  // Filter for failed analyses
  for (const auto &analysis : data["analyses"]) {
    if (analysis.contains("feedback") &&
        analysis["feedback"]["submitted"] == true &&
        analysis["feedback"]["success"] == false) {
      results.push_back(AnalysisRecord::from_json(analysis));
      if (results.size() >= (size_t)limit)
        break;
    }
  }

  return results;
}

bool AnalysisStorage::delete_analysis(const std::string &analysis_id) {
  auto data = load_from_file();

  if (!data.contains("analyses")) {
    return false;
  }

  // Find and remove the analysis
  auto &analyses = data["analyses"];
  bool found = false;

  for (size_t i = 0; i < analyses.size(); i++) {
    if (analyses[i]["id"] == analysis_id) {
      analyses.erase(analyses.begin() + i);
      found = true;
      break;
    }
  }

  if (found) {
    save_to_file(data);
    std::cout << "🗑️  Deleted analysis: " << analysis_id << std::endl;
  }

  return found;
}

json AnalysisStorage::load_from_file() {
  std::ifstream file(filename_);
  if (!file.good()) {
    return json{{"analyses", json::array()}};
  }
  return json::parse(file);
}

void AnalysisStorage::save_to_file(const json &data) {
  std::ofstream file(filename_);
  file << data.dump(2); // Pretty print with 2-space indent
}

std::string AnalysisStorage::generate_id() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);

  const char *hex = "0123456789abcdef";
  std::stringstream ss;

  for (int i = 0; i < 32; i++) {
    ss << hex[dis(gen)];
    if (i == 7 || i == 11 || i == 15 || i == 19) {
      ss << '-';
    }
  }

  return ss.str();
}

std::string AnalysisStorage::get_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}
