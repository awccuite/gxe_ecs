cmake_minimum_required(VERSION 3.14)

# Set compilers BEFORE project() command
# Hack for my apple silicon mac.
if(APPLE)
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
endif()

project (gxe_ecs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    main.cpp
    
    archetype_ecs/types.hpp
    archetype_ecs/ecs.hpp
    archetype_ecs/idManager.hpp
    archetype_ecs/idManager.cpp
    archetype_ecs/archetype.hpp
)

find_package(raylib REQUIRED)

add_executable(gxe_ecs ${SOURCE_FILES})

target_compile_definitions(gxe_ecs PRIVATE DEBUG_SIGNATURES=0 DEBUG_ENTITY_DESTRUCTION=0)

target_include_directories(gxe_ecs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(gxe_ecs PRIVATE raylib)