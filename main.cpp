#include <SDL.h>
#include "escapi.h"

int main(int argc, char *argv[])
{
    if (setupESCAPI() < 1)             { SDL_Log("Unable to init ESCAPI\n"); return -1; }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { SDL_Log("Unable to init SDL\n");    return -1; }

    const float SCREEN_SCALING = 1.25;
    const int SCREEN_WIDTH = 2560 / SCREEN_SCALING;
    const int SCREEN_HEIGHT = 1440 / SCREEN_SCALING;
    int width = 320;
    int height = 240;
    float size = 1.0f;
    float targetSize = size;
    const float START_X = SCREEN_WIDTH - width - 10;
    const float START_Y = 10;
    float posX = START_X;
    float posY = START_Y;
    float targetX = posX;
    float targetY = posY;
    bool dragging = false;
    float offsetX = 0;
    float offsetY = 0;
    int timeLastFrame = SDL_GetTicks();
    bool dvdMode = false;
    float dvdBeforeX;
    float dvdBeforeY;
    float dvdBeforeSize;
    float dvdDx = 1.0f;
    float dvdDy = -1.0f;

    auto reset = [&]()
    {
        targetX = START_X;
        targetY = START_Y;
        targetSize = 1;
        dvdMode = false;
    };

    auto centre = [&]()
    {
        targetSize = 2;
        targetX = SCREEN_WIDTH / 2 - targetSize * width / 2;
        targetY = SCREEN_HEIGHT / 2 - targetSize * height / 2;
        dvdMode = false;
    };

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
            else if (event.key.keysym.sym == SDLK_LEFT)   targetX = 10;
            else if (event.key.keysym.sym == SDLK_RIGHT)  targetX = SCREEN_WIDTH - width * targetSize - 10;
            else if (event.key.keysym.sym == SDLK_UP)     targetY = 10;
            else if (event.key.keysym.sym == SDLK_DOWN)   targetY = SCREEN_HEIGHT - height * targetSize - 10;
            else if (event.key.keysym.sym == SDLK_r)      reset();
            else if (event.key.keysym.sym == SDLK_c)      centre();
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

        // Handle right click window dragging
        auto scaledWidth = width * size;
        auto scaledHeight = height * size;
        int mx, my;
        auto mouse = SDL_GetGlobalMouseState(&mx, &my);
        auto flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_INPUT_FOCUS && mouse & 4)
        {
            if (!dragging)
            {
                offsetX = mx - scaledWidth/2 - posX;
                offsetY = my - scaledHeight/2 - posY;
                dragging = true;
            }
            targetX = posX = mx - scaledWidth/2 - offsetX;
            targetY = posY = my - scaledHeight/2 - offsetY;
            SDL_SetWindowPosition(window, targetX, targetY);
        }
        else
        {
            dragging = false;
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