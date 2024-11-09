import os.path

from conan import ConanFile

from conan.tools.cmake import CMake
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMakeToolchain
from conan.tools.files import copy


class GFSRecipe(ConanFile):
    name = "gfs"
    package_type = "application"
    version = "0.0.0"

    settings = "os", "arch", "compiler", "build_type"

    requires = "cglm/0.9.1", "sdl/2.30.7", "glm/cci.20230113", "imgui/1.91.2", "freetype/2.13.3"

    def generate(self):
        # NOTE(gr3yknigh1): This is stupid
        copy(self, "*sdl2*", os.path.join(self.dependencies["imgui"].package_folder,
            "res", "bindings"), os.path.join(self.source_folder, "build", "bindings"))
        copy(self, "*opengl3*", os.path.join(self.dependencies["imgui"].package_folder,
            "res", "bindings"), os.path.join(self.source_folder, "build", "bindings"))

        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
            "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
            # "GFS_OPENGL_DEBUG": "ON" if self.settings.build_type == "Debug" else "OFF",
        })
        cmake.build()

