from simplebuild import Package

package = Package(name="gfs", version="0.0.0")

target = package.add_executable("simplebuild_demo")
target.sources = [
    "demos/simplebuild/main.c"
]
