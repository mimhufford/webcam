#include "escapi.h"
#include "raylib.h"

void main()
{
    int width = 640;
    int height = 480;

    SetTargetFPS(10);
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

    Image image;
    image.data = capture.mTargetBuf;
    image.width = width;
    image.height = height;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    image.mipmaps = 1;

    // Begin capture
    initCapture(0, &capture);
    doCapture(0);

    Texture2D texture = LoadTextureFromImage(image);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RED);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();

        // Poll webcam
        if (isCaptureDone(0))
        {
            UpdateTexture(texture, capture.mTargetBuf);
            doCapture(0);
        }
    }
    CloseWindow();
}