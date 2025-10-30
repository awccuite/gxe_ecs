#include <cstdlib>
#include <raylib.h>

#include "ecs/ecs.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

struct transform {
    float x;
    float y;
};

struct pos {
    float x;
    float y;
};

struct physics {
    float mass;
};

struct lifetime {
    float ttl; // TTL in seconds
};

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    int currentFps = 60;
    gxe::ecs<transform, pos, physics, lifetime> ecs; // Construct a default ecs.

    while(!WindowShouldClose()){
        float deltaT = GetFrameTime();

        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 pos = GetMousePosition();
            ecs.createEntity()
                .addComponent<transform>({pos.x, pos.y})
                .addComponent<lifetime>({2.0f});
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        ecs.forEachEntityWith<transform, lifetime>([deltaT, &ecs](auto& id, auto& transform, auto& lifetime){
            DrawCircle(transform.x, transform.y, 5, RED);
            // transform.y += 0.05f;
            lifetime.ttl -= deltaT;
            if(lifetime.ttl <= 0) { ecs.destroyEntity(id); };
        });

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
}