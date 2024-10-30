
find_package(freetype REQUIRED)
find_package(cglm REQUIRED)

add_executable(breakout
  WIN32   # XXX
  ${CMAKE_CURRENT_SOURCE_DIR}/breakout_entry.c
  ${CMAKE_CURRENT_SOURCE_DIR}/breakout_render.c
  ${CMAKE_CURRENT_SOURCE_DIR}/breakout_sound.c
)

target_link_libraries(breakout
  PRIVATE
    cglm::cglm
    Freetype::Freetype
    glad
    gfs
)

if(MSVC)
  set_target_properties(breakout
    PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )

  target_link_options(breakout
    PUBLIC
      /DEBUG:FULL
  )

  target_compile_options(breakout
    PRIVATE
      /MP  # Build with multiple processes
      /W4  # Warning level
  )
endif()

target_compile_features(breakout
  PRIVATE
    c_std_17
)
