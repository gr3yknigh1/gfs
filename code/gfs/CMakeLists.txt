
function(_gfs_add_library _target_name)

  add_library(${_target_name}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/atlas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/bmp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/entry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/game_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/memory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/physics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/random.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/render_opengl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/static_assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gfs/wave.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/atlas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game_state.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/physics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/random.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render_opengl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wave.c
  )

  if (WIN32)
    target_sources(${_target_name}
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_win32.c
    )
  endif()

  if(UNIX)
    target_sources(${_target_name}
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_linux.c
    )
  endif()

  target_include_directories(${_target_name}
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  target_compile_features(${_target_name}
    PUBLIC
      c_std_17
  )

  if (MSVC)
    set_target_properties(${_target_name}
      PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_link_options(${_target_name}
      PUBLIC
        /DEBUG:FULL
    )

    target_compile_options(${_target_name}
      PRIVATE
        /MP  # Build with multiple processes
        /W4  # Warning level
    )
  endif()

  if(WIN32)
    target_link_libraries(${_target_name}
      PRIVATE
        user32.lib
        kernel32.lib
        shlwapi.lib
        opengl32.dll
    )
  endif()

  target_link_libraries(${_target_name}
    PRIVATE
      glad
  )

  if (GFS_OPENGL_DEBUG)
    target_compile_definitions(gfs
      PUBLIC
        GFS_OPENGL_DEBUG=1
    )
  endif()

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
endfunction()

_gfs_add_library(gfs)
_gfs_add_library(gfs_noentry)

target_compile_definitions(gfs_noentry
  PRIVATE
    GFS_NOENTRY=1
)
