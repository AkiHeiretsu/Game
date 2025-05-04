cmake_minimum_required(VERSION 3.10)
project(SDL2Hangman)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} include)

add_executable(hangman src/main.cpp)
target_link_directories(hangman PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS} ${SDL2_MIXER_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS})
target_link_libraries(hangman PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
