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

    while(!WindowShouldClose()){
        ClearBackground(RAYWHITE);
            
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 mPos = GetMousePosition();
            ecs.createEntity<PhysicsEntity>(
                Position{mPos.x, mPos.y},
                Mass{50.0},
                Velocity{-0.1, 0.1}
            );
        }

        ecs.forEach<PhysicsEntity>([&ecs](entityid id, Position& pos, Mass& m, Velocity& vel){
            DrawCircle(pos.x, pos.y, 5, RED);
            pos.y += vel.dy;
            if(pos.y > GetScreenHeight() || pos.y < 0 || pos.x < 0 || pos.x > GetScreenWidth()){
                ecs.destroyEntity(id);
            }
        });

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
    
    return 0;
}
