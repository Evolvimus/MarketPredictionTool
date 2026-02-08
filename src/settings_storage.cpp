#include "settings_storage.hpp"
#include <fstream>
#include <iostream>

SettingsStorage::SettingsStorage(const std::string &filename)
    : filename_(filename) {
  ensure_file_exists();
}

void SettingsStorage::ensure_file_exists() {
  std::ifstream f(filename_);
  if (!f.good()) {
    UserSettings default_settings;
    save_settings(default_settings);
  }
}

UserSettings SettingsStorage::get_settings() {
  try {
    std::ifstream file(filename_);
    if (!file.is_open())
      return UserSettings();
    json j;
    file >> j;
    return UserSettings::from_json(j);
  } catch (...) {
    return UserSettings();
  }
}

void SettingsStorage::save_settings(const UserSettings &settings) {
  std::ofstream file(filename_);
  file << settings.to_json().dump(4);
}
