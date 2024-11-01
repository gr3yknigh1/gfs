from __future__ import annotations
from typing import Optional
from typing import List

from argparse import ArgumentParser
from argparse import Namespace
from dataclasses import dataclass
from dataclasses import field
from enum import IntEnum
from enum import auto
import importlib
import importlib.util
import subprocess
import logging
import os
import os.path
import sys


RECIPE_MODULE_NAME = "__simplebuild_recipe_module__"

@dataclass
class MSVCDevEnv:
    includes: List[str]
    libdirs: List[str]
    paths: List[str]
    executables: Dict[str, List[str]]

# VS2022 `vcvarsall x64` result.
MSVCENV_194_X64 = MSVCDevEnv(
    includes=[
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\include",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\ATLMFC\include",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\include",
        r"C:\Program Files (x86)\Windows Kits\10\include\10.0.22621.0\ucrt",
        r"C:\Program Files (x86)\Windows Kits\10\\include\10.0.22621.0\\um",
        r"C:\Program Files (x86)\Windows Kits\10\\include\10.0.22621.0\\shared",
        r"C:\Program Files (x86)\Windows Kits\10\\include\10.0.22621.0\\winrt",
        r"C:\Program Files (x86)\Windows Kits\10\\include\10.0.22621.0\\cppwinrt",
        r"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\include\um",
    ],
    libdirs=[
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\ATLMFC\lib\x64",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\lib\x64",
        r"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\lib\um\x64",
        r"C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\ucrt\x64",
        r"C:\Program Files (x86)\Windows Kits\10\\lib\10.0.22621.0\\um\x64",
    ],
    # Some of the things are from other default setted PATHs. Remote unused later.
    paths=[
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\bin\HostX64\x64",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VC\VCPackages",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\bin\Roslyn",
        r"C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A\bin\NETFX 4.8 Tools\x64",
        r"C:\Program Files (x86)\HTML Help Workshop",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\FSharp\Tools",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\DiagnosticsHub\Collector",
        r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\\x64",
        r"C:\Program Files (x86)\Windows Kits\10\bin\\x64",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\\MSBuild\Current\Bin\amd64",
        r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools",
        r"C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit",
        r"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VC\Linux\bin\ConnectionManagerExe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg",
    ],
    executables=dict(
        cl=[
            r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\bin\Hostx64\x64\cl.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\cl.exe",
        ],
        link=[
            r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\bin\Hostx64\x64\link.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\link.exe",
        ]
    )
)

class TargetKind(IntEnum):
    EXECUTABLE = auto()
    STATIC_LIBRARY = auto()


def get_target_kind_extention(kind: TargetKind) -> str:
    if kind == TargetKind.EXECUTABLE:
        return "exe"

    if kind == TargetKind.STATIC_LIBRARY:
        return "lib"

    raise Exception("Unreachable") # XXX


class Arch(IntEnum):
    X86_64 = auto()
    X86 = auto()

    @classmethod
    def from_str(cls, s: str) -> Arch:
        if  s == "x86":
            return Arch.X86

        if s == "x86_64":
            return Arch.X86_64

        raise ValueError(f"Invalid `s` value: {s!r}")  # XXX

    def to_str(self) -> str:
        if self.value == Arch.X86:
            return "x86"

        if self.value == Arch.X86_64:
            return "x86_64"

        raise Exception("Unreachable")  # XXX


@dataclass
class Configuration:
    name: str
    arch: Arch


DEFAULT_DEBUG_X86_64_CONFIUGRATION = Configuration(name="Debug", arch=Arch.X86_64)
DEFAULT_RELEASE_X86_64_CONFIUGRATION = Configuration(name="Release", arch=Arch.X86_64)


@dataclass
class Target:
    name: str
    kind: TargetKind

    sources: List[str]
    include_directories: List[str]


@dataclass
class Package:
    name: str
    version: str
    targets: List[Target] = field(default_factory=list)

    def add_executable(self, name: str) -> Target:
        target = Target(name=name, kind=TargetKind.EXECUTABLE, sources=[], include_directories=[])
        self.targets.append(target)
        return target


def _target_build(
    logger: logging.Logger,
    target: Target,
    target_root: str,
    package: Package,
    configuration: Configuration
):
    ...


def _execute_build(logger: logging.Logger, build_root: str, package: Package):
    ...


def _handle_print_help(logger: logging.Logger, args: Namespace):
    logger.debug(f"args={args!r}")

    parser: ArgumentParser = args.parser
    parser.print_help()


def _handle_build(logger: logging.Logger, args: Namespace):
    logger.debug(f"args={args!r}")

    recipe_path: str = args.recipe_path
    build_root: str = args.build_root
    package_variable: str = args.package_variable

    logger.debug("Searching for recipe file...")

    if not os.path.exists(recipe_path):
        raise Exception("Failed to find any recipe file: {recipe_path!r}")  # XXX

    recipe_module_spec = importlib.util.spec_from_file_location(
        RECIPE_MODULE_NAME, recipe_path
    )

    if recipe_module_spec is None or recipe_module_spec.loader is None:
        raise NotImplementedError()  # XXX


    logger.debug("Loading recipe module...")

    recipe_module = importlib.util.module_from_spec(recipe_module_spec)
    sys.modules[RECIPE_MODULE_NAME] = recipe_module
    recipe_module_spec.loader.exec_module(recipe_module)

    logger.debug("Searching for package declaration...")

    package_instance: Optional[Package] = getattr(
        recipe_module, package_variable, None
    )

    if package_instance is None:
        raise Exception(f"Failed to find package variable specified: {package_variable!r}") # XXX

    logger.debug(f"Found package declaration: {package_instance!r}")

    _execute_build(logger, build_root, package_instance)


def main() -> int:
    parser = ArgumentParser(prog="simplebuild")
    parser.add_argument("-R", "--recipe-path", default="recipe.py", dest="recipe_path")
    parser.add_argument("-V", "--package-var", default="package", dest="package_variable")
    parser.set_defaults(handle=_handle_print_help, parser=parser)

    subparsers = parser.add_subparsers()

    help_parser = subparsers.add_parser("help")
    help_parser.set_defaults(handle=_handle_print_help, parser=help_parser)

    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("-B", "--build-root", default="build", dest="build_root")
    build_parser.set_defaults(handle=_handle_build, parser=build_parser)

    logging.basicConfig(level=os.getenv("LOG_LEVEL", "INFO"))
    logger = logging.getLogger("simplebuild")

    args = parser.parse_args()
    args.handle(logger, args)

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
