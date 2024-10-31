cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

project(gfs
  VERSION 0.0.1
  LANGUAGES C
)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/glad)
add_subdirectory(${PROJECT_SOURCE_DIR}/code/gfs)
add_subdirectory(${PROJECT_SOURCE_DIR}/demos/ffs_3d)
add_subdirectory(${PROJECT_SOURCE_DIR}/demos/breakout)
add_subdirectory(${PROJECT_SOURCE_DIR}/demos/badcraft)

