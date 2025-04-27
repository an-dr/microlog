from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conan.tools import files


class MicrologConan(ConanFile):
    name = "microlog"
    version = "6.2.2"
    license = "MIT"
    author = "Andrei Gramakov"
    url = "https://github.com/an-dr/microlog"
    description = "A simple customizable logging library for C with optional features for embedded and desktop use."
    topics = ("logging", "embedded", "minimal", "color")
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    # generators = "CMakeToolchain", "CMakeDeps"
    options = {
        "with_color": [True, False],
        "with_time": [True, False],
        "with_extra_outputs": [True, False],
    }
    default_options = {
        "with_color": True,
        "with_time": False,
        "with_extra_outputs": False,
    }
    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "include/*",
        "version",
        "LICENSE",
        "cmake/*",
    )

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

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "microlog")
        self.cpp_info.set_property("cmake_target_name", "microlog::microlog")
        self.cpp_info.builddirs.append("cmake/microlog")
