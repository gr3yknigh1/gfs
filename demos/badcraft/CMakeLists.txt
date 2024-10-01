find_package(SDL2 REQUIRED)
find_package(cglm REQUIRED)

enable_language(CXX)
add_executable(badcraft
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_link_libraries(badcraft
  PRIVATE
    SDL2::SDL2main
    cglm::cglm

    gfs_noentry
)

