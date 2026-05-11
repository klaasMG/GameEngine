cmake_minimum_required(VERSION 3.18)
project(GameEngine)

set(CMAKE_CXX_STANDARD 23)
macro(add_glad)
    target_sources(GameEngine PRIVATE vendor/glad/src/glad.c)
endmacro()

macro(add_glfw)
    add_subdirectory(vendor/glfw)
    target_link_libraries(GameEngine PRIVATE glfw)
endmacro()

macro(add_EnTT)
    include(FetchContent)
    FetchContent_Declare(EnTT GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.12.2)
    FetchContent_MakeAvailable(EnTT)
    target_link_libraries(GameEngine PRIVATE EnTT::EnTT)
endmacro()

add_library(common_lib
        lib/Error.cpp
        lib/Error.h
)

target_include_directories(common_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/cpp_base
)

add_executable(GameEngine src/main.cpp
        src/renderer.cpp
        src/renderer.h
        src/game_simulation.cpp)

target_link_libraries(GameEngine PRIVATE common_lib)

target_include_directories(GameEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)
add_glad()
add_glfw()
add_EnTT()
message(STATUS "build the project")
message(STATUS "dj")
