#include "archetype_ecs/ecs.hpp"
#include <iostream>

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

int main() {
    using namespace gxe;
    
    // Define archetypes
    using MovingEntity = archetype<Position, Velocity>;
    using StaticEntity = archetype<Position>;
    using Damageable = archetype<Position, Health>;
    
    // Create ECS with these archetypes
    ecs<MovingEntity, StaticEntity, Damageable> ecs;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    while(!WindowShouldClose()){
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            ecs.createEntity<StaticEntity>(
                Position{mPos.x, mPos.y}
            );
        }

        ecs.forEach<StaticEntity>([](entityid id, Position& pos){
            DrawCircle(pos.x, pos.y, 5, RED);
        });

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
    
    return 0;
}
