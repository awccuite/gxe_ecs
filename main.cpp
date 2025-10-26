#include <cstdlib>
#include <raylib.h>

#include "ecs/components.hpp"
#include "ecs/ecs.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    int currentFps = 60;
    gxe::ecs ecs; // Construct a default ecs.

    while(!WindowShouldClose()){
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 pos = GetMousePosition();
            ecs.createEntity().addComponent<gxe::transform>({pos.x, pos.y, 0.0f, 0.0f});
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Lets create a simple system that draws over the transform components.
        ecs.forEachEntityWith<gxe::transform>([](auto& id, auto& transform){
            DrawCircle(transform.x, transform.y, 5, RED);
        });

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
}