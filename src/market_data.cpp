#include "market_data.hpp"
#include <ctime>
#include <curl/curl.h>
#include <iostream>

// Helper for libcurl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::vector<Candle> MarketData::fetch_history(const std::string &ticker,
                                              const std::string &interval) {
  std::cout << "Fetching data for " << ticker << "..." << std::endl;
  std::vector<Candle> candles;

  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    // Yahoo Finance chart API (unofficial but works for demo)
    // range=1mo works, interval is passed
    // For C++, simpler to hardcode period for this MVP
    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" +
                      ticker + "?range=3mo&interval=" + interval;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    // User-Agent is sometimes required by Yahoo
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    } else {
      try {
        auto json_data = nlohmann::json::parse(readBuffer);

        auto result = json_data["chart"]["result"][0];
        auto timestamps = result["timestamp"];
        auto quote = result["indicators"]["quote"][0];

        auto opens = quote["open"];
        auto highs = quote["high"];
        auto lows = quote["low"];
        auto closes = quote["close"];
        auto volumes = quote["volume"];

        for (size_t i = 0; i < timestamps.size(); ++i) {
          if (opens[i].is_null() || closes[i].is_null())
            continue;

          Candle c;
          std::time_t t = timestamps[i];
          char buffer[80];
          std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                        std::localtime(&t));
          c.timestamp = std::string(buffer);

          c.open = opens[i];
          c.high = highs[i];
          c.low = lows[i];
          c.close = closes[i];
          c.volume = volumes[i].is_null() ? 0 : (long long)volumes[i];

          candles.push_back(c);
        }
      } catch (const std::exception &e) {
        std::cerr << "JSON Parsing error: " << e.what() << std::endl;
      }
    }
    curl_easy_cleanup(curl);
  }
  return candles;
}
