#include "services.h"

#include <grpcpp/grpcpp.h>

namespace cacheoracle {

grpc::Status CodegenServiceImpl::GenerateResponse(grpc::ServerContext* /*context*/, const ::cacheoracle::GenerateRequest* request,
                                                ::cacheoracle::Response* response) {
    const std::string &prompt = request->prompt();
    std::string out = "# Hello from cacheoracle:\n# ";
    out += prompt;
    response->set_response(out);
    return grpc::Status::OK;
}

} // namespace cacheoracle
