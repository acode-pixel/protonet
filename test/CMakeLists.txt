# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ProtoNet-linuxTests LANGUAGES C)

# ---- Tests ----

add_executable(ProtoNet-linux_test source/ProtoNet-linux_test.c)
target_link_libraries(ProtoNet-linux_test PRIVATE ProtoNet-linux_lib)
target_compile_features(ProtoNet-linux_test PRIVATE c_std_99)

add_test(NAME ProtoNet-linux_test COMMAND ProtoNet-linux_test)

# ---- End-of-file commands ----

add_folders(Test)
