::
:: FILE      build.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin
::
::                          USAGE
::
:: 1. cmd /c build.bat <build_type>
:: 2. done.
::
:: Build types:
::     - Debug
::     - Release
::

@echo off

set project_path=%~dp0
set configuration_path=%project_path%\build
set build_venv_path=%project_path%\.venv-build"

set build_type=%1
shift

if [%build_type%]==[] set build_type=Debug

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

pushd %project_path%

:: Preparing virtualenv...
:: NOTE(gr3yknigh1): Uncomment code below this if you need to make separate virtual
:: environment for build. [2024/10/25]
python -m venv %build_venv_path%
call %build_venv_path%\Scripts\activate.bat
python -m pip install -r build-requirements.txt
::                       ^^^^^^^^^^^^^^^^^^^^^^.lock
:: TODO(gr3yknigh1): Change to `.lock` file when done with simplebuild [2024/11/01]

:: Compiling project...

if "%build_type%" == "Debug" (
  echo I: Building debug
  conan build . --output-folder %configuration_path% --build=missing --profile %project_path%\conan\msvc-193-x86_64-static-debug
) else (
  if "%build_type%" == "Release" (
    echo I: Building release
    conan build . --output-folder %configuration_path% --build=missing --profile %project_path%\conan\msvc-193-x86_64-static-release
  ) else (
    echo E: Invalid build type: %build_type%
    exit 1
  )
)

if exist %configuration_path%\compile_commands.json (
  echo I: Copying compilation database...
  copy %configuration_path%\compile_commands.json %project_path%\compile_commands.json
)

popd

:: set /p DUMMY=Hit ENTER to continue...
