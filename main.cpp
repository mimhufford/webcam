#include <SDL.h>
#include "escapi.h"

// TODO
//   Use offset so dragging doesn't cause initial snap
//   Start in top right, with smaller size
//   Arrows to snap to edges

int main(int argc, char *argv[])
{
    if (setupESCAPI() < 1)             { SDL_Log("Unable to init ESCAPI\n"); return -1; }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { SDL_Log("Unable to init SDL\n");    return -1; }

    int width = 640;
    int height = 480;
    float targetSize = 1.0f;
    float size = 1.0f;

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
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)                   quit = true;
            else if (event.type == SDL_MOUSEWHEEL)        targetSize += event.wheel.y * 0.1f;
            else if (event.type != SDL_KEYUP)             continue;
            if (event.key.keysym.sym == SDLK_ESCAPE)      quit = true;
            else if (event.key.keysym.sym == SDLK_EQUALS) targetSize += 0.1f;
            else if (event.key.keysym.sym == SDLK_MINUS)  targetSize -= 0.1f;
        }
        
        // Clamp target size and lerp size towards target
        if (targetSize < 0.1f) targetSize = 0.1f;
        size = size + 0.2f * (targetSize - size);

        // Handle drag to move
        auto scaledWidth = width * size;
        auto scaledHeight = height * size;
        int x, y;
        auto mouse = SDL_GetGlobalMouseState(&x, &y);
        auto flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_INPUT_FOCUS && mouse & 4) SDL_SetWindowPosition(window, x - scaledWidth / 2, y - scaledHeight / 2);

        SDL_SetWindowSize(window, scaledWidth, scaledHeight);

        // Don't hog all CPU time
        SDL_Delay(10);

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