install(
    TARGETS ProtoNet-linux_exe
    RUNTIME COMPONENT ProtoNet-linux_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
