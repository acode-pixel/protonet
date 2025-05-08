from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
import os


class libprotoRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    name = "proto"

    def requirements(self):
        if self.settings.build_type == "Debug":
            self.requires("libuv/[>=1.49.2]")
            self.requires("libsodium/[1.0.20]")
        else:
            self.requires("libuv/[>=1.49.2]", options={"shared": True})
            self.requires("libsodium/[1.0.20]", options={"shared": True})
        
    #def build_requirements(self):
        #self.tool_requires("cmake/[>=3.28.3]")
    
    #def configure(self):
        #print(type(self.options))
        #if self.settings.build_type == "Debug":
        #    self.options.update('libuv/*:shared', False)
        #    self.options.update('libsodium/*:shared', False)
        #else:
        #    self.options.update('libuv/*:shared', True)
        #    self.options.update('libsodium/*:shared', True) 
        
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
    def layout(self):
        #self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
        #self.folders.build = os.path.join("build", str(self.settings.build_type))
        cmake_layout(self)
        
    def package(self):
        cmake = CMake(self)
        cmake.install()
        
    def package_info(self):
        if self.settings.build_type != "Debug":
            self.cpp_info.system_libs = ["libuv", "libsodium"]
        self.cpp_info.libs = ["proto"]
    
    def validate(self):
        if self.settings.os == "Android":
            raise ConanInvalidConfiguration("Android not supported")