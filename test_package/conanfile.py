from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conan.tools import files


class MicrologTestConan(ConanFile):
    name = "example_package"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    test_type = "explicit"  # Optional: marks this as a test package
    exports_sources = "CMakeLists.txt", "example.cpp"
    requires = "microlog/6.2.2"

    def layout(self):
        cmake_layout(self)
        
    def generate(self):
        tc = CMakeToolchain(self)
        # your options, e.g. tc.cache_variables["ULOG_NO_COLOR"] = not self.options.with_color
        tc.generate()
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
