cmake_minimum_required(VERSION 3.21)

include(cmake/prelude.cmake)

project(
    ProtoNet-linux
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C
)

include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    ProtoNet-linux_lib OBJECT
    source/lib.c
)

target_include_directories(
    ProtoNet-linux_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(ProtoNet-linux_lib PUBLIC c_std_99)

# ---- Declare executable ----

add_executable(ProtoNet-linux_exe source/main.c)
add_executable(ProtoNet-linux::exe ALIAS ProtoNet-linux_exe)

set_property(TARGET ProtoNet-linux_exe PROPERTY OUTPUT_NAME ProtoNet-linux)

target_compile_features(ProtoNet-linux_exe PRIVATE c_std_99)

target_link_libraries(ProtoNet-linux_exe PRIVATE ProtoNet-linux_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ProtoNet-linux_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ProtoNet-linux"
  )
endif()

include(cmake/dev-mode.cmake)
