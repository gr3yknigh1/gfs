
find_package(cglm REQUIRED)

add_executable(badcraft
  WIN32   # XXX
  ${CMAKE_CURRENT_SOURCE_DIR}/mainloop.c
)

target_link_libraries(badcraft
  PRIVATE
    cglm::cglm
    glad
    gfs
)

if(MSVC)
  set_target_properties(badcraft
    PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )

  target_link_options(badcraft
    PUBLIC
      /DEBUG:FULL
  )

  target_compile_options(badcraft
    PRIVATE
      /MP  # Build with multiple processes
      /W4  # Warning level
  )
endif()

target_compile_features(badcraft
  PRIVATE
    c_std_17
)
