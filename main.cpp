#include "archetype_ecs/ecs.hpp"
#include "archetype_ecs/systems/physics.hpp"
#include "archetype_ecs/types.hpp"
#include "archetype_ecs/system.hpp"

#include <chrono>
#include <array>
#include <algorithm>
#include <iostream>
#include <cstdlib>

#include <raylib.h>

[[maybe_unused]] const int SCREEN_WIDTH = 1200;
[[maybe_unused]] const int SCREEN_HEIGHT = 800;

int main() {
    using namespace gxe;
    using StaticEntity = archetype<Position, Velocity, Lifetime, EColor>;
    // using PhsyicsEntity = archetype<Position, Velocity, Hitbox2D, EColor>;

    constexpr size_t COLOR_COUNT = 21;
    [[maybe_unused]] std::array<Color, COLOR_COUNT> colors{
        DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE 
    };

    ecs<StaticEntity> ecs;
    ecs.registerSystem<PhysicsSystem>();

    std::cout << "Created systems" << std::endl;

    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    InitWindow(GetScreenWidth(), GetScreenWidth(), "CPU Render");
    SetTargetFPS(999);

    RenderTexture circleTex = LoadRenderTexture(6, 6);
    BeginTextureMode(circleTex);
        DrawCircle(3, 3, 3.0f, WHITE);
    EndTextureMode();


    std::cout << "Initialized raylib" << std::endl;
    std::cout << "Init complete" << std::endl;

    std::srand(std::time({}));
    while(!WindowShouldClose()){
        ecs.step(); // Step the ecs

        // Render Logic
        BeginDrawing(); // Tell raylib we are drawing
        ClearBackground(WHITE);

        EndDrawing(); // Finish drawing commands
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
    
    return 0;
}
