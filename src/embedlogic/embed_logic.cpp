#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"
#include "cache/cache_globals.h"
#include "cache/query_embedding_cache.h"
#include "embedlogic/embed_logic.h"
#include "codegen/llm.h"

constexpr size_t CLUSTER_THRESHOLD = 10;
constexpr int MAX_PROGRAM_GENERATION_TRIES = 3;
constexpr double CWP_SIMILARITY_THRESHOLD = 0.40;

using namespace std;

static const string ENCODER_PATH = []() {
    string src_file = __FILE__;  // absolute path to embed_logic.cpp
    size_t last_slash = src_file.rfind('/');
    return src_file.substr(0, last_slash + 1) + "encoder.py";
}();

static bool runEncoder(const string &text, vector<double> &embedding)
{
    string escaped = text;
    // Escape double quotes in the query so the shell argument stays intact
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != string::npos) {
        escaped.replace(pos, 1, "\\\"");
        pos += 2;
    }

    string command = "python3 \"" + ENCODER_PATH + "\" \"" + escaped + "\" 2>/dev/null";
    std::cout << "[runEncoder] command: " << command << "\n";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cout << "[runEncoder] popen failed\n";
        return false;
    }

    string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe))
        output += buffer;
    int exit_code = pclose(pipe);
    std::cout << "[runEncoder] exit_code: " << exit_code << "\n";
    std::cout << "[runEncoder] output: " << output << "\n";

    // Parse Python list literal: [0.123, -0.456, ...]
    embedding.clear();
    size_t start = output.find('[');
    size_t end = output.rfind(']');
    std::cout << "[runEncoder] bracket start=" << start << " end=" << end << "\n";
    if (start == string::npos || end == string::npos) return false;

    string inner = output.substr(start + 1, end - start - 1);
    stringstream ss(inner);
    string token;
    while (getline(ss, token, ',')) {
        try { embedding.push_back(stod(token)); }
        catch (...) {
            std::cout << "[runEncoder] failed to parse token: '" << token << "'\n";
            return false;
        }
    }
    std::cout << "[runEncoder] parsed " << embedding.size() << " values\n";
    return !embedding.empty();
}

static bool runPythonProgramText(const string &python_program, string &response, const string &query = "")
{
    cout << python_program << endl;
    string python_program_A = python_program;
    if (python_program_A.empty()) {
        response.clear();
        return false;
    }
    cout << "EPTY" << endl;

    const string &python_file_path = python_program_A;
    if (python_file_path.size() < 3 || python_file_path.substr(python_file_path.size() - 3) != ".py") {
        response = "Invalid python file path: expected .py file";
        return false;
    }

    ifstream input(python_file_path);
    if (!input) {
        response = "Python file not found or unreadable: " + python_file_path;
        return false;
    }

    string command = "python3 \"" + python_file_path + "\" \"" + query + "\" 2>&1";
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        response = "Failed to start python process";
        return false;
    }

    cout << "nfail" << endl;

    response.clear();
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        response += buffer;
    }
    cout << response << endl;

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
    std::cout << "[searchCWP] best cosine similarity: " << best_sim
              << " (threshold: " << CWP_SIMILARITY_THRESHOLD << ")\n";
    if (best_sim < CWP_SIMILARITY_THRESHOLD)
        return {{}, "", {}};
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

ClusterEmbedding computeCentroid(const QueryEmbeddingList &embeddings) {
    if (embeddings.empty()) return {};
    size_t dim = embeddings[0].size();
    if (dim == 0) return {};
    ClusterEmbedding centroid(dim, 0.0);
    for (const auto &e : embeddings)
        for (size_t i = 0; i < dim; ++i)
            centroid[i] += e[i];
    for (auto &v : centroid)
        v /= static_cast<double>(embeddings.size());
    return centroid;
}

std::string generate_codegen_input(QueryEmbeddingCache &q_cache, ClusterCacheNoProgram::Entry qec_resp) {
    string codegen_input = "";
    for (const auto &query_emb : qec_resp.query_embeddings) {
        QueryEmbeddingCacheValue qec_val;
        if (q_cache.get(query_emb, qec_val)) {
            codegen_input += "question:" + qec_val.question_text + "\n" + "answer:" + qec_val.response_text + "\n";
        }
    }

    return codegen_input;
}

void insertQEC(QueryEmbeddingCache &q_cache, ClusterCacheNoProgram &cnp, string key, string value) {

    QueryEmbedding key_embedding;
    if (!runEncoder(key, key_embedding)) return;
    ResponseEmbedding value_embedding;
    if (!runEncoder(value, value_embedding)) return;

    q_cache.put(key_embedding, key, value, value_embedding);
    ClusterCacheNoProgram::Entry qec_resp = searchCNP(key_embedding, cnp);

    if (bool is_in_cluster = !qec_resp.cluster_embedding.empty()) {
        // recompute centroid of relevant cnp cluster
        QueryEmbeddingList qec_resp_query_embeddings = qec_resp.query_embeddings;
        qec_resp_query_embeddings.push_back(key_embedding);
        ClusterEmbedding new_cluster_embedding = computeCentroid(qec_resp_query_embeddings);

        if (qec_resp_query_embeddings.size() < CLUSTER_THRESHOLD) {
            return;
        }

        // codegen to generate program, have max tries
        string codegen_input = generate_codegen_input(q_cache, qec_resp);
        string program;
        string program_response;
        int tries = 0;
        while (program.empty() && tries < MAX_PROGRAM_GENERATION_TRIES) {
            program = g_llm.generate_codegen_response(codegen_input);
            cout << "BBBBBBBBBBBBBB2" << endl;

            if (bool fail_sanity_check = !runPythonProgramText(program, program_response)) {
                cout << "BBBBBBBBBBBBBB1" << endl;
                program = "";
            }
            tries++;
        }

        // delete related CNP cluster and its associated QEC entries
        for (const auto& query_emb : qec_resp.query_embeddings)
            q_cache.erase(query_emb);
        cnp.erase(qec_resp.cluster_embedding);

        if (!program.empty()) {
            vector<double> program_response_embedding;
            if (runEncoder(program_response, program_response_embedding)) {
                CWP.put(new_cluster_embedding, program, program_response_embedding);
            }
        }
    } else {
        // not in existing CNP cluster, create new cluster
        QueryEmbeddingList new_query_embedding_list = {key_embedding};
        cnp.put(key_embedding, new_query_embedding_list);
    }
}

void encodeLogic(const vector<double> &input_embedding, const string &prompt, string &response)
{
    //init all three caches
    ClusterCacheNoProgram &cnp = CNP;
    ClusterCacheWithProgram &cwp = CWP;
    QueryEmbeddingCache &q_cache = QEC;

    ClusterCacheWithProgram::Entry cwp_res = searchCWP(input_embedding, cwp);
    if (!cwp_res.python_program.empty()) {
        cout << "AAAAAAAAAAAAAAAAAAAAAAAA" << endl;

        size_t sp = prompt.rfind(' ');
        string last_word = (sp != string::npos) ? prompt.substr(sp + 1) : prompt;
        runPythonProgramText("embed_input_test.py", response, last_word);
        return;
        // optional TODO: implement sanity check
    }
    else {
        //try to find in qcache
        cout << "BBBBBBBBBBBBBB" << endl;

        QueryEmbeddingCacheValue qec_resp = searchQEC(input_embedding, q_cache);
        if (!qec_resp.response_text.empty()) { // hit qcache
            response = qec_resp.response_text;
            // optional TODO: update message history
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
    std::vector<double> input_embedding;
    runEncoder(prompt, input_embedding);

    std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";
    std::cout << "input_embedding size after runEncoder: " << input_embedding.size() << "\n";
    std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";

    std::cout << "input_embedding (" << input_embedding.size() << "): [";
    for (size_t i = 0; i < input_embedding.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << input_embedding[i];
    }
    std::cout << "]\n";
    encodeLogic(input_embedding, prompt, response);
}

#ifdef EMBEDLOGIC_STANDALONE

llm g_llm;

string generateProgramAndCheckSanity() { return ""; }

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

int main()
{
    cout << "TESTSETSET" << endl;
    string response;
    const string test_program_text = "embed_input_test.py";

    cout << "QWEQWEQWEQWE" << endl;
    const string test_query = "Buy apple from Amazon";
    size_t sp = test_query.rfind(' ');
    string last_word = (sp != string::npos) ? test_query.substr(sp + 1) : test_query;
    const bool ok = runPythonProgramText(test_program_text, response, last_word);
    cout << "EOIHEGOSIHGEOI" << endl;
    // remove(program_path.c_str());

    if (!ok) {
        cerr << "runPythonProgramText failed. Output:\n" << response << endl;
        return 2;
    }

    if (response.find("cacheoracle_test_ok") == string::npos) {
        cerr << "Expected output not found. Actual output:\n" << response << endl;
        return 3;
    }

    cout << "runPythonProgramText test passed. Output:\n" << response;

    // Test runEncoder
    // vector<double> embedding;
    // if (!runEncoder("Hello from embed_logic", embedding)) {
    //     cerr << "runEncoder failed\n";
    //     return 4;
    // }
    // if (embedding.size() != 384) {
    //     cerr << "runEncoder: expected dim 384, got " << embedding.size() << "\n";
    //     return 5;
    // }
    // cout << "runEncoder test passed. dim=" << embedding.size()
    //      << " first=" << embedding[0] << "\n";
    //
    // return 0;
}
#endif