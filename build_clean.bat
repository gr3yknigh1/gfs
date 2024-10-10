::
:: FILE      build_clean.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin
::

@echo off

set project_path=%~dp0
set configuration_path=%project_path%\build

pushd %project_path%

if exist %configuration_path% (
    echo I: Deleting configuration directory [%configuration_path%].
    rmdir /Q /S %configuration_path%
) else (
    echo I: Nothing to clean.
)

popd

