#ifndef EMBED_LOGIC_H
#define EMBED_LOGIC_H

#include <string>

bool searchCWP(const std::string &prompt, std::string &response);
bool searchQEC(const std::string &prompt, std::string &response);
bool newLLMRespone(const std::string &prompt, std::string &response);
void runProgram(const std::string &prompt, std::string &response);
void retQECResponse(const std::string &prompt, std::string &response);

void start(const std::string &prompt, std::string &response);

#endif // EMBED_LOGIC_H
