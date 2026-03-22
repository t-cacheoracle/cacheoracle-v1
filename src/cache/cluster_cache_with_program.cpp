#include "cluster_cache_with_program.h"

#include <fstream>

namespace {

bool hasPyExtension(const std::string& path) {
    return path.size() >= 3 && path.substr(path.size() - 3) == ".py";
}

bool readEntireFile(const std::string& path, std::string& out_contents) {
    std::ifstream input(path);
    if (!input) {
        return false;
    }

    out_contents.assign(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    return true;
}

bool writeEntireFile(const std::string& path, const std::string& contents) {
    std::ofstream output(path);
    if (!output) {
        return false;
    }

    output << contents;
    return static_cast<bool>(output);
}

} // namespace

ClusterCacheWithProgram::ClusterCacheWithProgram(int capacity)
    : cap_(capacity > 0 ? capacity : 1), size_(0), left_(nullptr), right_(nullptr) {
    left_ = new Node({}, "", {});
    right_ = new Node({}, "", {});
    left_->next = right_;
    right_->prev = left_;
}

ClusterCacheWithProgram::~ClusterCacheWithProgram() {
    clear();
    delete left_;
    delete right_;
}

void ClusterCacheWithProgram::remove(Node* node) {
    Node* prev = node->prev;
    Node* next = node->next;
    prev->next = next;
    next->prev = prev;
}

void ClusterCacheWithProgram::insert(Node* node) {
    Node* prev = right_->prev;
    prev->next = node;
    node->prev = prev;
    node->next = right_;
    right_->prev = node;
}

ClusterCacheWithProgram::Node* ClusterCacheWithProgram::findNode(
    const std::vector<double>& cluster_embedding) const {
    for (Node* node = left_->next; node != right_; node = node->next) {
        if (node->cluster_embedding == cluster_embedding) {
            return node;
        }
    }
    return nullptr;
}

void ClusterCacheWithProgram::put(const std::vector<double>& cluster_embedding,
                                  const std::string& python_program,
                                  const std::vector<double>& response_embedding) {
    Node* existing = findNode(cluster_embedding);
    if (existing != nullptr) {
        existing->python_program = python_program;
        existing->response_embedding = response_embedding;
        remove(existing);
        insert(existing);
        return;
    }

    Node* node = new Node(cluster_embedding, python_program, response_embedding);
    insert(node);
    ++size_;

    if (size_ > cap_) {
        Node* lru = left_->next;
        remove(lru);
        delete lru;
        --size_;
    }
}

bool ClusterCacheWithProgram::putFromFile(
    const std::vector<double>& cluster_embedding,
    const std::string& python_file_path,
    const std::vector<double>& response_embedding) {
    if (!hasPyExtension(python_file_path)) {
        return false;
    }

    std::string python_program;
    if (!readEntireFile(python_file_path, python_program)) {
        return false;
    }

    put(cluster_embedding, python_program, response_embedding);
    return true;
}

bool ClusterCacheWithProgram::get(const std::vector<double>& cluster_embedding,
                                  std::string* python_program,
                                  std::vector<double>* response_embedding) {
    Node* node = findNode(cluster_embedding);
    if (node == nullptr) {
        return false;
    }

    if (python_program != nullptr) {
        *python_program = node->python_program;
    }
    if (response_embedding != nullptr) {
        *response_embedding = node->response_embedding;
    }

    remove(node);
    insert(node);
    return true;
}

bool ClusterCacheWithProgram::getToFile(
    const std::vector<double>& cluster_embedding,
    const std::string& output_python_file_path,
    std::vector<double>* response_embedding) {
    if (!hasPyExtension(output_python_file_path)) {
        return false;
    }

    std::string python_program;
    if (!get(cluster_embedding, &python_program, response_embedding)) {
        return false;
    }

    return writeEntireFile(output_python_file_path, python_program);
}

std::vector<ClusterCacheWithProgram::Entry> ClusterCacheWithProgram::getEntries() const {
    std::vector<Entry> entries;
    entries.reserve(size_);

    for (Node* node = left_->next; node != right_; node = node->next) {
        entries.push_back({node->cluster_embedding, node->python_program, node->response_embedding});
    }

    return entries;
}

void ClusterCacheWithProgram::clear() {
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