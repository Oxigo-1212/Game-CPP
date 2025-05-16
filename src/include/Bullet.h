#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Constants.h>

class Bullet {
private:
    float x, y;        // Position
    float velocityX, velocityY;
    float rotation;    // Bullet rotation angle
    bool active;
    SDL_Texture* texture;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    static constexpr float SPEED = 800.0f;  // Pixels per second
    static constexpr int BULLET_SIZE = 8;   // Display size of the bullet

public:
    Bullet(SDL_Renderer* renderer, float startX, float startY, float angle);
    ~Bullet();
    
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    bool IsActive() const { return active; }
    void Deactivate() { active = false; }

private:
    bool LoadTexture(SDL_Renderer* renderer);
};
