#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>

namespace grpc {
class Server;
}

#include "services.h"

namespace cacheoracle {

class GrpcServer {
public:
    explicit GrpcServer(std::string server_address);
    ~GrpcServer();
    // Start the gRPC server (blocks until shutdown)
    void Run();

private:
    std::string server_address_;
    std::unique_ptr<class ::grpc::Server> server_;

    // service implementations
    CodegenServiceImpl codegen_service_;
};

} // namespace cacheoracle

#endif // SERVER_H
