#include "escapi.h"
#include "raylib.h"

void main()
{
    int width = 640;
    int height = 480;

    InitWindow(width, height, "test");
    
    if (setupESCAPI() < 1)
    {
        TraceLog(LOG_ERROR, "Unable to init ESCAPI");
        return;
    }

    // Setup capture buffer
    SimpleCapParams capture;
    capture.mWidth = width;
    capture.mHeight = height;
    capture.mTargetBuf = new int[width * height];

    // Begin capture
    initCapture(0, &capture);
    doCapture(0);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();

        // Poll webcam
        if (isCaptureDone(0))
        {
            doCapture(0);
        }
    }
    CloseWindow();
}