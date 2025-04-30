cmake_minimum_required(VERSION 3.25.1)
enable_testing()

project("protonet")
find_package(libuv CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
include_directories(${libuv_INCLUDE_DIR} ${cryptopp_INCLUDE_DIRS})

add_subdirectory(protonet-core)
add_subdirectory(tests)