cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(gfs 
    VERSION 0.0.1
    LANGUAGES CXX
)

add_executable(
    ${PROJECT_NAME}
    WIN32
    ${PROJECT_SOURCE_DIR}/gfs_main.cpp
    ${PROJECT_SOURCE_DIR}/gfs_bmr.cpp
    ${PROJECT_SOURCE_DIR}/gfs_string.cpp

    ${PROJECT_SOURCE_DIR}/gfs_bmr.hpp
    ${PROJECT_SOURCE_DIR}/gfs_color.hpp
    ${PROJECT_SOURCE_DIR}/gfs_geom.hpp
    ${PROJECT_SOURCE_DIR}/gfs_lin.hpp
    ${PROJECT_SOURCE_DIR}/gfs_macros.hpp
    ${PROJECT_SOURCE_DIR}/gfs_string.hpp
    ${PROJECT_SOURCE_DIR}/gfs_string_view.hpp
    ${PROJECT_SOURCE_DIR}/gfs_types.hpp
    ${PROJECT_SOURCE_DIR}/gfs_win32_keys.hpp
    ${PROJECT_SOURCE_DIR}/gfs_win32_misc.hpp
    ${PROJECT_SOURCE_DIR}/gfs_win32_scoped_dc.hpp
)
