#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static bool runEncoder(const std::string &text, std::vector<double> &embedding)
{
    std::string escaped = text;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\"");
        pos += 2;
    }

    std::string command = "python3 src/embedlogic/encoder.py \"" + escaped + "\"";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) return false;

    std::string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe))
        output += buffer;
    int exit_code = pclose(pipe);
    if (exit_code != 0) {
        std::cerr << "encoder.py failed:\n" << output << "\n";
        return false;
    }

    embedding.clear();
    size_t start = output.find('[');
    size_t end   = output.rfind(']');
    if (start == std::string::npos || end == std::string::npos) {
        std::cerr << "Could not find vector in output:\n" << output << "\n";
        return false;
    }

    std::string inner = output.substr(start + 1, end - start - 1);
    std::stringstream ss(inner);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try { embedding.push_back(std::stod(token)); }
        catch (...) { return false; }
    }
    return !embedding.empty();
}

int main()
{
    const std::string query = "Hello Indonesa from Vancouver";
    std::vector<double> embedding;

    std::cout << "Encoding: \"" << query << "\"\n";

    if (!runEncoder(query, embedding)) {
        std::cerr << "FAIL: runEncoder returned false\n";
        return 1;
    }

    std::cout << "Embedding dim: " << embedding.size() << "\n";
    std::cout << "First 8 values: ";
    for (size_t i = 0; i < 8 && i < embedding.size(); ++i)
        std::cout << embedding[i] << (i + 1 < 8 ? ", " : "\n");

    if (embedding.size() != 384) {
        std::cerr << "FAIL: expected dim 384, got " << embedding.size() << "\n";
        return 2;
    }

    std::cout << "PASS\n";
    return 0;
}
