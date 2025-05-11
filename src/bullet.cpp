#include "include/Bullet.h"
#include <cmath>
#include <iostream>

Bullet::Bullet(SDL_Renderer* renderer, float startX, float startY, float angle)
    : x(startX), y(startY), rotation(angle), active(true), texture(nullptr) {
    
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
    x += velocityX * deltaTime;
    y += velocityY * deltaTime;

    // Update destination rectangle
    destRect.x = static_cast<int>(x - BULLET_SIZE/2);
    destRect.y = static_cast<int>(y - BULLET_SIZE/2);

    // Deactivate if off screen (with some margin)
    if (x < -50 || x > 850 || y < -50 || y > 650) {
        active = false;
    }
}

void Bullet::Render(SDL_Renderer* renderer) {
    if (!active || !texture) return;

    // Render the bullet with rotation
    SDL_Point center = {BULLET_SIZE/2, BULLET_SIZE/2};
    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, rotation, &center, SDL_FLIP_NONE);
}
