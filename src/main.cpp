// Minimal entrypoint to start gRPC server.
#include "server.h"

#include <iostream>

int main(int argc, char** argv) {
	std::string addr = "0.0.0.0:50051";
	if (argc > 1) addr = argv[1];

	cacheoracle::GrpcServer server(addr);
	server.Run();

	return 0;
}
