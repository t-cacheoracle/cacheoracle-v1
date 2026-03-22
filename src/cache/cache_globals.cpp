#include "cache/cache_globals.h"

// Initialize global cache instances with a default capacity of 10.
ClusterCacheWithProgram CWP(10);
ClusterCacheNoProgram CNP(10);
QueryEmbeddingCache QEC(10);
