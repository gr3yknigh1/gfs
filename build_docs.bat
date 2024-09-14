::
:: FILE      build_docs.bat
:: AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
:: COPYRIGHT (c) 2024 Ilya Akkuzin
::
::                               USAGE
::
:: 1. You need to install `docs.requirements.txt` with `pip` package
::    manager. Docs using python tools in order to generate html pages.
::
:: 2. Recommended usage of virtual environment. So activate it, install
::    neccesery dependencies and run this script.
::
:: 3. After successful build, you can serve http server on folder
::    `docs/sphinx/build/html`. For example:
::
::    ```cmd
::    python -m http.server --directory docs/sphinx/build/html 8080
::    ```
::
:: Reference guide: https://rgoswami.me/posts/doc-cpp-dox-sph-exhale/

@echo off

set project_path=%~dp0
set doxygen_folder=%project_path%\docs\doxygen
set sphinx_folder=%project_path%\docs\sphinx

pushd %doxygen_folder%
doxygen Doxyfile
popd

pushd %sphinx_folder%
make html
popd

