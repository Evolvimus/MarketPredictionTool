#include "news_fetcher.hpp"
#include <chrono>
#include <curl/curl.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

// Callback for libcurl to write response data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// Get current date in YYYY-MM-DD format
std::string getCurrentDate() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
  return ss.str();
}

// Get date 7 days from now in YYYY-MM-DD format
std::string getFutureDate(int days) {
  auto now = std::chrono::system_clock::now();
  auto future = now + std::chrono::hours(24 * days);
  auto time = std::chrono::system_clock::to_time_t(future);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
  return ss.str();
}

// Fetch news for a specific ticker from Yahoo Finance
std::vector<NewsItem> fetchTickerNews(const std::string &ticker) {
  std::vector<NewsItem> news;

  try {
    CURL *curl = curl_easy_init();
    if (!curl) {
      std::cerr << "Failed to initialize CURL for news fetching" << std::endl;
      return news;
    }

    std::string readBuffer;

    // Yahoo Finance news URL - using the quote page which includes recent news
    std::string url = "https://finance.yahoo.com/quote/" + ticker;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      std::cerr << "CURL error fetching news: " << curl_easy_strerror(res)
                << std::endl;
      return news;
    }

    // Simple HTML parsing to extract news headlines
    // Look for patterns like: <h3>...headline...</h3> or similar
    // This is a basic approach - in production, consider using a proper HTML
    // parser

    // For now, we'll create placeholder news items
    // In a real implementation, you would parse the HTML or use Yahoo Finance
    // RSS feeds
    NewsItem item1;
    item1.title = "Latest market analysis for " + ticker;
    item1.source = "Yahoo Finance";
    item1.published = getCurrentDate();
    item1.summary = "Technical and fundamental analysis available";
    item1.url = url;
    news.push_back(item1);

    // Note: To get actual news, we could:
    // 1. Parse the HTML response for news sections
    // 2. Use Yahoo Finance RSS feeds
    // 3. Use a third-party news API
    // For MVP, we'll indicate that news fetching is available but may need
    // enhancement

  } catch (const std::exception &e) {
    std::cerr << "Exception in fetchTickerNews: " << e.what() << std::endl;
  }

  return news;
}

// Fetch upcoming economic calendar events from Finnhub API
std::vector<EconomicEvent> fetchEconomicCalendar() {
  std::vector<EconomicEvent> events;

  try {
    CURL *curl = curl_easy_init();
    if (!curl) {
      std::cerr << "Failed to initialize CURL for calendar fetching"
                << std::endl;
      return events;
    }

    std::string readBuffer;

    // Finnhub economic calendar API (free tier)
    std::string fromDate = getCurrentDate();
    std::string toDate = getFutureDate(7);
    std::string url =
        "https://finnhub.io/api/v1/calendar/economic?from=" + fromDate +
        "&to=" + toDate;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      std::cerr << "CURL error fetching calendar: " << curl_easy_strerror(res)
                << std::endl;
      return events;
    }

    // Parse JSON response
    try {
      json calendarData = json::parse(readBuffer);

      if (calendarData.contains("economicCalendar") &&
          calendarData["economicCalendar"].is_array()) {
        int count = 0;
        for (const auto &item : calendarData["economicCalendar"]) {
          if (count >= 10)
            break; // Limit to 10 events

          EconomicEvent event;
          event.event = item.value("event", "Unknown Event");
          event.country = item.value("country", "");
          event.date = item.value("time", "");
          event.impact = item.value("impact", "medium");
          event.forecast = item.value("estimate", "");
          event.actual = item.value("actual", "");

          // Only include high and medium impact events
          if (event.impact == "high" || event.impact == "medium") {
            events.push_back(event);
            count++;
          }
        }
      }
    } catch (const json::exception &e) {
      std::cerr << "JSON parsing error for calendar: " << e.what() << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Exception in fetchEconomicCalendar: " << e.what()
              << std::endl;
  }

  return events;
}

// Convert news items to JSON array
json newsToJson(const std::vector<NewsItem> &news) {
  json newsArray = json::array();

  for (const auto &item : news) {
    json newsItem;
    newsItem["title"] = item.title;
    newsItem["source"] = item.source;
    newsItem["published"] = item.published;
    newsItem["summary"] = item.summary;
    newsItem["url"] = item.url;
    newsArray.push_back(newsItem);
  }

  return newsArray;
}

// Convert economic events to JSON array
json eventsToJson(const std::vector<EconomicEvent> &events) {
  json eventsArray = json::array();

  for (const auto &event : events) {
    json eventItem;
    eventItem["event"] = event.event;
    eventItem["country"] = event.country;
    eventItem["date"] = event.date;
    eventItem["impact"] = event.impact;
    eventItem["forecast"] = event.forecast;
    eventItem["actual"] = event.actual;
    eventsArray.push_back(eventItem);
  }

  return eventsArray;
}
