#include "server.h"
#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"
#include "cache/query_embedding_cache.h"

#include <iostream>

int main(int argc, char** argv) {
	std::string addr = "0.0.0.0:50051";
	if (argc > 1) addr = argv[1];

	cacheoracle::GrpcServer server(addr);
	server.Run();

    ClusterCacheWithProgram cache(10);
    ClusterCacheNoProgram cache(10);
    QueryEmbeddingCache cache(10);
	
	return 0;
}
