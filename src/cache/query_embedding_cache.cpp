#include "query_embedding_cache.h"

QueryEmbeddingCache::QueryEmbeddingCache(std::size_t capacity)
    : cap(capacity), current_size(0) {
    left = new Node({}, "", {});
    right = new Node({}, "", {});
    left->next = right;
    right->prev = left;
}

QueryEmbeddingCache::~QueryEmbeddingCache() {
    Node* curr = left;
    while (curr) {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
}

void QueryEmbeddingCache::remove(Node* node) {
    Node* prev = node->prev;
    Node* next = node->next;
    prev->next = next;
    next->prev = prev;
}

void QueryEmbeddingCache::insert(Node* node) {
    Node* prev = right->prev;
    prev->next = node;
    node->prev = prev;
    node->next = right;
    right->prev = node;
}

void QueryEmbeddingCache::put(const QueryEmbedding& query_embedding,
                              const std::string& response_text,
                              const ResponseEmbedding& response_embedding) {
    Node* existing = nullptr;
    for (Node* node = left->next; node != right; node = node->next) {
        if (node->key == query_embedding) {
            existing = node;
            break;
        }
    }

    if (existing) {
        remove(existing);
        existing->value.response_text = response_text;
        existing->value.response_embedding = response_embedding;
        insert(existing);
        return;
    }

    if (cap == 0) {
        return;
    }

    if (current_size >= cap) {
        Node* lru = left->next;
        remove(lru);
        delete lru;
        current_size--;
    }

    Node* new_node = new Node(query_embedding, response_text, response_embedding);
    insert(new_node);
    current_size++;
}

bool QueryEmbeddingCache::get(const QueryEmbedding& query_embedding,
                              QueryEmbeddingCacheValue& out_value) {
    for (Node* node = left->next; node != right; node = node->next) {
        if (node->key == query_embedding) {
            remove(node);
            insert(node);
            out_value = node->value;
            return true;
        }
    }

    return false;
}

std::size_t QueryEmbeddingCache::size() const {
    return current_size;
}

std::vector<std::pair<QueryEmbedding, QueryEmbeddingCacheValue>> QueryEmbeddingCache::getEntries() const {
    std::vector<std::pair<QueryEmbedding, QueryEmbeddingCacheValue>> out;
    for (Node* node = left->next; node != right; node = node->next) {
        out.emplace_back(node->key, node->value);
    }
    return out;
}