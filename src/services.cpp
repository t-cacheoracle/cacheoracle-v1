#include "services.h"
#include "embedlogic/embed_logic.h"

#include <grpcpp/grpcpp.h>

namespace cacheoracle {

grpc::Status CodegenServiceImpl::GenerateResponse(grpc::ServerContext* /*context*/, const ::cacheoracle::GenerateRequest* request,
                                                ::cacheoracle::Response* response) {
    const std::string &prompt = request->prompt();
    std::string out;
    start(prompt, out);
    response->set_response(out);
    return grpc::Status::OK;
}

} // namespace cacheoracle
