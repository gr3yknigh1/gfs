find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)


enable_language(CXX)
add_executable(badcraft
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

add_library(imgui_impl
  STATIC
  ${CMAKE_BINARY_DIR}/bindings/imgui_impl_sdl2.cpp
  ${CMAKE_BINARY_DIR}/bindings/imgui_impl_sdl2.h
  ${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl3.cpp
  ${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl3.h
  ${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl3_loader.h
)

target_link_libraries(imgui_impl
  PUBLIC
    imgui::imgui
    SDL2::SDL2
)

target_compile_definitions(imgui_impl
 PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_SDL2
)

target_link_libraries(badcraft
  # External (conan)
  SDL2::SDL2main
  glm::glm
  imgui_impl

  # External (local)
  glad

  # Internal
  gfs_noentry
)

target_include_directories(badcraft
  PRIVATE
    ${CMAKE_BINARY_DIR}/bindings
)

