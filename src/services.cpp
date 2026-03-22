#include "services.h"

#include <grpcpp/grpcpp.h>

namespace cacheoracle {

grpc::Status CodegenServiceImpl::GeneratePython(grpc::ServerContext* /*context*/, const ::cacheoracle::GenerateRequest* request,
                                                ::cacheoracle::GenerateResponse* response) {
    const std::string &prompt = request->prompt();
    std::string out = "# Generated Python from prompt:\n# ";
    out += prompt;
    out += "\n\nprint('Hello from codegen')\n";
    response->set_python_code(out);
    return grpc::Status::OK;
}

} // namespace cacheoracle
