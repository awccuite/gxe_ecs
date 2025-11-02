#include "archetype_ecs/ecs.hpp"
#include <chrono>
#include <array>
#include <algorithm>

#include <raylib.h>
#include <cstdlib>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Example component types
struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Health {
    int hp;
};

struct Mass {
    float mass;
};

int main() {
    using namespace gxe;
    
    // Define archetypes
    using MovingEntity = archetype<Position, Velocity>;
    using StaticEntity = archetype<Position>;
    using Damageable = archetype<Position, Health>;
    using PhysicsEntity = archetype<Position, Mass, Velocity>;
    
    // Create ECS with these archetypes
    ecs<MovingEntity, StaticEntity, Damageable, PhysicsEntity> ecs;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    constexpr int FRAME_HISTORY = 100;
    std::array<double, FRAME_HISTORY> frameTimesUs{};
    std::array<double, FRAME_HISTORY> spawnTimesUs{};
    std::array<double, FRAME_HISTORY> forEachTimesUs{};
    int frameIndex = 0;
    int totalFrames = 0;

    while(!WindowShouldClose()){
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        float dt = GetFrameTime();

        // Spawn timing
        auto spawnStart = std::chrono::high_resolution_clock::now();
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            ecs.createEntity<PhysicsEntity>(
                Position{mPos.x, mPos.y},
                Mass{50.0},
                Velocity{5, 15}
            );
        }
        auto spawnEnd = std::chrono::high_resolution_clock::now();
        double currentSpawnTime = std::chrono::duration<double, std::micro>(spawnEnd - spawnStart).count();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ForEach timing
        auto forEachStart = std::chrono::high_resolution_clock::now();
        ecs.forEachWith<PhysicsEntity, Position, Velocity>([&ecs, dt](entityid id, Position& pos, Velocity& vel){
            DrawCircle(pos.x, pos.y, 5, RED);
            pos.y += vel.dy * dt;
            if(pos.y > GetScreenHeight() || pos.y < 0 || pos.x < 0 || pos.x > GetScreenWidth()){
                ecs.destroyEntity(id);
            }
        });
        auto forEachEnd = std::chrono::high_resolution_clock::now();
        double currentForEachTime = std::chrono::duration<double, std::micro>(forEachEnd - forEachStart).count();
        
        auto frameEnd = std::chrono::high_resolution_clock::now();
        double currentFrameTime = std::chrono::duration<double, std::micro>(frameEnd - frameStart).count();
        
        // Store in circular buffer
        frameTimesUs[frameIndex] = currentFrameTime;
        spawnTimesUs[frameIndex] = currentSpawnTime;
        forEachTimesUs[frameIndex] = currentForEachTime;
        frameIndex = (frameIndex + 1) % FRAME_HISTORY;
        totalFrames++;
        
        // Calculate averages over last N frames
        int samplesToAverage = std::min(totalFrames, FRAME_HISTORY);
        double avgFrameTime = 0.0;
        double avgSpawnTime = 0.0;
        double avgForEachTime = 0.0;
        
        for (int i = 0; i < samplesToAverage; ++i) {
            avgFrameTime += frameTimesUs[i];
            avgSpawnTime += spawnTimesUs[i];
            avgForEachTime += forEachTimesUs[i];
        }
        
        avgFrameTime /= samplesToAverage;
        avgSpawnTime /= samplesToAverage;
        avgForEachTime /= samplesToAverage;

        // Display timing stats
        DrawText(TextFormat("Entity Count: %d", ecs.entityCount()), 10, 10, 20, BLACK);
        DrawText(TextFormat("Avg Frame Time (last %d): %.2f us", samplesToAverage, avgFrameTime), 10, 35, 20, BLACK);
        DrawText(TextFormat("Avg Spawn Time (last %d): %.2f us", samplesToAverage, avgSpawnTime), 10, 60, 20, BLACK);
        DrawText(TextFormat("Avg ForEach Time (last %d): %.2f us", samplesToAverage, avgForEachTime), 10, 85, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
    
    return 0;
}
