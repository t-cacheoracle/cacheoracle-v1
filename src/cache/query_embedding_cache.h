#ifndef QUERY_EMBEDDING_CACHE_H
#define QUERY_EMBEDDING_CACHE_H

#include <cstddef>
#include <string>
#include <vector>

using QueryEmbedding = std::vector<double>;
using ResponseEmbedding = std::vector<double>;

struct QueryEmbeddingCacheValue {
	std::string response_text;
	ResponseEmbedding response_embedding;
};

class QueryEmbeddingCache {
private:
	struct Node {
		QueryEmbedding key;
		QueryEmbeddingCacheValue value;
		Node* prev;
		Node* next;

		Node(const QueryEmbedding& query_embedding,
			 const std::string& response_text,
			 const ResponseEmbedding& response_embedding)
			: key(query_embedding),
			  value{response_text, response_embedding},
			  prev(nullptr),
			  next(nullptr) {}
	};

	std::size_t cap;
	std::size_t current_size;
	Node* left;
	Node* right;

	void remove(Node* node);
	void insert(Node* node);

public:
	explicit QueryEmbeddingCache(std::size_t capacity);
	~QueryEmbeddingCache();

	void put(const QueryEmbedding& query_embedding,
			 const std::string& response_text,
			 const ResponseEmbedding& response_embedding);

	bool get(const QueryEmbedding& query_embedding, QueryEmbeddingCacheValue& out_value);
	void erase(const QueryEmbedding& query_embedding);

	// Return a copy of all entries as (key, value) pairs in LRU order (most-recent last).
	std::vector<std::pair<QueryEmbedding, QueryEmbeddingCacheValue>> getEntries() const;
	std::size_t size() const;
};

#endif // QUERY_EMBEDDING_CACHE_H
