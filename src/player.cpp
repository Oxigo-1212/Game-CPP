#include "include/Player.h"
#include "include/Bullet.h"
#include <iostream>
#include <cmath>

Player::Player(SDL_Renderer* renderer, float startX, float startY) 
    : renderer(renderer), x(startX), y(startY), speed(200.0f),
    currentFrame(0), frameTimer(0), frameDuration(DEFAULT_FRAME_DURATION),
    rotation(0.0f), mouseX(0), mouseY(0), shootTimer(0.0f), meleeTimer(0.0f),
    isMeleeing(false), currentState(PlayerState::IDLE),
    currentAmmo(MAX_AMMO), isReloading(false), reloadTimer(0.0f),
    currentHealth(STARTING_HEALTH) {
    
    LoadTextures(renderer);
    
    // Initialize key states
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        keyStates[i] = false;
    }
}

Player::~Player() {
    // Clean up all textures
    auto cleanupFrames = [](std::vector<SDL_Texture*>& frames) {
        for (auto tex : frames) {
            if (tex) SDL_DestroyTexture(tex);
        }
        frames.clear();
    };

    cleanupFrames(idleFrames);
    cleanupFrames(moveFrames);
    cleanupFrames(shootFrames);
    cleanupFrames(reloadFrames);
    cleanupFrames(meleeFrames);

    // Clean up bullets
    for (auto bullet : bullets) {
        delete bullet;
    }
    bullets.clear();
}

void Player::LoadAnimationSet(SDL_Renderer* renderer, std::vector<SDL_Texture*>& frames, 
                            const std::string& path, int frameCount) {
    frames.clear();
    
    for (int i = 0; i < frameCount; i++) {
        std::string fullPath = path + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(fullPath.c_str());
        if (!surface) {
            std::cerr << "Failed to load image " << fullPath << ": " << IMG_GetError() << std::endl;
            continue;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        if (!tex) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(surface);
            continue;
        }

        frames.push_back(tex);
        
        // Set up source and destination rectangles using the first frame of idle animation
        if (path.find("idle") != std::string::npos && i == 0) {
            srcRect = {0, 0, surface->w, surface->h};
            destRect = {(int)x - surface->w/4, (int)y - surface->h/4, surface->w/2, surface->h/2}; // Scale down by 50%
        }
        
        SDL_FreeSurface(surface);
    }
}

void Player::LoadTextures(SDL_Renderer* renderer) {
    // Load all animation sets
    LoadAnimationSet(renderer, idleFrames, "assets/player/handgun/idle/survivor-idle_handgun_", IDLE_FRAME_COUNT);
    LoadAnimationSet(renderer, moveFrames, "assets/player/handgun/move/survivor-move_handgun_", MOVE_FRAME_COUNT);
    LoadAnimationSet(renderer, shootFrames, "assets/player/handgun/shoot/survivor-shoot_handgun_", SHOOT_FRAME_COUNT);
    LoadAnimationSet(renderer, reloadFrames, "assets/player/handgun/reload/survivor-reload_handgun_", RELOAD_FRAME_COUNT);
    LoadAnimationSet(renderer, meleeFrames, "assets/player/handgun/meleeattack/survivor-meleeattack_handgun_", MELEE_FRAME_COUNT);
}

std::vector<SDL_Texture*>& Player::GetCurrentAnimationFrames() {
    switch (currentState) {
        case PlayerState::MOVING: return moveFrames;
        case PlayerState::SHOOTING: return shootFrames;
        case PlayerState::RELOADING: return reloadFrames;
        case PlayerState::MELEE: return meleeFrames;
        case PlayerState::IDLE:
        default: return idleFrames;
    }
}

int Player::GetCurrentAnimationFrameCount() const {
    switch (currentState) {
        case PlayerState::MOVING: return MOVE_FRAME_COUNT;
        case PlayerState::SHOOTING: return SHOOT_FRAME_COUNT;
        case PlayerState::RELOADING: return RELOAD_FRAME_COUNT;
        case PlayerState::MELEE: return MELEE_FRAME_COUNT;
        case PlayerState::IDLE:
        default: return IDLE_FRAME_COUNT;
    }
}

void Player::UpdateAnimation(float deltaTime) {
    frameTimer += deltaTime;
    if (frameTimer >= frameDuration) {
        frameTimer = 0;
        currentFrame = (currentFrame + 1) % GetCurrentAnimationFrameCount();
        
        // If we complete a shooting/reloading/melee animation, return to idle
        if (currentFrame == 0 && 
            (currentState == PlayerState::SHOOTING || 
             currentState == PlayerState::RELOADING || 
             currentState == PlayerState::MELEE)) {
            currentState = PlayerState::IDLE;
        }
    }
}

void Player::HandleInput(SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            keyStates[event.key.keysym.scancode] = true;
            if (event.key.keysym.scancode == SDL_SCANCODE_R && !isReloading && currentAmmo < MAX_AMMO) {
                isReloading = true;
                reloadTimer = RELOAD_TIME;
                currentState = PlayerState::RELOADING;
                currentFrame = 0;
            }
            break;
        case SDL_KEYUP:
            keyStates[event.key.keysym.scancode] = false;
            break;
        case SDL_MOUSEMOTION:
            UpdateMousePosition(event.motion.x, event.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && !isReloading) {
                // Only shoot if enough time has passed and we have ammo
                if (shootTimer <= 0 && currentAmmo > 0) {
                    Shoot(this->renderer);
                    shootTimer = PISTOL_FIRE_RATE;
                    currentAmmo--;
                    
                    // Auto-reload when empty
                    if (currentAmmo == 0) {
                        isReloading = true;
                        reloadTimer = RELOAD_TIME;
                        currentState = PlayerState::RELOADING;
                        currentFrame = 0;
                    }
                }
            }
            else if (event.button.button == SDL_BUTTON_RIGHT && !isReloading) {
                // Only melee if enough time has passed
                if (meleeTimer <= 0 && !isMeleeing) {
                    MeleeAttack();
                    meleeTimer = MELEE_COOLDOWN;
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

void Player::Update(float deltaTime) {    // Update timers
    if (shootTimer > 0) {
        shootTimer -= deltaTime;
    }
    
    if (meleeTimer > 0) {
        meleeTimer -= deltaTime;
    }

    // Handle reload timer
    if (isReloading) {
        reloadTimer -= deltaTime;
        if (reloadTimer <= 0) {
            isReloading = false;
            currentAmmo = MAX_AMMO;
            currentState = PlayerState::IDLE;
        }
    }
    
    // Reset melee state when animation completes
    if (isMeleeing && currentState != PlayerState::MELEE) {
        isMeleeing = false;
    }// Handle movement and set state (WASD only)
    bool isMoving = false;
    if (keyStates[SDL_SCANCODE_W]) {
        y -= speed * deltaTime;
        isMoving = true;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        y += speed * deltaTime;
        isMoving = true;
    }
    if (keyStates[SDL_SCANCODE_A]) {
        x -= speed * deltaTime;
        isMoving = true;
    }    if (keyStates[SDL_SCANCODE_D]) {
        x += speed * deltaTime;
        isMoving = true;
    }

    // Update state based on movement
    if (isMoving && currentState == PlayerState::IDLE) {
        currentState = PlayerState::MOVING;
        currentFrame = 0;
    } else if (!isMoving && currentState == PlayerState::MOVING) {
        currentState = PlayerState::IDLE;
        currentFrame = 0;
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
    UpdateAnimation(deltaTime);

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet* bullet = *it;
        bullet->Update(deltaTime);
        
        if (!bullet->IsActive()) {
            delete bullet;
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::Shoot(SDL_Renderer* renderer) {
    currentState = PlayerState::SHOOTING;
    currentFrame = 0;
    frameTimer = 0;

    // Calculate muzzle position and create bullet...
    float rotationRad = rotation * M_PI / 180.0f;
    float muzzleX = x + (PISTOL_MUZZLE_OFFSET_X * cos(rotationRad)) - (PISTOL_MUZZLE_OFFSET_Y * sin(rotationRad));
    float muzzleY = y + (PISTOL_MUZZLE_OFFSET_X * sin(rotationRad)) + (PISTOL_MUZZLE_OFFSET_Y * cos(rotationRad));
    
    Bullet* bullet = new Bullet(renderer, muzzleX, muzzleY, rotation);
    bullets.push_back(bullet);
}

void Player::MeleeAttack() {
    // Switch to melee state and reset animation
    currentState = PlayerState::MELEE;
    currentFrame = 0;
    frameTimer = 0;
    isMeleeing = true;

    // Load knife melee animation if not already loaded
    if (meleeFrames.empty()) {
        LoadAnimationSet(renderer, meleeFrames, "assets/player/knife/meleeattack/survivor-meleeattack_knife_", MELEE_FRAME_COUNT);
    }
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

    // Get the current animation frames
    auto& currentFrames = GetCurrentAnimationFrames();
    if (!currentFrames.empty() && currentFrame < currentFrames.size()) {
        SDL_Point center = {destRect.w / 2, destRect.h / 2};
        SDL_RenderCopyEx(renderer, currentFrames[currentFrame], &srcRect, &destRect, 
                        rotation, &center, SDL_FLIP_NONE);
        
        // Debug visualization
        RenderAimingLine(renderer);
        RenderMuzzlePosition(renderer);
    }
}

void Player::TakeDamage(int amount) {
    currentHealth = std::max(0, currentHealth - amount);
}

void Player::Heal(int amount) {
    currentHealth = std::min(MAX_HEALTH, currentHealth + amount);
}