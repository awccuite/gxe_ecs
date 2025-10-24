cmake_minimum_required(VERSION 3.14)

project (cpu_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    main.cpp
    ecs/components.hpp
    ecs/idManager.hpp
    ecs/idManager.cpp
    ecs/sparseSet.hpp
    ecs/entity.hpp
    ecs/ecs.hpp
)

find_package(raylib REQUIRED)

add_executable(cpu_renderer ${SOURCE_FILES})

target_include_directories(cpu_renderer PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(cpu_renderer PRIVATE raylib)