#include "escapi.h"
#include "raylib.h"
#include "raymath.h"

void main()
{
    Vector2 targetPosition;
    int width = 640;
    int height = 480;

    SetTargetFPS(60);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT);
    InitWindow(height, height, "test");
    
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

    // Load the transparent circle shader
    Shader shader = LoadShader(0, "shader.fs");
    auto sizeLoc = GetShaderLocation(shader, "size");

    // @TODO: Need to convert from ABGR to RBGA, so just swizzling for now
    //        There's probably a way to ask escapi for a certain pixel format
    char *swizzled = new char[width * height * 3];

    Image image;
    image.data = swizzled;
    image.width = width;
    image.height = height;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    image.mipmaps = 1;

    Texture2D texture = LoadTextureFromImage(image);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_LEFT))  targetPosition.x = 0;
        if (IsKeyPressed(KEY_RIGHT)) targetPosition.x = GetMonitorWidth(0) - GetRenderWidth();
        if (IsKeyPressed(KEY_UP))    targetPosition.y = 0;
        if (IsKeyPressed(KEY_DOWN))  targetPosition.y = GetMonitorHeight(0) - GetRenderHeight();
        Vector2 currentPosition = GetWindowPosition();
        currentPosition = Vector2Lerp(currentPosition, targetPosition, GetFrameTime() * 10);
        SetWindowPosition(currentPosition.x, currentPosition.y);
        
        float fSize = (float)GetScreenWidth();
        SetShaderValue(shader, sizeLoc, &fSize, SHADER_UNIFORM_FLOAT);

        BeginDrawing();
        ClearBackground(BLANK);
        BeginShaderMode(shader);
        DrawTexture(texture, 0, 0, WHITE);
        EndShaderMode();
        EndDrawing();

        // Poll webcam
        if (isCaptureDone(0))
        {
            // Swizzle, hopefully get rid of this
            for (int i = 0; i < width * height; i++)
            {
                int raw = capture.mTargetBuf[i];
                swizzled[i * 3 + 0] = (raw >> 16) & 0xFF;
                swizzled[i * 3 + 1] = (raw >>  8) & 0xFF;
                swizzled[i * 3 + 2] = (raw >>  0) & 0xFF;
            }

            UpdateTexture(texture, swizzled);
            doCapture(0);
        }
    }

    UnloadShader(shader);
    UnloadTexture(texture);
    CloseWindow();
}