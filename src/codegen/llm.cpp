//
// Created by Fabian Siswanto on 2026-03-21.
//

#include "llm.h"

#include "include/openai/openai.hpp"

llm::llm() {
    // TODO: move to env
    std::string openai_key = std::getenv("OPENAI_API_KEY");
    if (openai_key.empty()) {
        throw std::runtime_error("OPENAI_API_KEY not set");
    }
    openai::start(openai_key);
}

llm::~llm() = default;

std::string llm::generate_response(nlohmann::json &message_history) {
    nlohmann::json request;
    request["model"] = "gpt-5.4-nano";
    request["messages"] = message_history;
    request["max_completion_tokens"] = 1000;
    request["temperature"] = 0.7;

    nlohmann::json chat = openai::chat().create(request);
    std::string llm_response = chat["choices"][0]["message"]["content"];

    message_history.push_back({
        {"role", "assistant"},
        {"content", llm_response}
    });

    return llm_response;
}

int main() {
    llm llm;
    nlohmann::json sample_message_history = {
        {
            {"role", "user"},
            {"content", "What is the largest planet in the solar system?"}
        }
    };

    std::cout << llm.generate_response(sample_message_history) << std::endl;
    return 0;
}
