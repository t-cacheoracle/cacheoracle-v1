#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <regex>
#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"
#include "cache/query_embedding_cache.h"

using namespace std;

// Mock function to simulate running the stored python_program string
string executePythonProgram(const string &python_script, const string &raw_prompt)
{
    // In a real GenCache implementation, this would invoke a Python interpreter
    // or a regex-based extractor as defined in the paper.
    if (python_script.find("tax") != string::npos)
    {
        smatch m;
        if (regex_search(raw_prompt, m, regex("([0-9]+)")))
        {
            double val = stod(m[1].str());
            return "Total with tax: " + to_string(val * 1.12);
        }
    }
    return "Execution Error";
}

double cosineSimilarity(const vector<double> &a, const vector<double> &b)
{
    if (a.size() != b.size() || a.empty())
        return 0.0;
    double dot = 0, normA = 0, normB = 0;
    for (size_t i = 0; i < a.size(); i++)
    {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    return (normA == 0 || normB == 0) ? 0.0 : dot / (sqrt(normA) * sqrt(normB));
}

// Adjusted to use the Entry struct from your new class definition
ClusterCacheWithProgram::Entry findBestCluster(const vector<double> &newPromptEmbed,
                                               const ClusterCacheWithProgram &cache,
                                               double threshold = 0.8)
{
    double maxSim = -1.0;
    ClusterCacheWithProgram::Entry bestEntry = {{}, "", {}};
    bool found = false;

    for (const auto &entry : cache.getEntries())
    {
        double sim = cosineSimilarity(newPromptEmbed, entry.cluster_embedding);
        if (sim > maxSim && sim >= threshold)
        {
            maxSim = sim;
            bestEntry = entry;
            found = true;
        }
    }

    // Using an empty program string as the "not found" signal
    return found ? bestEntry : ClusterCacheWithProgram::Entry{{}, "", {}};
}

void run(const string &rawPrompt,
         const vector<double> &promptEmbed,
         const ClusterCacheWithProgram &clusterCache,
         const string &regexPattern)
{ // Regex pattern passed as the gatekeeper

    auto bestCluster = findBestCluster(promptEmbed, clusterCache);

    if (!bestCluster.python_program.empty())
    {
        // Structural check using the gatekeeper regex
        if (regex_match(rawPrompt, regex(regexPattern)))
        {
            string response = executePythonProgram(bestCluster.python_program, rawPrompt);
            cout << "Cache Hit! Output: " << response << endl;
            return;
        }
    }
    cout << "Cache Miss. Routing to LLM..." << endl;
}

int main()
{
    ClusterCacheWithProgram cache(10);

    // Python program stored as a string, along with the required response embedding
    string pythonCode = "python_tax_module: calculate_tax(prompt) * 1.12";
    vector<double> clusterCentroid = {0.1, 0.5, 0.8};
    vector<double> responseEmbed = {0.9, 0.1, 0.1}; // Placeholder for GenCache verification
    string pattern = "Calculate tax for ([0-9]+) dollars";

    cache.put(clusterCentroid, pythonCode, responseEmbed);

    cout << "--- Test 1: Hit ---" << endl;
    run("Calculate tax for 100 dollars", {0.11, 0.49, 0.81}, cache, pattern);

    cout << "\n--- Test 2: Miss (Structural) ---" << endl;
    // Same embedding, but the regex gatekeeper will catch the structural change
    run("What is the tax on 100 dollars?", {0.11, 0.49, 0.81}, cache, pattern);

    return 0;
}