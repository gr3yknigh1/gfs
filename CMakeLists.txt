cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(gfs 
    VERSION 0.0.1
    LANGUAGES C
)

add_executable(
    ${PROJECT_NAME}
    WIN32
    ${PROJECT_SOURCE_DIR}/gfs_main.c
    ${PROJECT_SOURCE_DIR}/gfs_string.c
    ${PROJECT_SOURCE_DIR}/gfs_win32_bmr.c

    ${PROJECT_SOURCE_DIR}/gfs_color.h
    ${PROJECT_SOURCE_DIR}/gfs_color.c

    ${PROJECT_SOURCE_DIR}/gfs_geometry.h
    ${PROJECT_SOURCE_DIR}/gfs_geometry.c
    
    ${PROJECT_SOURCE_DIR}/gfs_memory.h
    ${PROJECT_SOURCE_DIR}/gfs_memory.c

    ${PROJECT_SOURCE_DIR}/gfs_linalg.h
    ${PROJECT_SOURCE_DIR}/gfs_macros.h

    ${PROJECT_SOURCE_DIR}/gfs_string.h
    ${PROJECT_SOURCE_DIR}/gfs_string.c
    
    ${PROJECT_SOURCE_DIR}/gfs_win32_bmr.h
    ${PROJECT_SOURCE_DIR}/gfs_win32_bmr.c

    ${PROJECT_SOURCE_DIR}/gfs_win32_misc.h
    ${PROJECT_SOURCE_DIR}/gfs_win32_misc.c

    ${PROJECT_SOURCE_DIR}/gfs_types.h
    ${PROJECT_SOURCE_DIR}/gfs_win32_keys.h
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Xinput.lib
)
