#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Player.h"
#include "UI.h"
#include "TileMap.h"
#include "Camera.h" // Added Camera include

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
    TileMap* tilemap;
    Camera* camera; // Added camera member

    // Screen dimensions - consider moving to a Constants.h or config file
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

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