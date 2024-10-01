from conan import ConanFile

from conan.tools.cmake import CMake
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMakeToolchain


class GFSRecipe(ConanFile):
    name = "gfs"
    package_type = "application"
    version = "0.0.0"

    settings = "os", "arch", "compiler", "build_type"

    requires = "cglm/0.9.1", "sdl/2.30.7"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

