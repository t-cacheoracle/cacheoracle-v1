#ifndef CLUSTER_CACHE_NO_PROGRAM_H
#define CLUSTER_CACHE_NO_PROGRAM_H

#include <cstddef>
#include <vector>

using ClusterEmbedding = std::vector<double>;
using QueryEmbedding = std::vector<double>;
using QueryEmbeddingList = std::vector<QueryEmbedding>;

class ClusterCacheNoProgram {
public:
	struct Entry {
		ClusterEmbedding cluster_embedding;
		QueryEmbeddingList query_embeddings;
	};

	explicit ClusterCacheNoProgram(std::size_t capacity);
	~ClusterCacheNoProgram();

	ClusterCacheNoProgram(const ClusterCacheNoProgram&) = delete;
	ClusterCacheNoProgram& operator=(const ClusterCacheNoProgram&) = delete;

	void put(const ClusterEmbedding& cluster_embedding,
			 const QueryEmbeddingList& query_embeddings);

	bool get(const ClusterEmbedding& cluster_embedding,
			 QueryEmbeddingList* out_query_embeddings);

	std::vector<Entry> getEntries() const;
	std::size_t size() const;

private:
	struct Node {
		ClusterEmbedding cluster_embedding;
		QueryEmbeddingList query_embeddings;
		Node* prev;
		Node* next;

				Node(const ClusterEmbedding& key,
						 const QueryEmbeddingList& value)
						: cluster_embedding(key),
							query_embeddings(value),
			  prev(nullptr),
			  next(nullptr) {}
	};

	std::size_t cap_;
	std::size_t size_;
	Node* left_;
	Node* right_;

	void remove(Node* node);
	void insert(Node* node);
	Node* findNode(const ClusterEmbedding& cluster_embedding) const;
	void clear();
};

#endif // CLUSTER_CACHE_NO_PROGRAM_H
