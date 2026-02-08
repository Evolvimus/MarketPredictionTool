#pragma once
#include <string>

class OllamaClient {
public:
  OllamaClient(const std::string &model_name);
  std::string get_market_analysis(const std::string &ticker,
                                  const std::string &market_summary,
                                  const std::string &feedback_context = "");

  // Generic chat method
  std::string ask_question(const std::string &system_prompt,
                           const std::string &user_message);

private:
  std::string model;
  std::string base_url = "http://localhost:11434";
};
