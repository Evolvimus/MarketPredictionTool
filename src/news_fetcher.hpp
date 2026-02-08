#pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

// News item structure
struct NewsItem {
  std::string title;
  std::string source;
  std::string published;
  std::string summary;
  std::string url;
};

// Economic event structure
struct EconomicEvent {
  std::string event;
  std::string country;
  std::string date;
  std::string impact; // "high", "medium", "low"
  std::string forecast;
  std::string actual;
};

// Fetch news for a specific ticker from Yahoo Finance
std::vector<NewsItem> fetchTickerNews(const std::string &ticker);

// Fetch upcoming economic calendar events from Finnhub API
std::vector<EconomicEvent> fetchEconomicCalendar();

// Convert news items to JSON array
json newsToJson(const std::vector<NewsItem> &news);

// Convert economic events to JSON array
json eventsToJson(const std::vector<EconomicEvent> &events);
