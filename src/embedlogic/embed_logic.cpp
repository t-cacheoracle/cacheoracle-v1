#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>
#include <vector>

#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"
#include "cache/cache_globals.h"
#include "cache/query_embedding_cache.h"
#include "embedlogic/embed_logic.h"
#include "codegen/llm.h"

extern llm g_llm;

using namespace std;

static bool runPythonProgramText(const string &python_program, string &response)
{
    if (python_program.empty()) {
        response.clear();
        return false;
    }

    const string &python_file_path = python_program;
    if (python_file_path.size() < 3 || python_file_path.substr(python_file_path.size() - 3) != ".py") {
        response = "Invalid python file path: expected .py file";
        return false;
    }

    ifstream input(python_file_path);
    if (!input) {
        response = "Python file not found or unreadable: " + python_file_path;
        return false;
    }

    string command = "python3 \"" + python_file_path + "\" 2>&1";
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        response = "Failed to start python process";
        return false;
    }

    response.clear();
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        response += buffer;
    }

    int exit_code = pclose(pipe);
    return exit_code == 0;
}

static double cosineSimilarity(const vector<double> &a, const vector<double> &b)
{
    if (a.size() != b.size() || a.empty())
        return 0.0;
    double dot = 0.0, normA = 0.0, normB = 0.0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    if (normA == 0.0 || normB == 0.0)
        return 0.0;
    return dot / (sqrt(normA) * sqrt(normB));
}


ClusterCacheWithProgram::Entry searchCWP(const vector<double> &input_embedding, const ClusterCacheWithProgram &cwp) {
    ClusterCacheWithProgram::Entry best{{}, "", {}};
    double best_sim = -1.0;
    for (const auto &e : cwp.getEntries()) {
        double s = cosineSimilarity(input_embedding, e.cluster_embedding);
        if (s > best_sim) {
            best_sim = s;
            best = e;
        }
    }
    return best;
}

ClusterCacheNoProgram::Entry searchCNP(const vector<double> &input_embedding, const ClusterCacheNoProgram &cnp) {
    ClusterCacheNoProgram::Entry best{{}, {}};
    double best_sim = -1.0;
    for (const auto &e : cnp.getEntries()) {
        double s = cosineSimilarity(input_embedding, e.cluster_embedding);
        if (s > best_sim) {
            best_sim = s;
            best = e;
        }
    }
    return best;
}

QueryEmbeddingCacheValue searchQEC(const vector<double> &input_embedding, const QueryEmbeddingCache &q_cache) {
    QueryEmbeddingCacheValue best{{}, {}};
    double best_sim = -1.0;
    for (const auto &e : q_cache.getEntries()) {
        double s = cosineSimilarity(input_embedding, e.first);
        if (s > best_sim) {
            best_sim = s;
            best = e.second;
        }
    }
    return best;
}

void insertQEC(QueryEmbeddingCache &q_cache, const ClusterCacheNoProgram &cnp, string key, string value) {
    //encode key and valueinto embedding
    QueryEmbedding key_embedding; //THIS DOES NOT WORK
    ResponseEmbedding value_embedding; //THIS DOES NOT WORK
    q_cache.put(key_embedding, value, value_embedding);
    ClusterCacheNoProgram::Entry qec_resp = searchCNP(key_embedding, cnp);
    if (!qec_resp.cluster_embedding.empty()) {
        //recompute centroid of relevant cnp cluster
        //check if cnp members are >= threshold 
        //// > then codegen --> check sanity --> return response
        //// if we try code again more than 3 times and fail, END
        // IF < threshold then END
    } else {
        //recompute centroid of cnp cluster if totally new which ti is, new centroid
        //new method to add to existing cnp cluster    
    }
}

void insertCNP();

void encodeLogic(const vector<double> &input_embedding, const string &prompt, string &response)
{
    //init all three caches
    ClusterCacheNoProgram &cnp = CNP;
    ClusterCacheWithProgram &cwp = CWP;
    QueryEmbeddingCache &q_cache = QEC;

    ClusterCacheWithProgram::Entry cwp_res = searchCWP(input_embedding, cwp);
    if (!cwp_res.python_program.empty()) {
        runPythonProgramText(cwp_res.python_program, response);

        // TODO: implement sanity check (optional)
    }
    else {
        //try to find in qcache
        QueryEmbeddingCacheValue qec_resp = searchQEC(input_embedding, q_cache);
        if (!qec_resp.response_text.empty()) { // hit qcache
            response = qec_resp.response_text;
            // TODO: update message history (optional)
        } else { // miss qcache
            response = g_llm.generate_response(prompt);
            insertQEC(q_cache, cnp, prompt, response); // handle group logic too
        }
    }

}

void start(const string &prompt, string &response)
{
    // if (!searchCWP(prompt, response)) {
    //     if (!searchQEC(prompt, response)) {
    //         newLLMRespone(prompt, response);
    //     }
    // }
    
    // PYTHON CODE, using bash

    const vector<double> input_embedding;
    encodeLogic(input_embedding, prompt, response);
}

bool searchCWP(const string &prompt, string &response) {
    bool valid = false;
    if (valid) {
        runProgram(prompt, response);
        return true;
    } else {
        return false;
    }
}

bool searchQEC(const string &prompt, string &response) {
    bool valid = false;
    if (valid) {
        retQECResponse(prompt, response);
        return true;
    } else {
         return false;
    }
}

bool newLLMRespone(const string &prompt, string &response) {
    // LLM call placeholder
    string out = "LLM_generated_response";
    response = out;
    return true;
}

// Simple stub: execute the program or simulate running stored program
void runProgram(const string &prompt, string &response) {
    // Placeholder implementation — integrate real program execution here
    response = "Program executed for prompt: " + prompt;
}

// Simple stub: return a response from the Query Embedding Cache (QEC)
void retQECResponse(const string &prompt, string &response) {
    // Placeholder implementation — replace with actual QEC retrieval logic
    response = "QEC_response_for: " + prompt;
}

#ifdef EMBEDLOGIC_STANDALONE
int main()
{
    string response;
    const string program_path = "embed_input_test.py";
    {
        ofstream test_program(program_path);
        if (!test_program) {
            cerr << "Failed to create test python file: " << program_path << endl;
            return 1;
        }
        test_program << "print('cacheoracle_test_okfasdfasdfad')\n";
    }

    const bool ok = runPythonProgramText(program_path, response);
    remove(program_path.c_str());

    if (!ok) {
        cerr << "runPythonProgramText failed. Output:\n" << response << endl;
        return 2;
    }

    if (response.find("cacheoracle_test_ok") == string::npos) {
        cerr << "Expected output not found. Actual output:\n" << response << endl;
        return 3;
    }

    cout << "runPythonProgramText test passed. Output:\n" << response;
    return 0;
}
#endif