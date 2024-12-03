#include "raylib.h"
#include "network.h"

int main(void) 
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);           
    //SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        // Update
        // example_network_function();

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();     
	return 0;
}
