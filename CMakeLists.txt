cmake_minimum_required(VERSION 3.25.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_testing()

project("protonet")
find_package(libuv CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(libsodium CONFIG REQUIRED)
include_directories(${libuv_INCLUDE_DIR} ${cryptopp_INCLUDE_DIRS})

set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};protonet-core;ALL;/")

add_subdirectory(protonet-core)
add_subdirectory(tests)

#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
