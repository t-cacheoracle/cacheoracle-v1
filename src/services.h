#ifndef SERVICES_H
#define SERVICES_H

#include <grpcpp/grpcpp.h>

#include "cacheoracle.grpc.pb.h"

namespace cacheoracle {

class CodegenServiceImpl final : public ::cacheoracle::CodegenService::Service {
public:
    grpc::Status GeneratePython(grpc::ServerContext* context, const ::cacheoracle::GenerateRequest* request,
                                ::cacheoracle::GenerateResponse* response) override;
};

} // namespace cacheoracle

#endif // SERVICES_H
