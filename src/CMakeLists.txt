cmake_minimum_required(VERSION 3.10)
project(cacheoracle CXX)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/cacheoracle.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

find_program(PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc not found. Please install protobuf compiler.")
endif()
if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "grpc_cpp_plugin not found. Please install gRPC plugins.")
endif()

set(GENERATED_PB_CC ${GENERATED_DIR}/cacheoracle.pb.cc)
set(GENERATED_PB_H  ${GENERATED_DIR}/cacheoracle.pb.h)
set(GENERATED_GRPC_CC ${GENERATED_DIR}/cacheoracle.grpc.pb.cc)
set(GENERATED_GRPC_H  ${GENERATED_DIR}/cacheoracle.grpc.pb.h)

add_custom_command(
  OUTPUT ${GENERATED_PB_CC} ${GENERATED_PB_H} ${GENERATED_GRPC_CC} ${GENERATED_GRPC_H}
  COMMAND ${PROTOC_EXECUTABLE}
          --cpp_out=${GENERATED_DIR}
          --grpc_out=${GENERATED_DIR}
          --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
          -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
          ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
  COMMENT "Generating C++ protocol buffer and gRPC code from ${PROTO_FILE}"
)

add_library(cacheoracle_proto
    ${GENERATED_PB_CC}
    ${GENERATED_GRPC_CC}
)

target_include_directories(cacheoracle_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(cacheoracle_proto PUBLIC gRPC::grpc++ protobuf::libprotobuf)

add_executable(cacheoracle
    main.cpp
    server.cpp
    services.cpp
)

target_include_directories(cacheoracle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GENERATED_DIR})
target_link_libraries(cacheoracle PRIVATE cacheoracle_proto)

add_executable(cacheoracle_client
    client.cpp
)

target_include_directories(cacheoracle_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GENERATED_DIR})
target_link_libraries(cacheoracle_client PRIVATE cacheoracle_proto)
