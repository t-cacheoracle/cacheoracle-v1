//
// Created by Fabian Siswanto on 2026-03-22.
//

#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#pragma once
#include <fstream>
#include <iostream>
#include <string>

inline std::string defaultEnvPath() {
    std::string f = __FILE__;
    return f.substr(0, f.rfind('/')) + "/.env";
}

inline void loadEnv(const std::string &filepath = defaultEnvPath()) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open environment file: " << filepath << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Remove surrounding quotes if present
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
            value = value.substr(1, value.size() - 2);

        setenv(key.c_str(), value.c_str(), 1); // POSIX
        // On Windows: _putenv_s(key.c_str(), value.c_str());
    }
}

#endif //ENV_LOADER_H
