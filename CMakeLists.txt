cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

project(gfs
  VERSION 0.0.1
  LANGUAGES C
)

find_package(cglm REQUIRED)

add_executable(gfs
  WIN32
  ${PROJECT_SOURCE_DIR}/code/gfs_string.c
  ${PROJECT_SOURCE_DIR}/code/gfs_string.h
  ${PROJECT_SOURCE_DIR}/code/gfs_physics.h
  ${PROJECT_SOURCE_DIR}/code/gfs_physics.c
  ${PROJECT_SOURCE_DIR}/code/gfs_memory.h
  ${PROJECT_SOURCE_DIR}/code/gfs_memory.c
  ${PROJECT_SOURCE_DIR}/code/gfs_string.h
  ${PROJECT_SOURCE_DIR}/code/gfs_string.c
  ${PROJECT_SOURCE_DIR}/code/gfs_wave.h
  ${PROJECT_SOURCE_DIR}/code/gfs_wave.c
  ${PROJECT_SOURCE_DIR}/code/gfs_bmp.h
  ${PROJECT_SOURCE_DIR}/code/gfs_bmp.c
  ${PROJECT_SOURCE_DIR}/code/gfs_static_assert.h
  ${PROJECT_SOURCE_DIR}/code/gfs_assert.h
  ${PROJECT_SOURCE_DIR}/code/gfs_types.h
  ${PROJECT_SOURCE_DIR}/code/gfs_macros.h
  ${PROJECT_SOURCE_DIR}/code/gfs_render.h
  ${PROJECT_SOURCE_DIR}/code/gfs_render.c
  ${PROJECT_SOURCE_DIR}/code/gfs_render_opengl.h
  ${PROJECT_SOURCE_DIR}/code/gfs_render_opengl.c
  ${PROJECT_SOURCE_DIR}/code/gfs_platform.h
  ${PROJECT_SOURCE_DIR}/code/gfs_platform_win32.c
  ${PROJECT_SOURCE_DIR}/code/gfs_game.c
  ${PROJECT_SOURCE_DIR}/code/gfs_game.h
  ${PROJECT_SOURCE_DIR}/code/gfs_game_state.c
  ${PROJECT_SOURCE_DIR}/code/gfs_game_state.h
)

target_link_libraries(gfs
  PRIVATE
    cglm::cglm
)

set_target_properties(gfs
  PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_features(gfs
  PRIVATE
    c_std_17
)

target_link_options(gfs
  PUBLIC
    /DEBUG:FULL
)

target_compile_options(gfs
  PRIVATE
    /MP  # Build with multiple processes
    /W4  # Warning level
)

target_link_libraries(gfs
  PRIVATE
    shlwapi.lib
    opengl32.dll
)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/glad)

target_link_libraries(gfs
  PRIVATE
    glad
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
