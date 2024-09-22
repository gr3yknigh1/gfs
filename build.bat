:: FILE      build.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin

@echo off

:: Detect vcvarsall for x64 build...
set vc2022_bootstrap="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set vc2019_bootstrap="C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat"

if exist %vc2022_bootstrap% (
  echo I: Found VC 2022 bootstrap script!
  call %vc2022_bootstrap% amd64
) else (
  if exist %vc2019_bootstrap% (
    echo I: No script for VC 2022, but found VC 2019 bootstrap script!
    call %vc2019_bootstrap% amd64
  ) else (
    echo W: Failed to find nor VC 2019, nor VC 2022 bootstrap scripts!
  )
)

:: Compiling project...
set project_path=%~dp0
pushd %project_path%

set configuration_path=%project_path%\build

conan install . --output-folder %configuration_path% --build=missing --profile msvc-193-x86_64-static-ninja --settings build_type=Debug --settings compiler.runtime_type=Debug
cmake --preset conan-default -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build --preset conan-debug

if exist %configuration_path%\compile_commands.json (
  echo I: Copying compilation database...
  copy %configuration_path%\compile_commands.json %project_path%\compile_commands.json
)

popd

:: set /p DUMMY=Hit ENTER to continue...
