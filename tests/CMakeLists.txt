cmake_minimum_required(VERSION 3.25.1)
project("tests" C)

set(core_functions init)
add_executable(core_test_init core_init.cpp)

set(client_functions connectToNetwork)
foreach(i IN LISTS client_functions)
    add_executable(client_test_${i} client_${i}.cpp)
    target_include_directories(client_test_${i} PUBLIC "../protonet-core/")
    target_link_libraries(client_test_${i} ${LIB_PATH} uv)
    add_test(NAME LIBPROTO_client_${i} COMMAND client_test_${i})
endforeach()

if(WIN32)
    set(DLL_PATH "${CMAKE_BINARY_DIR}/protonet-core/libproto.dll")
    set(OUT_PATH "${PROJECT_BINARY_DIR}/libproto.dll")
    set(LIB_PATH "${OUT_PATH}")
    add_custom_target(copy_lib
    COMMAND ${CMAKE_COMMAND} -E copy
            ${DLL_PATH}
            ${OUT_PATH}
    COMMENT "Copying proto library to test folder"
    DEPENDS proto
)
    add_dependencies(core_test_init copy_lib)
else()
    set(LIB_PATH "${CMAKE_BINARY_DIR}/protonet-core/libproto.so")
endif()

target_link_libraries(core_test_init ${LIB_PATH})
target_include_directories(core_test_init PUBLIC "../protonet-core/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(debug_functions getInterIP failCallback sendPck)
    list(APPEND core_functions ${debug_functions})
    foreach(i IN LISTS debug_functions)
        add_executable(core_test_${i} core_${i}.cpp)
        target_include_directories(core_test_${i} PUBLIC "../protonet-core/")
        target_link_libraries(core_test_${i} ${LIB_PATH} uv)
        target_compile_definitions(core_test_${i} PUBLIC DEBUG)
    endforeach()
endif()

foreach(i IN LISTS core_functions)
    add_test(NAME LIBPROTO_core_${i} COMMAND core_test_${i})
endforeach()