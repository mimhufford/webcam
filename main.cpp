#include <SDL.h>
#include "escapi.h"

// TODO
//   Use offset so dragging doesn't cause initial snap
//   Arrows to snap to edges

int main(int argc, char *argv[])
{
    if (setupESCAPI() < 1)             { SDL_Log("Unable to init ESCAPI\n"); return -1; }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { SDL_Log("Unable to init SDL\n");    return -1; }

    int width = 320;
    int height = 240;
    float size = 1.0f;
    float targetSize = size;
    float posX = 1920 - width - 10;
    float posY = 10;
    float targetX = posX;
    float targetY = posY;
    int timeLastFrame = SDL_GetTicks();
    bool dvdMode = false;
    float dvdBeforeX;
    float dvdBeforeY;
    float dvdBeforeSize;
    float dvdDx = 1.0f;
    float dvdDy = -1.0f;

    SimpleCapParams capture;
    capture.mWidth = width;
    capture.mHeight = height;
    capture.mTargetBuf = new int[width * height];

    // Begin capture
    initCapture(0, &capture);
    doCapture(0);

    // Setup window
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    auto window   = SDL_CreateWindow("Webcam", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    auto texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    for(bool quit = false; !quit;)
    {
        // Update dt
        int timeNow = SDL_GetTicks();
        float dt = (timeNow - timeLastFrame) / 1000.0f;
        timeLastFrame = timeNow;

        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)                   quit = true;
            else if (event.type == SDL_MOUSEWHEEL)        targetSize += event.wheel.y * 0.1f;
            else if (event.type != SDL_KEYUP)             continue;
            if (event.key.keysym.sym == SDLK_ESCAPE)      quit = true;
            else if (event.key.keysym.sym == SDLK_EQUALS) targetSize += 0.1f;
            else if (event.key.keysym.sym == SDLK_MINUS)  targetSize -= 0.1f;
            else if (event.key.keysym.sym == SDLK_d)
            {
                if (dvdMode)
                {
                    targetX = dvdBeforeX;
                    targetY = dvdBeforeY;
                    targetSize = dvdBeforeSize;
                    dvdMode = false;
                }
                else
                {
                    dvdBeforeX = targetX;
                    dvdBeforeY = targetY;
                    dvdBeforeSize = targetSize;
                    dvdMode = true;
                }
            }
        }

        // Handle mouse window move and resize
        auto scaledWidth = width * size;
        auto scaledHeight = height * size;
        int mx, my;
        auto mouse = SDL_GetGlobalMouseState(&mx, &my);
        auto flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_INPUT_FOCUS && mouse & 4)
        {
            targetX = posX = mx - scaledWidth / 2;
            targetY = posY = my - scaledHeight / 2;
            SDL_SetWindowPosition(window, targetX, targetY);
        }
        
        // Handle DVD mode
        if (dvdMode)
        {
            SDL_DisplayMode dm;
            SDL_GetDesktopDisplayMode(0, &dm);
            float dx = dt * dvdDx * 100;
            float dy = dt * dvdDy * 100;
            if (posX < 0) dvdDx = 1;
            if (posY < 0) dvdDy = 1;
            if (posX > dm.w - scaledWidth)  dvdDx = -1;
            if (posY > dm.h - scaledHeight) dvdDy = -1;
            targetX += dx;
            targetY += dy;
        }

        // Clamp and lerp towards target size
        if (targetSize < 0.1f) targetSize = 0.1f;
        size = size + dt * 20 * (targetSize - size);
        SDL_SetWindowSize(window, scaledWidth, scaledHeight);
        
        // Lerp towards target position
        posX = posX + dt * 20 * (targetX - posX);
        posY = posY + dt * 20 * (targetY - posY);
        SDL_SetWindowPosition(window, posX, posY);

        // Don't hog all CPU time
        SDL_Delay(1);

        // Render webcam texture
        SDL_RenderCopy(renderer, texture, 0, 0);

        // Poll webcam
        if (isCaptureDone(0))
        {
            SDL_UpdateTexture(texture, 0, capture.mTargetBuf, capture.mWidth * 4);
            doCapture(0);
        }

        // Swap buffers
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}