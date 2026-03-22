#include "server.h"

#include <grpcpp/grpcpp.h>
#include <iostream>

namespace cacheoracle {

GrpcServer::GrpcServer(std::string server_address)
	: server_address_(std::move(server_address)), server_(nullptr) {}

GrpcServer::~GrpcServer() = default;

void GrpcServer::Run() {
	grpc::ServerBuilder builder;
	// Listen on the given address without authentication for now.
	builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

	// register our service implementations
	builder.RegisterService(&codegen_service_);

	server_ = builder.BuildAndStart();
	if (!server_) {
		std::cerr << "Failed to start gRPC server on " << server_address_ << std::endl;
		return;
	}

	std::cout << "gRPC server listening on " << server_address_ << std::endl;
	server_->Wait();
}

} // namespace cacheoracle
