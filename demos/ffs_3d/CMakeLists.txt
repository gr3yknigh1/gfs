
find_package(cglm REQUIRED)

add_executable(ffs_3d
  WIN32   # XXX
  ${CMAKE_CURRENT_SOURCE_DIR}/mainloop.c
)

target_link_libraries(ffs_3d
  PRIVATE
    cglm::cglm
    glad
    gfs
)

if(MSVC)
  set_target_properties(ffs_3d
    PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )

  target_link_options(ffs_3d
    PUBLIC
      /DEBUG:FULL
  )

  target_compile_options(ffs_3d
    PRIVATE
      /MP  # Build with multiple processes
      /W4  # Warning level
  )
endif()

target_compile_features(ffs_3d
  PRIVATE
    c_std_17
)
