#include "archetype_ecs/ecs.hpp"
#include "archetype_ecs/types.hpp"
#include "archetype_ecs/system.hpp"

#include <chrono>
#include <array>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdlib>

#include <raylib.h>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

// Example component types
struct Position {
    float x, y;
};

struct Velocity { // Needs to be units/sec
    float dx, dy;
};

struct Health {
    int hp;
};

struct Mass {
    float mass;
};

struct Lifetime {
    float ttl;
};

int main() {
    using namespace gxe;
    
    using StaticEntity = archetype<Position, Velocity, Lifetime>;

    ecs<StaticEntity> ecs;
    System mover(120.0f); // System over positions
    mover.tickDef([&ecs](float sysDelta){
        ecs.forEachWithComponents<Position, Velocity>([sysDelta](entityid& id, Position& pos, Velocity& vel){
            pos.x += vel.dx * sysDelta;
            pos.y += vel.dy * sysDelta;
        });
    });

    System cleanup(5.0f);
    cleanup.tickDef([&ecs](){
        ecs.forEachWithComponents<Position>([&ecs](entityid& id, Position& pos){
            if(pos.x < 0 || pos.x > SCREEN_WIDTH || pos.y < 0 || pos.y > SCREEN_HEIGHT){
                ecs.destroyEntity(id);
            }
        });
    });
    
    std::cout << "Created systems" << std::endl;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");
    SetTargetFPS(999);

    RenderTexture circleTex = LoadRenderTexture(6, 6);
    BeginTextureMode(circleTex);
        DrawCircle(3, 3, 3.0f, RED);
        DrawCircleLines(3, 3, 3.0f, BLACK);
    EndTextureMode();
    std::cout << "Initialized raylib" << std::endl;
    
    constexpr int FRAME_HISTORY = 100;
    std::array<double, FRAME_HISTORY> frameTimesUs{};
    std::array<double, FRAME_HISTORY> spawnTimesUs{};
    std::array<double, FRAME_HISTORY> forEachTimesUs{};
    int frameIndex = 0;
    int totalFrames = 0;

    std::cout << "Init complete" << std::endl;

    std::srand(std::time({}));
    while(!WindowShouldClose()){
        auto frameStart = std::chrono::high_resolution_clock::now();
        float dt = GetFrameTime();

        // Want a system manager class.
        mover.update(dt);
        cleanup.update(dt); 

        // Spawn timing
        auto spawnStart = std::chrono::high_resolution_clock::now();
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            float x_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
            float y_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
            float lifetime = ((std::rand() % 100) / 50.0f) + 2.0f;
            ecs.createEntity<StaticEntity>(
                Position{mPos.x, mPos.y},
                Velocity{x_vel, y_vel},
                Lifetime{lifetime}
            );
        }

        if(IsKeyDown(KEY_G)){ // Spawn 100 per frame
            for(uint32_t i = 0; i < 100; i++){
                int x_pos = std::rand() % SCREEN_WIDTH;
                int y_pos = std::rand() % SCREEN_HEIGHT;
                float x_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
                float y_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
                float lifetime = ((std::rand() % 100) / 50.0f) + 2.0f;
                ecs.createEntity<StaticEntity>(
                    Position{static_cast<float>(x_pos), static_cast<float>(y_pos)},
                    Velocity{x_vel, y_vel},
                    Lifetime{lifetime}
                );
            }
        }

        if(IsKeyDown(KEY_R)){
            ecs.forEachWith<StaticEntity>([&ecs](entityid id){
                ecs.destroyEntity(id);
            });
        }

        auto spawnEnd = std::chrono::high_resolution_clock::now();
        double currentSpawnTime = std::chrono::duration<double, std::micro>(spawnEnd - spawnStart).count();

        // Render Logic
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // TODO: Replace with drawing system
        auto forEachStart = std::chrono::high_resolution_clock::now();
        ecs.forEachWithComponents<Position, Lifetime>([&ecs, &circleTex, dt](entityid id, Position& pos, Lifetime& lt){
            DrawTexture(circleTex.texture,
                pos.x - circleTex.texture.width / 2.0f,
                pos.y - circleTex.texture.height / 2.0f,
                RED
            );

            lt.ttl -= dt;
            if(lt.ttl <= 0){
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
        DrawText(TextFormat("FPS: %.0f", 1.0f / dt), 10, 5, 20, BLACK);
        DrawText(TextFormat("Entity Count: %d", ecs.entityCount()), 10, 30, 20, BLACK);
        DrawText(TextFormat("Avg Frame Time (last %d): %.2f us", samplesToAverage, avgFrameTime), 10, 55, 20, BLACK);
        DrawText(TextFormat("Avg Spawn Time (last %d): %.2f us", samplesToAverage, avgSpawnTime), 10, 80, 20, BLACK);
        DrawText(TextFormat("Avg ForEach Time (last %d): %.2f us", samplesToAverage, avgForEachTime), 10, 105, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
    
    return 0;
}
