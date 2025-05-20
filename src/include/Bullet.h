#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Constants.h>

class Camera; // Forward declaration

enum class BulletType {
    PISTOL,
    RIFLE,
    SHOTGUN_PELLET
};

class Bullet {
private:
    float x, y;        // Position
    float velocityX, velocityY;
    float rotation;    // Bullet rotation angle
    bool active;
    BulletType type;  // Track what weapon fired this bullet
    SDL_Texture* texture;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    static constexpr float SPEED = 800.0f;  // Pixels per second
    static constexpr int BULLET_SIZE = 8;   // Display size of the bullet

    // For distance-based lifetime
    float startX, startY;         // Initial position of the bullet
    float distanceTraveled;       // Distance traveled from the start position
    static constexpr float MAX_DISTANCE = 1000.0f; // Max distance bullet can travel (e.g., 1000 pixels)

public:
    Bullet(SDL_Renderer* renderer, float startX, float startY, float angle, BulletType bulletType);
    ~Bullet();
    
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer, Camera* camera);
    bool IsActive() const { return active; }
    void Deactivate() { active = false; }
    SDL_Rect GetHitbox() const { return destRect; }
    BulletType GetBulletType() const { return type; }

    // Position and rotation getters
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetRotation() const { return rotation; }

private:
    bool LoadTexture(SDL_Renderer* renderer);
};
