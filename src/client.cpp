#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "cacheoracle.grpc.pb.h"

int main(int argc, char** argv) {
    std::string target = "localhost:50051";
    std::string prompt = "sample prompt";

    if (argc > 1) {
        target = argv[1];
    }
    if (argc > 2) {
        prompt = argv[2];
    }

    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    auto stub = ::cacheoracle::CodegenService::NewStub(channel);

    ::cacheoracle::GenerateRequest request;
    request.set_prompt(prompt);

    ::cacheoracle::GenerateResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub->GeneratePython(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return 1;
    }

    std::cout << response.python_code() << std::endl;
    return 0;
}
