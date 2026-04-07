#ifndef CACHE_GLOBALS_H
#define CACHE_GLOBALS_H

#include "cache/cluster_cache_with_program.h"
#include "cache/cluster_cache_no_program.h"
#include "cache/query_embedding_cache.h"

extern ClusterCacheWithProgram CWP;
extern ClusterCacheNoProgram CNP;
extern QueryEmbeddingCache QEC;

void initGlobalCaches();

#endif // CACHE_GLOBALS_H
