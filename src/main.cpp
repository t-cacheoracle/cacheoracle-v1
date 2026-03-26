#include "server.h"
#include "cache/cache_globals.h"

#include <iostream>

#include "codegen/llm.h"

llm g_llm;

int main(int argc, char** argv) {
	std::string addr = "0.0.0.0:50051";
	if (argc > 1) addr = argv[1];

	initGlobalCaches();

	cacheoracle::GrpcServer server(addr);
	server.Run();

	return 0;
}
