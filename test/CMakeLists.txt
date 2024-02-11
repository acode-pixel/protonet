cmake_minimum_required(VERSION 3.21)

project(
    ProtoNet-linux_TestAPI
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX C
)
enable_testing()
add_executable(ProtoNet-linux_exe_Test)
target_compile_features(ProtoNet-linux_exe_Test PRIVATE cxx_std_17)
target_link_libraries(ProtoNet-linux_exe_Test PRIVATE ProtoNet-linux_lib)
target_sources(ProtoNet-linux_exe_Test PUBLIC ${PROJECT_SOURCE_DIR}/source/main.cpp)
add_test(NAME Test1 COMMAND ProtoNet-linux_exe_Test)
