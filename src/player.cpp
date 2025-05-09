#include "include/Player.h"
#include <iostream>

Player::Player(SDL_Renderer* renderer, float startX, float startY) 
    : x(startX), y(startY), speed(200.0f), texture(nullptr),
    currentFrame(0), frameCount(20), frameTimer(0), frameDuration(0.05f) {
    
    LoadTextures(renderer);
    
    // Initialize key states
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        keyStates[i] = false;
    }
}

Player::~Player() {
    // Clean up all textures
    for (SDL_Texture* tex : idleAnimFrames) {
        if (tex) SDL_DestroyTexture(tex);
    }
    idleAnimFrames.clear();
}

void Player::LoadTextures(SDL_Renderer* renderer) {
    for (int i = 0; i < frameCount; i++) {
        std::string path = "assets/player/handgun/idle/survivor-idle_handgun_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Failed to load image " << path << ": " << IMG_GetError() << std::endl;
            continue;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        if (!tex) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(surface);
            continue;
        }

        idleAnimFrames.push_back(tex);
        
        // Set up source and destination rectangles using the first frame
        if (i == 0) {
            srcRect = {0, 0, surface->w, surface->h};
            destRect = {(int)x - surface->w/4, (int)y - surface->h/4, surface->w/2, surface->h/2}; // Scale down by 50%
        }
        
        SDL_FreeSurface(surface);
    }
}

void Player::HandleInput(SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            keyStates[event.key.keysym.scancode] = true;
            break;
        case SDL_KEYUP:
            keyStates[event.key.keysym.scancode] = false;
            break;
    }
}

void Player::Update(float deltaTime) {
    // Handle movement based on key states
    if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_UP]) {
        y -= speed * deltaTime;
    }
    if (keyStates[SDL_SCANCODE_S] || keyStates[SDL_SCANCODE_DOWN]) {
        y += speed * deltaTime;
    }
    if (keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_LEFT]) {
        x -= speed * deltaTime;
    }
    if (keyStates[SDL_SCANCODE_D] || keyStates[SDL_SCANCODE_RIGHT]) {
        x += speed * deltaTime;
    }

    // Update destination rectangle position
    destRect.x = static_cast<int>(x - destRect.w/2);
    destRect.y = static_cast<int>(y - destRect.h/2);

    // Keep player within window bounds
    destRect.x = std::max(0, std::min(destRect.x, 800 - destRect.w));
    destRect.y = std::max(0, std::min(destRect.y, 600 - destRect.h));
    x = destRect.x + destRect.w/2;
    y = destRect.y + destRect.h/2;

    // Update animation
    frameTimer += deltaTime;
    if (frameTimer >= frameDuration) {
        frameTimer = 0;
        currentFrame = (currentFrame + 1) % frameCount;
    }
}

void Player::Render(SDL_Renderer* renderer) {
    if (!idleAnimFrames.empty() && currentFrame < idleAnimFrames.size()) {
        SDL_RenderCopy(renderer, idleAnimFrames[currentFrame], &srcRect, &destRect);
    }
}