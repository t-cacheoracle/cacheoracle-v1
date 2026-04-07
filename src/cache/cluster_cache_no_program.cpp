#include "cluster_cache_no_program.h"

ClusterCacheNoProgram::ClusterCacheNoProgram(std::size_t capacity)
    : cap_(capacity), size_(0), left_(nullptr), right_(nullptr) {
    left_ = new Node({}, {});
    right_ = new Node({}, {});
    left_->next = right_;
    right_->prev = left_;
}

ClusterCacheNoProgram::~ClusterCacheNoProgram() {
    clear();
    delete left_;
    delete right_;
}

void ClusterCacheNoProgram::remove(Node* node) {
    Node* prev = node->prev;
    Node* next = node->next;
    prev->next = next;
    next->prev = prev;
}

void ClusterCacheNoProgram::insert(Node* node) {
    Node* prev = right_->prev;
    prev->next = node;
    node->prev = prev;
    node->next = right_;
    right_->prev = node;
}

ClusterCacheNoProgram::Node* ClusterCacheNoProgram::findNode(
    const ClusterEmbedding& cluster_embedding) const {
    for (Node* node = left_->next; node != right_; node = node->next) {
        if (node->cluster_embedding == cluster_embedding) {
            return node;
        }
    }
    return nullptr;
}

void ClusterCacheNoProgram::put(const ClusterEmbedding& cluster_embedding,
                                const QueryEmbeddingList& query_embeddings) {
    Node* existing = findNode(cluster_embedding);
    if (existing != nullptr) {
        existing->query_embeddings = query_embeddings;
        remove(existing);
        insert(existing);
        return;
    }

    if (cap_ == 0) {
        return;
    }

    Node* node = new Node(cluster_embedding, query_embeddings);
    insert(node);
    ++size_;

    if (size_ > cap_) {
        Node* lru = left_->next;
        remove(lru);
        delete lru;
        --size_;
    }
}

bool ClusterCacheNoProgram::get(const ClusterEmbedding& cluster_embedding,
                                QueryEmbeddingList* out_query_embeddings) {
    Node* node = findNode(cluster_embedding);
    if (node == nullptr) {
        return false;
    }

    if (out_query_embeddings != nullptr) {
        *out_query_embeddings = node->query_embeddings;
    }

    remove(node);
    insert(node);
    return true;
}

std::vector<ClusterCacheNoProgram::Entry> ClusterCacheNoProgram::getEntries() const {
    std::vector<Entry> entries;
    entries.reserve(size_);

    for (Node* node = left_->next; node != right_; node = node->next) {
        entries.push_back({node->cluster_embedding, node->query_embeddings});
    }

    return entries;
}

void ClusterCacheNoProgram::erase(const ClusterEmbedding& cluster_embedding) {
    Node* node = findNode(cluster_embedding);
    if (node == nullptr) return;
    remove(node);
    delete node;
    --size_;
}

std::size_t ClusterCacheNoProgram::size() const {
    return size_;
}

void ClusterCacheNoProgram::clear() {
    Node* current = left_->next;
    while (current != right_) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    left_->next = right_;
    right_->prev = left_;
    size_ = 0;
}