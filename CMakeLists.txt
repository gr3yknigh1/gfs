cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(GameFromScratch
  VERSION 0.0.1
  LANGUAGES CXX
)

add_executable(GameFromScratch
  WIN32

  ${PROJECT_SOURCE_DIR}/Code/String.cpp
  ${PROJECT_SOURCE_DIR}/Code/String.hpp
  ${PROJECT_SOURCE_DIR}/Code/Physics.hpp
  ${PROJECT_SOURCE_DIR}/Code/Physics.cpp
  ${PROJECT_SOURCE_DIR}/Code/Memory.hpp
  ${PROJECT_SOURCE_DIR}/Code/Memory.cpp
  ${PROJECT_SOURCE_DIR}/Code/WaveAudio.hpp
  ${PROJECT_SOURCE_DIR}/Code/WaveAudio.cpp
  ${PROJECT_SOURCE_DIR}/Code/StaticAssert.hpp
  ${PROJECT_SOURCE_DIR}/Code/Assert.hpp
  ${PROJECT_SOURCE_DIR}/Code/Types.hpp
  ${PROJECT_SOURCE_DIR}/Code/Macros.hpp
  ${PROJECT_SOURCE_DIR}/Code/Render.hpp
  ${PROJECT_SOURCE_DIR}/Code/Render.cpp
  ${PROJECT_SOURCE_DIR}/Code/Platform.hpp
  ${PROJECT_SOURCE_DIR}/Code/PlatformWin32.cpp
  ${PROJECT_SOURCE_DIR}/Code/Game.cpp
  ${PROJECT_SOURCE_DIR}/Code/Game.hpp
  ${PROJECT_SOURCE_DIR}/Code/GameState.cpp
  ${PROJECT_SOURCE_DIR}/Code/GameState.hpp
)

target_include_directories(GameFromScratch
  PRIVATE
    ${PROJECT_SOURCE_DIR}/Code
)

target_compile_features(GameFromScratch
  PRIVATE
    cxx_std_20
)

target_link_options(GameFromScratch
  PUBLIC
    /DEBUG:FULL
)

target_compile_options(GameFromScratch
  PRIVATE
    /MP  # Build with multiple processes
    /W4  # Warning level
)

target_link_libraries(GameFromScratch
  PRIVATE
    shlwapi.lib
)

add_subdirectory(${PROJECT_SOURCE_DIR}/External/glad)

target_link_libraries(GameFromScratch
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
