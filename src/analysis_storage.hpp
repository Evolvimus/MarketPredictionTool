#include "nlohmann/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

struct FeedbackData {
  bool submitted = false;
  bool success = false;
  std::string remark = "";
};

// Simple struct for storage (duplicating analysis.hpp generic struct or we
// could include analysis.hpp) To avoid circular deps, let's just use a simple
// struct or include the header if safe. analysis.hpp includes market_data.hpp.
// analysis_storage.hpp doesn't. Let's redefine valid storage structs here or
// include analysis.hpp. Provided view shows analysis_storage.hpp includes json,
// string, vector. Let's rely on JSON for "structless" storage inside
// AnalysisRecord for complex types OR simpler: just add a json field. But to be
// type safe:

struct StoredStateVector {
  double x, y, z;
  std::string timestamp;

  json to_json() const {
    return json{{"x", x}, {"y", y}, {"z", z}, {"t", timestamp}};
  }
};

struct AnalysisRecord {
  std::string id;
  std::string timestamp;
  std::string ticker;
  std::string model;

  // Technical indicators
  double rsi;
  double macd;
  double macd_signal;
  double sma_50;
  double sma_200;
  double current_price;
  double adx;
  double boll_width;
  double atr_median;
  double volume_z_score;
  double roc_5;
  double roc_10;
  double roc_20;
  double obv;
  double vwap_dist;
  double sma_distance_pct;
  double range_pos;
  bool is_stock;

  // MTF Features
  double htf_rsi;
  double htf_sma_50;
  double htf_sma_200;

  // Trading levels
  double entry_price;
  double take_profit;
  double stop_loss;
  double trailing_sl;
  double partial_tp;

  // AI prediction
  std::string ai_prediction;

  // User feedback
  FeedbackData feedback;

  // Quantum Trajectory
  std::vector<StoredStateVector> state_history;

  // Convert to JSON
  json to_json() const;

  // Create from JSON
  static AnalysisRecord from_json(const json &j);
};

class AnalysisStorage {
public:
  AnalysisStorage(const std::string &filename = "analyses.json");

  // Save a new analysis
  std::string save_analysis(const AnalysisRecord &record);

  // Get recent analyses (limit = max number to return)
  std::vector<AnalysisRecord> get_recent_analyses(int limit = 50);

  // Update feedback for an analysis
  bool update_feedback(const std::string &analysis_id, bool success,
                       const std::string &remark);

  // Get successful analyses for AI learning context
  std::vector<AnalysisRecord> get_successful_analyses(int limit = 5);

  // Get failed analyses for AI learning context
  std::vector<AnalysisRecord> get_failed_analyses(int limit = 5);

  // Delete an analysis by ID
  bool delete_analysis(const std::string &analysis_id);

private:
  std::string filename_;

  // Load all analyses from file
  json load_from_file();

  // Save all analyses to file
  void save_to_file(const json &data);

  // Generate unique ID
  std::string generate_id();

  // Get current timestamp
  std::string get_timestamp();
};
