from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain

class MicrologTestConan(ConanFile):
    name = "example_package"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    test_type = "explicit"  # Optional: marks this as a test package
    exports_sources = "CMakeLists.txt", "example.cpp"
    requires = "microlog/6.1.0"
    

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

