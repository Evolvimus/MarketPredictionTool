#include "ollama_client.hpp"
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

OllamaClient::OllamaClient(const std::string &model_name) : model(model_name) {}

std::string
OllamaClient::get_market_analysis(const std::string &ticker,
                                  const std::string &market_summary,
                                  const std::string &feedback_context) {
  std::cout << "Meta-Analyst Thinking (Model: " << model << ")..." << std::endl;

  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  std::string result = "Error: Meta-Analysis failed.";

  curl = curl_easy_init();
  if (curl) {
    std::string url = base_url + "/api/generate";

    nlohmann::json request_body;
    request_body["model"] = model;
    request_body["stream"] = false;
    request_body["format"] = "json"; // Enforce JSON output for Meta-Analyst

    std::string prompt =
        "You are the 'Meta-Analyst' - an elite AI trading strategist with "
        "advanced reasoning capabilities.\\n"
        "You operate in a sophisticated 3-model trading system with access to "
        "regime classification, directional predictions, and comprehensive "
        "market data.\\n\\n"

        "═══════════════════════════════════════════════════════════════\\n"
        "CORE MISSION: Multi-Dimensional Market Analysis\\n"
        "═══════════════════════════════════════════════════════════════\\n\\n"

        "ANALYSIS FRAMEWORK (Execute in this order):\\n\\n"

        "1. MARKET REGIME ASSESSMENT\\n"
        "   - Analyze the current regime (trend_following, mean_reversion, "
        "high_vol, low_vol)\\n"
        "   - Evaluate regime stability and confidence level\\n"
        "   - Identify potential regime transitions or mixed signals\\n"
        "   - Consider historical regime persistence patterns\\n\\n"

        "2. MULTI-TIMEFRAME CONFLUENCE ANALYSIS\\n"
        "   - HTF (Higher Time Frame) vs LTF (Lower Time Frame) alignment\\n"
        "   - Identify divergences between timeframes\\n"
        "   - Assess the strength of trend alignment\\n"
        "   - Evaluate momentum consistency across timeframes\\n\\n"

        "3. TECHNICAL INDICATOR SYNTHESIS\\n"
        "   - RSI: Momentum exhaustion vs continuation signals\\n"
        "   - ADX: Trend strength and directional movement\\n"
        "   - MACD: Momentum shifts and crossover significance\\n"
        "   - Bollinger Bands: Volatility expansion/contraction\\n"
        "   - Volume Profile: Institutional participation and conviction\\n"
        "   - VWAP Distance: Price efficiency and mean reversion "
        "potential\\n\\n"

        "4. RISK FACTOR IDENTIFICATION\\n"
        "   - Overbought/Oversold extremes (RSI > 80 or < 20)\\n"
        "   - Volatility spikes or compression\\n"
        "   - Divergences between price and indicators\\n"
        "   - News/event risk from economic calendar\\n"
        "   - Liquidity concerns and gap risk\\n\\n"

        "5. DIRECTIONAL MODEL VALIDATION\\n"
        "   - Probability assessment (require > 60% for high confidence)\\n"
        "   - Expected R (Risk/Reward) evaluation\\n"
        "   - Signal strength and conviction level\\n"
        "   - Historical accuracy in similar market conditions\\n\\n"

        "6. CONTRADICTION DETECTION (CRITICAL)\\n"
        "   Examples of VETO-worthy contradictions:\\n"
        "   - Bullish signal but RSI > 85 (extreme overbought)\\n"
        "   - Bearish signal but RSI < 15 (extreme oversold)\\n"
        "   - LTF bullish but HTF in strong downtrend\\n"
        "   - High volatility regime but tight stop loss\\n"
        "   - Mean reversion regime but trend-following setup\\n"
        "   - Low probability (<50%) with high risk\\n\\n"

        "7. SOPHISTICATED DECISION LOGIC\\n"
        "   TRADE_ALLOWED criteria:\\n"
        "   ✓ HTF and LTF alignment confirmed\\n"
        "   ✓ No critical contradictions detected\\n"
        "   ✓ Probability > 55% (preferably > 65%)\\n"
        "   ✓ Risk/Reward ratio favorable (Expected R > 1.5)\\n"
        "   ✓ Regime matches strategy type\\n"
        "   ✓ No extreme indicator readings against direction\\n\\n"

        "   VETO criteria:\\n"
        "   ✗ HTF/LTF divergence\\n"
        "   ✗ Critical contradictions present\\n"
        "   ✗ Extreme overbought/oversold against signal\\n"
        "   ✗ High-impact news event imminent\\n"
        "   ✗ Probability < 50%\\n"
        "   ✗ Poor risk/reward (Expected R < 1.0)\\n\\n"

        "INPUT DATA (JSON):\\n" +
        market_summary;

    if (!feedback_context.empty()) {
      prompt += "\n\nCONTEXT & EVENTS:\n" + feedback_context;
    }

    prompt +=
        "\\n\\nOUTPUT FORMAT (STRICT JSON ONLY - No markdown, no explanations "
        "outside JSON):\\n"
        "{\\n"
        "  \\\"decision\\\": \\\"trade_allowed\\\" | \\\"veto\\\",\\n"
        "  \\\"confidence\\\": 0.0-1.0,\\n"
        "  \\\"reason\\\": \\\"Comprehensive explanation covering: regime "
        "analysis, HTF/LTF alignment, indicator synthesis, risk factors, and "
        "final verdict. Be specific and detailed.\\\",\\n"
        "  \\\"htf_confirmation\\\": \\\"confirmed\\\" | \\\"not_confirmed\\\" "
        "| \\\"divergent\\\",\\n"
        "  \\\"annotation\\\": \\\"Concise actionable insight (max 2 "
        "sentences)\\\",\\n"
        "  \\\"risk_level\\\": \\\"low\\\" | \\\"medium\\\" | \\\"high\\\",\\n"
        "  \\\"regime_alignment\\\": \\\"perfect\\\" | \\\"good\\\" | "
        "\\\"weak\\\" | \\\"contradictory\\\",\\n"
        "  \\\"key_factors\\\": [\\\"List 3-5 most critical factors "
        "influencing this decision\\\"],\\n"
        "  \\\"warnings\\\": [\\\"Any critical warnings or concerns (empty "
        "array if none)\\\"]\\n"
        "}";

    request_body["prompt"] = prompt;
    std::string json_str = request_body.dump();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      try {
        auto json_response = nlohmann::json::parse(readBuffer);
        if (json_response.contains("response")) {
          result = json_response["response"].get<std::string>();
        }
      } catch (...) {
        std::cerr << "Failed to parse Ollama response." << std::endl;
      }
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  return result;
}

std::string OllamaClient::ask_question(const std::string &system_prompt,
                                       const std::string &user_message) {
  std::cout << "Chat Request (Model: " << model << ")..." << std::endl;

  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  std::string result = "Error: Chat failed.";

  curl = curl_easy_init();
  if (curl) {
    std::string url = base_url + "/api/generate";

    nlohmann::json request_body;
    request_body["model"] = model;
    request_body["stream"] = false;

    // Construct prompt properly
    std::string prompt =
        system_prompt + "\n\nUser: " + user_message + "\n\nAssistant:";
    request_body["prompt"] = prompt;

    std::string json_str = request_body.dump();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      try {
        auto json_response = nlohmann::json::parse(readBuffer);
        if (json_response.contains("response")) {
          result = json_response["response"].get<std::string>();
        }
      } catch (...) {
        std::cerr << "Failed to parse Ollama chat response." << std::endl;
      }
    } else {
      std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  return result;
}
