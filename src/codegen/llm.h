//
// Created by Fabian Siswanto on 2026-03-21.
//

#ifndef LLM_H
#define LLM_H
#include <string>

#include "include/openai/nlohmann/json.hpp"


class llm {
public:
    llm();
    ~llm();
    std::string generate_response(const std::string& user_query);
    std::string generate_codegen_response(const std::string& user_query);

};



extern llm g_llm;

#endif //LLM_H
