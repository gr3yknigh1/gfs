:: Build script
::
:: FILE      build.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin

@echo off


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

set vcpkg_toolchain=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

set project_path=%~dp0
pushd %project_path%

set configuration_path=%project_path%\Build

if exist %configuration_path%\ (
  echo I: Debug configuration already exists!
) else (
  echo I: Debug configuration not found. Generating new one!
  mkdir %configuration_path%
  cmake -G "Ninja Multi-Config" -B %configuration_path% -S %project_path% -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
)

cmake --build %configuration_path%

popd

set /p DUMMY=Hit ENTER to continue...
