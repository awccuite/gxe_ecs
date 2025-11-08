#include "archetype_ecs/ecs.hpp"
#include "archetype_ecs/types.hpp"
#include "archetype_ecs/system.hpp"

#include <chrono>
#include <array>
#include <algorithm>
#include <iostream>
#include <memory>
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

struct EColor {
    size_t col;
};

int main() {
    using namespace gxe;
    using StaticEntity = archetype<Position, Velocity, Lifetime, EColor>;

    constexpr size_t COLOR_COUNT = 21;
    std::array<Color, COLOR_COUNT> colors{
        DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE };

    ecs<StaticEntity> ecs;
    System mover(0); // Framerate independent system
    mover.tickDef([&ecs](float dt){
        ecs.forEachWithComponents<Position, Velocity>([dt](Position& pos, Velocity& vel){
            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
        });
    });

    const float GRAVITY = 9.8f;
    System gravityAppl(15);
    gravityAppl.tickDef([&ecs, GRAVITY](){
        ecs.forEachWithComponents<Velocity>([GRAVITY](Velocity& vel){
            vel.dy += GRAVITY;
        });
    });

    System cleanup(20); // Run every frame.;
    cleanup.tickDef([&ecs](){
        ecs.forEachWithComponents<Position>([&ecs](entityid id, Position& pos){
            if(pos.x < 0 || pos.x > SCREEN_WIDTH || pos.y < 0 || pos.y > SCREEN_HEIGHT){
                ecs.destroyEntity(id);
            }
        });
    });
    
    std::cout << "Created systems" << std::endl;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");
    SetTargetFPS(999);

    // Array of color_textures
    // std::array<RenderTexture, COLOR_COUNT> color_tex;
    // for(size_t i = 0; i < COLOR_COUNT; i++){
        RenderTexture circleTex = LoadRenderTexture(6, 6);
        BeginTextureMode(circleTex);
            DrawCircle(3, 3, 3.0f, WHITE);
        EndTextureMode();
    // }

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
        gravityAppl.update(dt);

        // Spawn timing
        auto spawnStart = std::chrono::high_resolution_clock::now();
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            float x_vel = ((std::rand() % 2000) / 20.0f) - 50.0f;
            float y_vel = ((std::rand() % 2000) / 20.0f) - 50.0f;
            float lifetime = ((std::rand() % 100) / 50.0f) + 2.0f;
            size_t col_index ((std::rand() % 21));

            ecs.createEntity<StaticEntity>(
                Position{mPos.x, mPos.y},
                Velocity{x_vel, y_vel},
                Lifetime{lifetime},
                EColor{col_index}
            );
        }

        if(IsKeyDown(KEY_G)){ // Spawn 100 per frame
            for(uint32_t i = 0; i < 100; i++){
                int x_pos = std::rand() % SCREEN_WIDTH;
                int y_pos = std::rand() % SCREEN_HEIGHT;
                float x_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
                float y_vel = ((std::rand() % 2000) / 50.0f) - 20.0f;
                float lifetime = ((std::rand() % 100) / 50.0f) + 2.0f;
                size_t col_index ((std::rand() % 21));

                ecs.createEntity<StaticEntity>(
                    Position{static_cast<float>(x_pos), static_cast<float>(y_pos)},
                    Velocity{x_vel, y_vel},
                    Lifetime{lifetime},
                    EColor{col_index}
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
        ClearBackground(WHITE);

        // TODO: Replace with drawing system
        auto forEachStart = std::chrono::high_resolution_clock::now();
        ecs.forEachWithComponents<Position, Lifetime, EColor>([&ecs, &circleTex, &colors, dt](entityid id, Position& pos, Lifetime& lt, EColor& ecol){
            DrawTexture(circleTex.texture,
                pos.x - circleTex.texture.width / 2.0f,
                pos.y - circleTex.texture.height / 2.0f,
                colors[ecol.col]
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
