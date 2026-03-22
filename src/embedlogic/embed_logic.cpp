#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"
#include "cache/cache_globals.h"
#include "cache/query_embedding_cache.h"
#include "embedlogic/embed_logic.h"

using namespace std;

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

void encodeLogic(const vector<double> &input_embedding)
{
    //init all three caches
    ClusterCacheNoProgram &cnp = CNP;
    ClusterCacheWithProgram &cwp = CWP;
    QueryEmbeddingCache &q_cache = QEC;

    ClusterCacheWithProgram::Entry cwp_res = searchCWP(input_embedding, cwp);
    if (!cwp_res.python_program.empty()) {
        //run program --> check sanity --> return response
    }
    else {
        //try to find in qcache
        QueryEmbeddingCacheValue qec_resp = searchQEC(input_embedding, q_cache);  
        if (qec_resp.response_text.empty()) {
            //return response from qcache and end
        } else{
            //CALL LLM --> take response
            //TO DO: logic for inserting response into qcache and cnp

        }
    }

}

void start(const string &prompt,
           string &response)
{
    if (!searchCWP(prompt, response)) {
        if (!searchQEC(prompt, response)) {
            newLLMRespone(prompt, response);
        }
    }
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

    return 0;
}
#endif