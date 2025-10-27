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
    ecs/ecs.hpp

    ecs/entities/idManager.hpp
    ecs/entities/idManager.cpp
    ecs/entities/sparseSet.hpp
    ecs/entities/sparseSet.tpp
    ecs/entities/entity.hpp

    ecs/systems/system.hpp
    ecs/systems/coreSystems.hpp
    ecs/systems/systemManager.hpp
)

find_package(raylib REQUIRED)

add_executable(gxe_ecs ${SOURCE_FILES})

target_include_directories(gxe_ecs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(gxe_ecs PRIVATE raylib)