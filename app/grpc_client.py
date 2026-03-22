def generate_response(prompt: str) -> str:
    import grpc
    import cacheoracle_pb2
    import cacheoracle_pb2_grpc
    
    channel = grpc.insecure_channel("localhost:50051")
    stub = cacheoracle_pb2_grpc.CodegenServiceStub(channel)

    request = cacheoracle_pb2.GenerateRequest(prompt=prompt)
    response = stub.GenerateResponse(request)

    return response.response