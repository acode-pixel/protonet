cmake_minimum_required(VERSION 3.25.1)
project("tests" C)

set(core_functions init)

set(client_functions connectToNetwork makeFileReq disconnectFromNetwork)
set(server_functions tracCheck re-broadcasting tracPersistence 2C1S 4C1S)
## MingW
if(WIN32)
    set(DLL_PATH "${CMAKE_BINARY_DIR}/protonet-core/libproto.dll")
    set(OUT_PATH "${PROJECT_BINARY_DIR}/libproto.dll")
    set(LIB_PATH "${OUT_PATH}")
    set(OUT_PATH2 "${PROJECT_BINARY_DIR}/libuv.dll")

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(LIBUV_PATH "${libuv_BIN_DIRS_RELEASE}/libuv.dll")
        add_custom_target(copy_libuv
        COMMAND ${CMAKE_COMMAND} -E copy
                ${LIBUV_PATH}
                ${OUT_PATH2}
        COMMENT "Copying libuv library to test folder"
        DEPENDS uv
    )
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LIBSODIUM_PATH "${libsodium_BIN_DIRS_DEBUG}/libsodium-26.dll")
    else()
        set(LIBSODIUM_PATH "${libsodium_BIN_DIRS_RELEASE}/libsodium-26.dll")
    endif()
    set(OUT_PATH2 "${PROJECT_BINARY_DIR}/libsodium-26.dll")

    add_custom_target(copy_libsodium
    COMMAND ${CMAKE_COMMAND} -E copy
            ${LIBSODIUM_PATH}
            ${OUT_PATH2}
    COMMENT "Copying libsodium library to test folder"
    DEPENDS libsodium::libsodium
)

    add_custom_target(copy_lib
    COMMAND ${CMAKE_COMMAND} -E copy
            ${DLL_PATH}
            ${OUT_PATH}
    COMMENT "Copying proto library to test folder"
    DEPENDS proto
)
else()
    set(LIB_PATH "${CMAKE_BINARY_DIR}/protonet-core/libproto.so")
endif()

foreach(i IN LISTS client_functions)
    add_executable(client_test_${i} client_${i}.cpp)
    target_include_directories(client_test_${i} PUBLIC "../protonet-core/")
    target_link_libraries(client_test_${i} ${LIB_PATH} uv)

    if(WIN32)
        add_dependencies(client_test_${i} copy_lib)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_dependencies(client_test_${i} copy_libuv)
            add_dependencies(client_test_${i} copy_libsodium)
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(client_test_${i} PUBLIC DEBUG)
    endif()

    add_test(NAME LIBPROTO_client_${i} COMMAND client_test_${i})
endforeach()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND core_functions failCallback getInterIP sendPck)
endif()

foreach(i IN LISTS core_functions)
    add_executable(core_test_${i} core_${i}.cpp)
    target_link_libraries(core_test_${i} ${LIB_PATH} uv)
    target_include_directories(core_test_${i} PUBLIC "../protonet-core/")

    if(WIN32)
        add_dependencies(core_test_${i} copy_lib)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_dependencies(core_test_${i} copy_libuv)
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
        target_compile_definitions(core_test_${i} PUBLIC DEBUG)
    endif()

    add_test(NAME LIBPROTO_core_${i} COMMAND core_test_${i})
endforeach()

foreach(i IN LISTS server_functions)
    add_executable(server_test_${i} server_${i}.cpp)
    target_link_libraries(server_test_${i} ${LIB_PATH} uv)
    target_include_directories(server_test_${i} PUBLIC "../protonet-core/")

    if(WIN32)
        add_dependencies(server_test_${i} copy_lib)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_dependencies(server_test_${i} copy_libuv)
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
        target_compile_definitions(server_test_${i} PUBLIC DEBUG)
    endif()

    add_test(NAME LIBPROTO_server_${i} COMMAND server_test_${i})
endforeach()