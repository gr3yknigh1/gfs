#!/bin/bash
#
# FILE      build.sh
# AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
# COPYRIGHT (c) 2024 Ilya Akkuzin
#
#                          USAGE
#
# 1. ./build.sh <build_type>
# 2. done.
#
# Build types:
#     - Debug
#     - Release
#

project_path="$(dirname "$0")"
configuration_path="$project_path/build"
build_venv_path="$project_path/.venv-build"

build_type="$1"
shift

if [ -z "$build_type" ]; then
  build_type="Debug"
fi

pushd "$project_path"

# Preparing virtualenv...
python3 -m venv "$build_venv_path"
source "$build_venv_path/bin/activate"
python3 -m pip install -r build-requirements.txt.lock

# Compiling project...
if [ "$build_type" == "Debug" ]; then
  echo "I: Building debug"
  conan build . --output-folder "$configuration_path" --build=missing --settings build_type=Debug --settings compiler.runtime_type=Debug
elif [ "$build_type" == "Release" ]; then
  echo "I: Building release"
  conan build . --output-folder "$configuration_path" --build=missing --settings build_type=Release --settings compiler.runtime_type=Release
else
  echo "E: Invalid build type: $build_type"
  exit 1
fi

if [ -f "$configuration_path/compile_commands.json" ]; then
  echo "I: Copying compilation database..."
  cp "$configuration_path/compile_commands.json" "$project_path/compile_commands.json"
fi

popd

