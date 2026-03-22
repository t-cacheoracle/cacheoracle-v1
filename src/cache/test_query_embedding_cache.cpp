#include "cache/query_embedding_cache.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    QueryEmbeddingCache cache(2);

    QueryEmbedding q1 = {0.1, 0.2};
    QueryEmbedding q2 = {0.3, 0.4};
    QueryEmbedding q3 = {0.5, 0.6};

    cache.put(q1, "question1", "resp1", {1.0});
    cache.put(q2, "question2", "resp2", {2.0});

    QueryEmbeddingCacheValue value;
    bool found = cache.get(q1, value);
    assert(found);
    assert(value.response_text == "resp1");
    assert(value.response_embedding == std::vector<double>({1.0}));

    cache.put(q3, "question3", "resp3", {3.0});

    QueryEmbeddingCacheValue evicted;
    bool evicted_found = cache.get(q2, evicted);
    assert(!evicted_found);

    bool still_found = cache.get(q1, value);
    assert(still_found);
    assert(value.response_text == "resp1");

    assert(cache.size() == 2);

    std::cout << "query embedding cache tests passed\n";
    return 0;
}
