#include <cstdlib>
#include <iostream>
#include <raylib.h>

#include "ecs/ecs.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPU Render");

    int currentFps = 60;
    gxe::ecs ecs; // Construct a default ecs.

    // Base idea, create entity on mouse click, at location (location in transform for now)

    // Break down the idea of a system.
    // A "system", iterates over a component (or a number of components) and
    // updates them given some rules/data. Eg, a "physics system" may apply
    // a gravity effect to each entity that has a "Physics" component.

    while(!WindowShouldClose()){
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){ // Spawn an entity.
            Vector2 pos = GetMousePosition();
            ecs.createEntity().addComponent<gxe::Transform>({pos.x, pos.y, 0.0f, 0.0f});
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Lets create a simple "render" system that draws over the transform components.
        auto transforms = ecs.getSet<gxe::Transform>();
        for(auto& t : transforms->data()){
            auto c = t.component;
            DrawCircle(c.x, c.y, 5.0f, RED);
        }

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
}