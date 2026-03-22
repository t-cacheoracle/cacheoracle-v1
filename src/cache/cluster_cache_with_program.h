#ifndef CLUSTER_CACHE_WITH_PROGRAM_H
#define CLUSTER_CACHE_WITH_PROGRAM_H

#include <string>
#include <vector>

class ClusterCacheWithProgram {
public:
	struct Entry {
		std::vector<double> cluster_embedding;
		std::string python_program;
		std::vector<double> response_embedding;
	};

	explicit ClusterCacheWithProgram(int capacity);
	~ClusterCacheWithProgram();

	ClusterCacheWithProgram(const ClusterCacheWithProgram&) = delete;
	ClusterCacheWithProgram& operator=(const ClusterCacheWithProgram&) = delete;

	void put(const std::vector<double>& cluster_embedding,
			 const std::string& python_program,
			 const std::vector<double>& response_embedding);

	bool putFromFile(const std::vector<double>& cluster_embedding,
				 const std::string& python_file_path,
				 const std::vector<double>& response_embedding);

	bool get(const std::vector<double>& cluster_embedding,
			 std::string* python_program,
			 std::vector<double>* response_embedding);

	bool getToFile(const std::vector<double>& cluster_embedding,
			   const std::string& output_python_file_path,
			   std::vector<double>* response_embedding);

	std::vector<Entry> getEntries() const;

private:
	struct Node {
		std::vector<double> cluster_embedding;
		std::string python_program;
		std::vector<double> response_embedding;
		Node* prev;
		Node* next;

		Node(const std::vector<double>& key,
			 const std::string& program,
			 const std::vector<double>& response)
			: cluster_embedding(key),
			  python_program(program),
			  response_embedding(response),
			  prev(nullptr),
			  next(nullptr) {}
	};

	int cap_;
	int size_;
	Node* left_;
	Node* right_;

	void remove(Node* node);
	void insert(Node* node);
	Node* findNode(const std::vector<double>& cluster_embedding) const;
	void clear();
};

#endif // CLUSTER_CACHE_WITH_PROGRAM_H
