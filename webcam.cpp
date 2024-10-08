#include "escapi.h"
#include "raylib.h"
#include "raymath.h"

void main()
{
    const int width = 640;
    const int height = 480;

    SetTargetFPS(60);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT);
    InitWindow(height, height, "test");

    Vector2 targetPosition = GetWindowPosition();
    Vector2 targetSize { height, height };

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

    // Set up the GPU texture format
    Image image = {nullptr, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_LEFT))  targetPosition.x -= 25;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_RIGHT)) targetPosition.x += 25;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_UP))    targetPosition.y -= 25;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_DOWN))  targetPosition.y += 25;
        if (IsKeyUp(KEY_LEFT_CONTROL)   && IsKeyPressed(KEY_LEFT))  targetPosition.x = 0;
        if (IsKeyUp(KEY_LEFT_CONTROL)   && IsKeyPressed(KEY_RIGHT)) targetPosition.x = GetMonitorWidth(0) - GetRenderWidth();
        if (IsKeyUp(KEY_LEFT_CONTROL)   && IsKeyPressed(KEY_UP))    targetPosition.y = 0;
        if (IsKeyUp(KEY_LEFT_CONTROL)   && IsKeyPressed(KEY_DOWN))  targetPosition.y = GetMonitorHeight(0) - GetRenderHeight();

        Vector2 currentPosition = GetWindowPosition();
        currentPosition = Vector2Lerp(currentPosition, targetPosition, GetFrameTime() * 10);
        SetWindowPosition(currentPosition.x, currentPosition.y);

        if (IsKeyPressed(KEY_R)) targetSize = { height, height };
        targetSize = Vector2Scale(targetSize, GetMouseWheelMove() * 0.1f + 1);
        Vector2 currentSize = { GetScreenWidth(), GetScreenHeight() };
        currentSize = Vector2Lerp(currentSize, targetSize, GetFrameTime() * 10);
        SetWindowSize(currentSize.x, currentSize.y);

        SetShaderValue(shader, sizeLoc, &currentSize.x, SHADER_UNIFORM_FLOAT);

        BeginDrawing();
        ClearBackground(BLANK);
        BeginShaderMode(shader);
        DrawTextureEx(texture, {}, 0, currentSize.y / height, WHITE);
        EndShaderMode();
        EndDrawing();

        // Poll webcam
        if (isCaptureDone(0))
        {
            for (int i = 0; i < width * height; i++)
            {
                // Swizzle pixel data
                int p = capture.mTargetBuf[i];
                capture.mTargetBuf[i] = 0xFF000000 | (p & 0xFF) << 16 | (p >> 8 & 0xFF) << 8 | p >> 16 & 0xFF;
            }

            UpdateTexture(texture, capture.mTargetBuf);
            doCapture(0);
        }
    }

    UnloadShader(shader);
    UnloadTexture(texture);
    CloseWindow();
}