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

std::string llm::generate_codegen_response(const std::string& user_query) {
    nlohmann::json messages = nlohmann::json::array();
    std::string system_prompt = R"(You are an expert program synthesis model.

    Your task is to analyze a few example input-output pairs and infer the underlying pattern that maps inputs to outputs.

    Given these examples, generate Python code that:
    - Correctly implements the inferred pattern
        - Works for arbitrary new inputs following the same format

    Requirements:
    - Output ONLY valid Python code (no explanations, no comments unless necessary)
    - The code must reproduce the exact format of the example outputs
    - Do not print anything extra
    - Ensure the solution is generalizable, not hardcoded to the examples

    You will be given:
    - Example inputs and their corresponding outputs

    Your goal:
    - Identify the pattern
    - Generate a Python function including functin name and header or script that produces the correct output for any valid input

    Strictly follow the output format shown in the examples.)";

    messages.push_back({
        {"role", "system"},
        {"content", system_prompt}
    });

    messages.push_back({
        {"role", "user"},
        {"content", user_query}
    });

    nlohmann::json request;
    request["model"] = "gpt-5.4-nano";
    request["messages"] = messages;
    request["max_completion_tokens"] = 1000;
    request["temperature"] = 0.7;

    nlohmann::json chat = openai::chat().create(request);
    std::string llm_response = chat["choices"][0]["message"]["content"];

    return llm_response;
}


int main() {
    llm llm;
    //std::cout << llm.generate_response("What is the largest planet in the solar system?") << std::endl;
    std::string prompt = R"(Example Inputs and Outputs:

Input: Buy food from Amazon
Output: Go to Amazon.com and search for Food

Input: Buy doll from Amazon
Output: Go to Amazon.com and search for doll

Input: Buy dragon from Amazon
Output: Go to Amazon.com and search for dragon

Input: Buy xyz from Amazon
Output: Go to Amazon.com and search for xyz

Input: Buy laptop from Amazon
Output: Go to Amazon.com and search for laptop

Now given:
Input: Buy dress from Amazon)";
    std::cout << llm.generate_codegen_response(prompt) << std::endl;
    return 0;
}
