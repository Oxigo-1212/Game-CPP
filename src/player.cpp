#include "include/Player.h"
#include "include/Bullet.h"
#include <iostream>
#include <cmath>

Player::Player(SDL_Renderer* renderer, float startX, float startY) 
    : renderer(renderer), x(startX), y(startY), speed(200.0f), texture(nullptr),
    currentFrame(0), frameCount(20), frameTimer(0), frameDuration(0.05f),
    rotation(0.0f), mouseX(0), mouseY(0), shootTimer(0.0f) {
    
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

    // Clean up bullets
    for (auto bullet : bullets) {
        delete bullet;
    }
    bullets.clear();
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
        case SDL_MOUSEMOTION:
            UpdateMousePosition(event.motion.x, event.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:            if (event.button.button == SDL_BUTTON_LEFT) {
                // Only shoot if enough time has passed
                if (shootTimer <= 0) {
                    Shoot(this->renderer);  // Use the stored renderer
                    shootTimer = PISTOL_FIRE_RATE;  // Reset the timer
                }
            }
            break;
    }
}

void Player::UpdateMousePosition(int x, int y) {
    mouseX = x;
    mouseY = y;
    
    // Calculate rotation angle
    float dx = mouseX - this->x;
    float dy = mouseY - this->y;
    rotation = atan2(dy, dx) * (180.0f / M_PI);
}

void Player::Update(float deltaTime) {
    // Update shoot timer
    if (shootTimer > 0) {
        shootTimer -= deltaTime;
    }

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

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet* bullet = *it;
        bullet->Update(deltaTime);
        
        // Remove inactive bullets
        if (!bullet->IsActive()) {
            delete bullet;
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::Shoot(SDL_Renderer* renderer) {
    // Convert rotation to radians for trigonometry
    float rotationRad = rotation * M_PI / 180.0f;
    
    // Calculate muzzle position
    float muzzleX = x + (PISTOL_MUZZLE_OFFSET_X * cos(rotationRad)) - (PISTOL_MUZZLE_OFFSET_Y * sin(rotationRad));
    float muzzleY = y + (PISTOL_MUZZLE_OFFSET_X * sin(rotationRad)) + (PISTOL_MUZZLE_OFFSET_Y * cos(rotationRad));
    
    // Create new bullet
    Bullet* bullet = new Bullet(renderer, muzzleX, muzzleY, rotation);
    bullets.push_back(bullet);
}

void Player::RenderAimingLine(SDL_Renderer* renderer) {
    // Draw a line from player center to mouse position
    int startX = static_cast<int>(x);
    int startY = static_cast<int>(y);
    
    // Draw the aiming line in red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, startX, startY, mouseX, mouseY);
}

void Player::RenderMuzzlePosition(SDL_Renderer* renderer) {
    // Convert rotation to radians for trigonometry
    float rotationRad = rotation * M_PI / 180.0f;
      // Calculate muzzle position
    float muzzleX = x + (PISTOL_MUZZLE_OFFSET_X * cos(rotationRad)) - (PISTOL_MUZZLE_OFFSET_Y * sin(rotationRad));
    float muzzleY = y + (PISTOL_MUZZLE_OFFSET_X * sin(rotationRad)) + (PISTOL_MUZZLE_OFFSET_Y * cos(rotationRad));
    
    // Draw a small dot at muzzle position
    SDL_Rect muzzleRect = {
        static_cast<int>(muzzleX) - 2,
        static_cast<int>(muzzleY) - 2,
        4, 4
    };
    
    // Draw in bright green for visibility
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &muzzleRect);
}

void Player::Render(SDL_Renderer* renderer) {
    // Render bullets first
    for (auto bullet : bullets) {
        bullet->Render(renderer);
    }

    if (!idleAnimFrames.empty() && currentFrame < idleAnimFrames.size()) {
        // Save the current renderer state
        SDL_Point center = {destRect.w / 2, destRect.h / 2};
        SDL_RenderCopyEx(renderer, idleAnimFrames[currentFrame], &srcRect, &destRect, rotation, &center, SDL_FLIP_NONE);
        
        // Draw the debug aiming line
        RenderAimingLine(renderer);
        
        // Draw the muzzle position
        RenderMuzzlePosition(renderer);
    }
}