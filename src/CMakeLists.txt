cmake_minimum_required(VERSION 3.31)
project(cacheoracle CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(CURL REQUIRED)

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
set(GENERATED_GRPC_CC ${GENERATED_DIR}/cacheoracle.grpc.pb.cc
        codegen/env_loader.h)
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

add_library(cacheoracle_cache
    cache/cluster_cache_no_program.cpp
    cache/cluster_cache_with_program.cpp
    cache/query_embedding_cache.cpp
    cache/cache_globals.cpp
)

target_include_directories(cacheoracle_cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cacheoracle
    main.cpp
    server.cpp
    services.cpp
    embedlogic/embed_logic.cpp
)

target_include_directories(cacheoracle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GENERATED_DIR})
target_link_libraries(cacheoracle PRIVATE cacheoracle_proto cacheoracle_cache)

add_executable(cacheoracle_client
    client.cpp
)

target_include_directories(cacheoracle_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GENERATED_DIR})
target_link_libraries(cacheoracle_client PRIVATE cacheoracle_proto cacheoracle_cache)

add_executable(cacheoracle_embedlogic
    embedlogic/embed_logic.cpp
)

target_include_directories(cacheoracle_embedlogic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cacheoracle_embedlogic PRIVATE cacheoracle_cache)
target_compile_definitions(cacheoracle_embedlogic PRIVATE EMBEDLOGIC_STANDALONE)

add_executable(cluster_cache_tests
    cache/test_cluster_caches.cpp
)

target_include_directories(cluster_cache_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cluster_cache_tests PRIVATE cacheoracle_cache)

add_executable(llm_test codegen/llm.cpp)
target_link_libraries(llm_test CURL::libcurl)

add_executable(query_embedding_cache_tests
    cache/test_query_embedding_cache.cpp
)

target_include_directories(query_embedding_cache_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(query_embedding_cache_tests PRIVATE cacheoracle_cache)

add_test(NAME cluster_cache_tests COMMAND cluster_cache_tests)
add_test(NAME query_embedding_cache_tests COMMAND query_embedding_cache_tests)
