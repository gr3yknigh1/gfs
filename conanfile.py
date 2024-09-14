from conan import ConanFile

from conan.tools.meson import Meson
from conan.tools.meson import MesonToolchain


class GFSRecipe(ConanFile):
    name = "gfs"
    package_type = "application"
    version = "0.0.0"

    settings = "os", "arch", "compiler", "build_type"

    def generate(self):
        tc = MesonToolchain(self)
        tc.generate()

    def build(self):
        meson = Meson(self)
        meson.configure()
        meson.build()

