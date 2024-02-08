cmake_minimum_required(VERSION 3.21)

project(
    ProtoNet-linux
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C
)
enable_testing()
add_test(Protonet-linux_test $<TARGET_FILE:ProtoNet-linux>)
