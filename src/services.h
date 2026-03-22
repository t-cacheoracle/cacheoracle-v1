#ifndef SERVICES_H
#define SERVICES_H

#include <grpcpp/grpcpp.h>

#include "cacheoracle.grpc.pb.h"

namespace cacheoracle {

class CodegenServiceImpl final : public ::cacheoracle::CodegenService::Service {
public:
    grpc::Status GenerateResponse(grpc::ServerContext* context, const ::cacheoracle::GenerateRequest* request,
                                ::cacheoracle::Response* response) override;
};

} // namespace cacheoracle

#endif // SERVICES_H
