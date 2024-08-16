cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(gfs
  VERSION 0.0.1
  LANGUAGES C
)

add_executable(
  gfs
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

  ${PROJECT_SOURCE_DIR}/gfs_string.h
  ${PROJECT_SOURCE_DIR}/gfs_string.c

  ${PROJECT_SOURCE_DIR}/gfs_fs.h
  ${PROJECT_SOURCE_DIR}/gfs_fs.c

  ${PROJECT_SOURCE_DIR}/gfs_io.h
  ${PROJECT_SOURCE_DIR}/gfs_io.c

  ${PROJECT_SOURCE_DIR}/gfs_wave.h
  ${PROJECT_SOURCE_DIR}/gfs_wave.c

  ${PROJECT_SOURCE_DIR}/gfs_win32_bmr.h
  ${PROJECT_SOURCE_DIR}/gfs_win32_bmr.c

  ${PROJECT_SOURCE_DIR}/gfs_win32_misc.h
  ${PROJECT_SOURCE_DIR}/gfs_win32_misc.c

  ${PROJECT_SOURCE_DIR}/gfs_win32_keys.h

  ${PROJECT_SOURCE_DIR}/gfs_assert.h
  ${PROJECT_SOURCE_DIR}/gfs_types.h
  ${PROJECT_SOURCE_DIR}/gfs_linalg.h
  ${PROJECT_SOURCE_DIR}/gfs_macros.h
  ${PROJECT_SOURCE_DIR}/gfs_platform.h
  ${PROJECT_SOURCE_DIR}/gfs_win32.c

  ${PROJECT_SOURCE_DIR}/gfs_game.c
  ${PROJECT_SOURCE_DIR}/gfs_game.h

  ${PROJECT_SOURCE_DIR}/gfs_state.c
  ${PROJECT_SOURCE_DIR}/gfs_state.h
)

target_compile_features(
  gfs
  PRIVATE
    c_std_17
)

target_link_options(
  gfs
  PUBLIC
    /DEBUG:FULL
)


target_compile_options(
  gfs
  PRIVATE
    /MP  # Build with multiple processes
    /W4  # Warning level
)

target_link_libraries(
  gfs
  PRIVATE
    shlwapi.lib
)


# TODO(ilya.a): Add unicode support. [2024/05/24]
# target_compile_definitions(
#   ${PROJECT_NAME}
#   PRIVATE UNICODE
# )

# NOTE(ilya.a): Maybe replace dynamic loaded functions from <xinput.h>
# with just static linking? Casey says that dll might be not on the system.
# I think thats this is right solution, because it's, in my humble opinion
# more easy approach. [2024/05/19]
#
# target_link_libraries(
#   ${PROJECT_NAME}
#   PRIVATE Xinput.lib
# )
