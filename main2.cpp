#include "escapi.h"
#include "raylib.h"

void main()
{
    InitWindow(640, 480, "test");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();
    }
    CloseWindow();
}