#ifndef SETTINGS_STORAGE_HPP
#define SETTINGS_STORAGE_HPP

#include "httplib.h"
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

struct UserSettings {
  double account_balance = 10000.0;
  double risk_per_trade_pct = 1.0;
  int max_leverage = 10;

  json to_json() const {
    return json{{"account_balance", account_balance},
                {"risk_per_trade_pct", risk_per_trade_pct},
                {"max_leverage", max_leverage}};
  }

  static UserSettings from_json(const json &j) {
    UserSettings s;
    s.account_balance = j.value("account_balance", 10000.0);
    s.risk_per_trade_pct = j.value("risk_per_trade_pct", 1.0);
    s.max_leverage = j.value("max_leverage", 10);
    return s;
  }
};

class SettingsStorage {
public:
  explicit SettingsStorage(const std::string &filename);
  UserSettings get_settings();
  void save_settings(const UserSettings &settings);

private:
  std::string filename_;
  void ensure_file_exists();
};

#endif
