#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>

class Player {
private:
    float x, y;
    float speed;
    SDL_Texture* texture;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    bool keyStates[SDL_NUM_SCANCODES];
    
    // Animation properties
    std::vector<SDL_Texture*> idleAnimFrames;
    int currentFrame;
    int frameCount;
    float frameTimer;
    float frameDuration;

public:
    Player(SDL_Renderer* renderer, float startX = 400.0f, float startY = 300.0f);
    ~Player();

    void HandleInput(SDL_Event& event);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    
private:
    void LoadTextures(SDL_Renderer* renderer);
};