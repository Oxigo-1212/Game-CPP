#include "include/Bullet.h"
#include "include/Camera.h" // Include Camera for its definition
#include <cmath>
#include <iostream>

Bullet::Bullet(SDL_Renderer* renderer, float startX, float startY, float angle, BulletType bulletType)
    : x(startX), y(startY), rotation(angle), active(true), texture(nullptr), type(bulletType),
      startX(startX), startY(startY), distanceTraveled(0.0f) { // Initialize new members
    
    // Convert angle to radians
    float angleRad = angle * M_PI / 180.0f;
    
    // Set velocity based on angle and speed
    velocityX = SPEED * cos(angleRad);
    velocityY = SPEED * sin(angleRad);
    
    // Load the bullet texture
    if (!LoadTexture(renderer)) {
        std::cerr << "Failed to load bullet texture!" << std::endl;
        active = false;
        return;
    }
}

Bullet::~Bullet() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

bool Bullet::LoadTexture(SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load("assets/bullet.png");
    if (!surface) {
        std::cerr << "Failed to load bullet image: " << IMG_GetError() << std::endl;
        return false;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create bullet texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }

    // Set up source and destination rectangles
    srcRect = {0, 0, surface->w, surface->h};
    destRect = {(int)x - BULLET_SIZE/2, (int)y - BULLET_SIZE/2, BULLET_SIZE, BULLET_SIZE};

    SDL_FreeSurface(surface);
    return true;
}

void Bullet::Update(float deltaTime) {
    if (!active) return;

    // Update position
    float prevX = x;
    float prevY = y;
    x += velocityX * deltaTime;
    y += velocityY * deltaTime;

    // Update distance traveled
    distanceTraveled += std::sqrt(std::pow(x - prevX, 2) + std::pow(y - prevY, 2));

    // Update destination rectangle
    destRect.x = static_cast<int>(x - BULLET_SIZE/2);
    destRect.y = static_cast<int>(y - BULLET_SIZE/2);

    // Deactivate if max distance is reached
    if (distanceTraveled >= MAX_DISTANCE) {
        active = false;
    }

    // Optional: Keep screen boundary check as a fallback or for very large MAX_DISTANCE
    // if (active) { // Only check if still active after distance check
    //     constexpr int margin = 50; 
    //     if (x < -margin || x > Constants::WINDOW_WIDTH + margin || 
    //         y < -margin || y > Constants::WINDOW_HEIGHT + margin) {
    //         active = false;
    //     }
    // }
}

void Bullet::Render(SDL_Renderer* renderer, Camera* camera) { // Modified to take Camera*
    if (!active || !texture || !camera) return;

    // Calculate screen position for the bullet
    SDL_Rect screenDestRect;
    screenDestRect.x = static_cast<int>(destRect.x - camera->GetX());
    screenDestRect.y = static_cast<int>(destRect.y - camera->GetY());
    screenDestRect.w = destRect.w;
    screenDestRect.h = destRect.h;

    SDL_Point center = {screenDestRect.w/2, screenDestRect.h/2};
    SDL_RenderCopyEx(renderer, texture, &srcRect, &screenDestRect, rotation, &center, SDL_FLIP_NONE);
}
