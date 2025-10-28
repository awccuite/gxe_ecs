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

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    int currentFps = 60;
    gxe::ecs<transform, pos, physics> ecs; // Construct a default ecs.

    while(!WindowShouldClose()){
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 pos = GetMousePosition();
            ecs.createEntity()
                .addComponent<transform>({pos.x, pos.y})
                .addComponent<physics>({50.0f});
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Lets create a simple system that draws over the transform components.
        ecs.forEachEntityWith<transform, physics>([](auto& id, auto& transform, auto& physics){
            DrawCircle(transform.x, transform.y, 5, RED);
            transform.y += 0.05f;
        });

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
}