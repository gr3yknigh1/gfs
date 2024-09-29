
add_library(gfs)

target_sources(gfs
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/bmp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/game.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/game_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/memory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/physics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/render_opengl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/static_assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/wave.h
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game_state.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/physics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_win32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render_opengl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wave.c
)

target_include_directories(gfs
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(gfs
  PUBLIC
    c_std_17
)

if (MSVC)
  set_target_properties(gfs
    PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
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
endif()

if(WIN32)
  target_link_libraries(gfs
    PRIVATE
      user32.lib
      kernel32.lib
      shlwapi.lib
      opengl32.dll
  )
endif()

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

