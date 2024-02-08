cmake_minimum_required(VERSION 3.21)

project(
    ProtoNet-linux
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C
)
enable_testing()
target_sources(ProtoNet-linux_exe PUBLIC ${PROJECT_SOURCE_DIR}/source/main.c)
add_test(NAME Test1 COMMAND ProtoNet-linux_exe)
