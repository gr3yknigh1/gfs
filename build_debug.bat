:: GFS. Debug build script
::
:: FILE      build_debug.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin

@echo off


set project_path=%~dp0
pushd %project_path%

set configuration_path=%project_path%\Build

if exist %configuration_path%\ (
  echo I: Debug configuration already exists!
) else (
  echo I: Debug configuration not found. Generating new one!
  cmake -S %project_path% -B %configuration_path% -S %project_path% -D CMAKE_EXPORT_COMPILE_COMMANDS=1
)

cmake --build %configuration_path%

popd
