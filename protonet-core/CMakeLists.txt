cmake_minimum_required(VERSION 3.25.1)
project("proto" C)
enable_testing()

file(GLOB SOURCES "./*.c" "./*.cpp")
add_library(proto SHARED ${SOURCES})
target_compile_definitions(proto PUBLIC MYLIB_EXPORTS)
target_link_libraries(proto uv cryptopp::cryptopp)
target_include_directories(proto PUBLIC "./")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(proto PUBLIC DEBUG)
    message("-- DEBUG MODE ENABLED")
endif()