:: GFS. Debug run script
::
:: FILE      run_debug.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin

@echo off


set project_path=%~dp0
pushd %project_path%


set configuration_path=%project_path%\Build
set executable_path=%configuration_path%\Debug\gfs.exe

call %project_path%\build_debug.bat

start %executable_path%

popd
