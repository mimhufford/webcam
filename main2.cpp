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

    // @TODO: Need to convert from ABGR to RBGA, so just swizzling for now
    //        There's probably a way to ask escapi for a certain pixel format
    int *swizzled = new int[width * height];

    Image image;
    image.data = swizzled;
    image.width = width;
    image.height = height;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1;

    // Begin capture
    initCapture(0, &capture);
    doCapture(0);

    Texture2D texture = LoadTextureFromImage(image);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(MAGENTA);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();

        // Poll webcam
        if (isCaptureDone(0))
        {
            // Swizzle, hopefully get rid of this
            for (int i = 0; i < width * height; i++)
            {
                int raw = capture.mTargetBuf[i];
                int b = (raw >>  0) & 0xFF;
                int g = (raw >>  8) & 0xFF;
                int r = (raw >> 16) & 0xFF;
                swizzled[i] = 0xFF << 24 | b << 16 | g << 8 | r;
            }

            UpdateTexture(texture, swizzled);
            doCapture(0);
        }
    }
    CloseWindow();
}