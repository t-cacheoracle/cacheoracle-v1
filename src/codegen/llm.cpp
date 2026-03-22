//
// Created by Fabian Siswanto on 2026-03-21.
//

#include "llm.h"

#include "../env_loader.h"
#include "include/openai/openai.hpp"

llm::llm() {
    loadEnv();
    std::string openai_key = std::getenv("OPENAI_API_KEY");
    if (openai_key.empty()) {
        throw std::runtime_error("OPENAI_API_KEY not set");
    }
    openai::start(openai_key);
}

llm::~llm() = default;

std::string llm::generate_response(const std::string& user_query) {
    nlohmann::json messages = nlohmann::json::array();
    messages.push_back({{"role", "user"}, {"content", user_query}});

    nlohmann::json request;
    request["model"] = "gpt-5.4-nano";
    request["messages"] = messages;
    request["max_completion_tokens"] = 1000;
    request["temperature"] = 0.7;

    nlohmann::json chat = openai::chat().create(request);
    std::string llm_response = chat["choices"][0]["message"]["content"];

    // todo: append to message history

    return llm_response;
}

int main() {
    llm llm;
    std::cout << llm.generate_response("What is the largest planet in the solar system?") << std::endl;
    return 0;
}
