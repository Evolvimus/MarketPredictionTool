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
        "You are the 'Meta-Analyst' in a 3-model trading system.\n"
        "Your task: Evaluate the input from the Regime Classifier and "
        "Directional Model.\n"
        "CRITICAL RULES:\n"
        "1. DO NOT invent trades. ONLY evaluate what is provided.\n"
        "2. Check for High Timeframe (HTF) Alignment. Does the signal match "
        "the HTF trend?\n"
        "3. You can VETO a trade if you see contradictions or high "
        "non-technical risk.\n"
        "4. Identify contradictions (e.g., Bullish prediction but RSI is 85 "
        "or HTF Trend is Bearish).\n\n"
        "INPUT DATA (JSON):\n" +
        market_summary;

    if (!feedback_context.empty()) {
      prompt += "\n\nCONTEXT & EVENTS:\n" + feedback_context;
    }

    prompt +=
        "\n\nOUTPUT FORMAT (JSON ONLY):\n"
        "{\n"
        "  \"decision\": \"trade_allowed\" | \"veto\",\n"
        "  \"confidence\": 0.0-1.0,\n"
        "  \"reason\": \"Explain why, mentioning HTF alignment and "
        "indicators\",\n"
        "  \"htf_confirmation\": \"confirmed\" | \"not_confirmed\",\n"
        "  \"annotation\": \"Short actionable trade note (e.g. 'Trend intact, "
        "TP/SL match regime')\"\n"
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
