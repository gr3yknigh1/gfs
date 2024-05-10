:: GFS. Debug build script
::
:: FILE    build_debug.bat
:: AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
:: LICENSE Copyright (c) 2024 Ilya Akkuzin

@echo off

pushd %~dp0
set project_path=%CD%
popd

set configuration_path=%project_dir%\Build\Debug

if exist %configuration_path%\ (
  echo "I: Debug configuration already exists!"
) else (
  echo "I: Debug configuration not found. Generating new one!"
  cmake -B %configuration_path% -S %project_path%
)

cmake --build %configuration_path%
