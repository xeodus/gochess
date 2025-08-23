cmake_minimum_required(VERSION 3.20)
project(ChessBit VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED CONFIG)

add_executable(chess 
    src/main.cpp
)

target_link_libraries(chess PRIVATE SDL2::SDL2)
# Some distros also provide SDL2::SDL2main; link it if present (harmless on Linux)
if(TARGET SDL2::SDL2main)
  target_link_libraries(chess PRIVATE SDL2::SDL2main)
endif()
