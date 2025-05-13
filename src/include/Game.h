#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Player.h"
#include "UI.h"

class Game {
private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    // Timing variables
    const int FPS;
    const int FRAME_TIME;
    Uint32 previousTime;
    float accumulator;
    const float FIXED_TIME_STEP;

    // Game objects
    Player* player;
    UI* ui;

public:
    Game();
    ~Game();
    
    bool Initialize();
    void HandleEvents();
    void Update(float deltaTime);
    void Render();
    void Run();
    void Cleanup();
};