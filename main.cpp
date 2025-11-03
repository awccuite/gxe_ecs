#include "archetype_ecs/ecs.hpp"
#include "archetype_ecs/types.hpp"
#include "archetype_ecs/system.hpp"

#include <chrono>
#include <array>
#include <algorithm>
#include <random>

#include <raylib.h>
#include <cstdlib>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

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
    
    using StaticEntity = archetype<Position>;

    ecs<StaticEntity> ecs;
    System tickable;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    constexpr int FRAME_HISTORY = 100;
    std::array<double, FRAME_HISTORY> frameTimesUs{};
    std::array<double, FRAME_HISTORY> spawnTimesUs{};
    std::array<double, FRAME_HISTORY> forEachTimesUs{};
    int frameIndex = 0;
    int totalFrames = 0;

    std::srand(std::time({}));

    while(!WindowShouldClose()){
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        float dt = GetFrameTime();

        // Spawn timing
        auto spawnStart = std::chrono::high_resolution_clock::now();
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            ecs.createEntity<StaticEntity>(
                Position{mPos.x, mPos.y}
            );
        }

        if(IsKeyDown(KEY_R)){
            ecs.forEachWith<StaticEntity>([&ecs](entityid id){
                ecs.destroyEntity(id);
            });
        }

        if(IsKeyDown(KEY_G)){
            for(uint32_t i = 0; i < 100; i++){
                int x_pos = std::rand() % SCREEN_WIDTH;
                int y_pos = std::rand() % SCREEN_HEIGHT;
                ecs.createEntity<StaticEntity>(
                    Position{static_cast<float>(x_pos), static_cast<float>(y_pos)}
                );
            }
        }

        // Calculate voronoi diagram over static entites
        // Create a voronoi system, calculate diagram (per frame, investigate offloading generation to thread)
        // On generation complete, we can then draw the generated diagram over our points.
        if(IsKeyDown(KEY_V)){

        }

        auto spawnEnd = std::chrono::high_resolution_clock::now();
        double currentSpawnTime = std::chrono::duration<double, std::micro>(spawnEnd - spawnStart).count();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        auto forEachStart = std::chrono::high_resolution_clock::now();
        ecs.forEachWith<StaticEntity, Position>([&ecs, dt](entityid id, Position& pos){
            DrawCircle(pos.x, pos.y, 2, RED);
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
