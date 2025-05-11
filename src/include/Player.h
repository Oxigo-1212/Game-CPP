#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include "Bullet.h"  // Include Bullet header

class Player {
private:
    // Weapon constants
    static constexpr float PISTOL_MUZZLE_OFFSET_X = 50.0f;
    static constexpr float PISTOL_MUZZLE_OFFSET_Y = 28.0f;
    static constexpr float PISTOL_FIRE_RATE = 0.25f;  // Time between shots in seconds

    SDL_Renderer* renderer;  // Store renderer for shooting
    float x, y;
    float speed;
    float rotation;  // Angle in degrees
    int mouseX, mouseY;  // Mouse coordinates
    float muzzleOffsetX;  // Distance from center to muzzle
    float muzzleOffsetY;  // Vertical offset for muzzle
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

    // Shooting properties
    float shootTimer;
    std::vector<Bullet*> bullets;

public:
    Player(SDL_Renderer* renderer, float startX = 400.0f, float startY = 300.0f);
    ~Player();

    void HandleInput(SDL_Event& event);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    void UpdateMousePosition(int x, int y);
    void UpdateBullets(float deltaTime);
    std::vector<Bullet*>& GetBullets() { return bullets; }
    
private:
    void LoadTextures(SDL_Renderer* renderer);
    void RenderAimingLine(SDL_Renderer* renderer);
    void RenderMuzzlePosition(SDL_Renderer* renderer);
    void Shoot(SDL_Renderer* renderer);
};