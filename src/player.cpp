#include "include/Player.h"
#include "include/Bullet.h"
#include "include/Camera.h" // Include Camera for its definition
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
    // First, clean up any existing textures in the frames vector
    for (auto tex : frames) {
        if (tex) {
            SDL_DestroyTexture(tex);
        }
    }
    frames.clear(); // Now clear the vector of pointers
    
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
            // destRect is now relative to player's x,y center. x,y are world coords.
            // The actual rendering position will be (world_x - camera_x, world_y - camera_y)
            // For now, destRect.w and destRect.h store dimensions.
            // destRect.x and .y will be updated in Update() based on world x,y
            destRect.w = surface->w / 2; // Scale down by 50%
            destRect.h = surface->h / 2; // Scale down by 50%
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

        int frameCount = GetCurrentAnimationFrameCount();
        if (frameCount > 0) {
            currentFrame = (currentFrame + 1) % frameCount;

            // If we complete a shooting/reloading/melee animation, return to idle
            if (currentFrame == 0) { // Animation cycle completed
                if (currentState == PlayerState::SHOOTING ||
                    currentState == PlayerState::RELOADING) {
                    currentState = PlayerState::IDLE;
                } else if (currentState == PlayerState::MELEE) {
                    currentState = PlayerState::IDLE;
                    isMeleeing = false; // Reset isMeleeing flag
                    std::cout << "Melee animation completed in UpdateAnimation. isMeleeing set to false. meleeTimer: " << meleeTimer << std::endl;
                }
            }
        } else {
            // If current animation has no frames, revert to IDLE and set frame to 0
            currentFrame = 0;
            if (currentState != PlayerState::IDLE) { // Avoid issues if IDLE itself has 0 frames
                if (currentState == PlayerState::MELEE) {
                    isMeleeing = false; // Also reset here if melee had 0 frames
                    std::cout << "Melee state with 0 frames ended in UpdateAnimation. isMeleeing set to false." << std::endl;
                }
                currentState = PlayerState::IDLE;
            }
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
            else if (event.button.button == SDL_BUTTON_RIGHT) {
                std::cout << "Right Mouse Button Down. isReloading: " << isReloading 
                          << ", meleeTimer: " << meleeTimer 
                          << ", isMeleeing: " << isMeleeing << std::endl;
                if (!isReloading) { // Check !isReloading before attempting melee
                    // Only melee if enough time has passed
                    if (meleeTimer <= 0 && !isMeleeing) {
                        std::cout << "Attempting MeleeAttack()" << std::endl;
                        MeleeAttack();
                        meleeTimer = MELEE_COOLDOWN; // Set cooldown AFTER initiating attack
                    } else {
                        if (meleeTimer > 0) std::cout << "Melee cooldown active. Time left: " << meleeTimer << std::endl;
                        if (isMeleeing) std::cout << "Already meleeing (isMeleeing is true)." << std::endl;
                    }
                } else {
                     std::cout << "Cannot melee: Currently reloading (isReloading is true)." << std::endl;
                }
            }
            break;
    }
}

void Player::UpdateMousePosition(int worldMouseX, int worldMouseY) {
    // mouseX and mouseY are now world coordinates, passed from Game.cpp after camera conversion
    mouseX = worldMouseX;
    mouseY = worldMouseY;
    
    // Calculate rotation angle based on player's world position and world mouse position
    float dx = worldMouseX - GetCenterX(); // Use player's center for more accurate aiming
    float dy = worldMouseY - GetCenterY();
    rotation = atan2(dy, dx) * (180.0f / M_PI);
}

void Player::Update(float deltaTime) { // MODIFIED - Removed mapPixelWidth, mapPixelHeight
    // Update timers
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
            // Only set to IDLE if not currently shooting or meleeing, 
            // though typically reload completion should interrupt those.
            if (currentState == PlayerState::RELOADING) { // Ensure we only switch from RELOADING
                currentState = PlayerState::IDLE;
            }
        }
    }
    
    // Reset melee state if currentState changed from MELEE for other reasons (fallback)
    // Primary reset is in UpdateAnimation when melee animation cycle finishes.
    if (isMeleeing && currentState != PlayerState::MELEE) {
        isMeleeing = false;
        std::cout << "isMeleeing reset in Player::Update() because currentState (" << static_cast<int>(currentState) 
                  << ") is no longer MELEE. meleeTimer: " << meleeTimer << std::endl;
    }

    float moveX = 0.0f;
    float moveY = 0.0f;

    // Player can now move while reloading or meleeing
    if (keyStates[SDL_SCANCODE_W]) {
        moveY -= 1;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        moveY += 1;
    }
    if (keyStates[SDL_SCANCODE_A]) {
        moveX -= 1;
    }
    if (keyStates[SDL_SCANCODE_D]) {
        moveX += 1;
    }

    // Normalize movement vector
    float length = sqrt(moveX * moveX + moveY * moveY);
    if (length > 0) {
        moveX /= length;
        moveY /= length;
        // If not performing a higher priority action, set state to MOVING
        if (!isReloading && !isMeleeing && currentState != PlayerState::SHOOTING && currentState != PlayerState::RELOADING && currentState != PlayerState::MELEE) {
            currentState = PlayerState::MOVING;
        }
    } else {
        // If not moving and not performing a higher priority action, set state to IDLE
        if (!isReloading && !isMeleeing && currentState != PlayerState::SHOOTING && currentState != PlayerState::RELOADING && currentState != PlayerState::MELEE) {
            currentState = PlayerState::IDLE;
        }
    }

    // Update player position
    x += moveX * speed * deltaTime;
    y += moveY * speed * deltaTime;

    // Update destRect for rendering based on new x, y (world coordinates)
    // The camera will handle the offset for screen coordinates in Render()
    destRect.x = static_cast<int>(x - destRect.w / 2.0f); // x is center
    destRect.y = static_cast<int>(y - destRect.h / 2.0f); // y is center

    // Update animation
    UpdateAnimation(deltaTime);

    // Update bullets (passing camera is not strictly needed here unless bullets need it for logic)
    // Bullets will use camera in their own Render methods.
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

void Player::RenderAimingLine(SDL_Renderer* renderer, Camera* camera) { // Added camera param
    // Player's center in world coordinates
    float playerWorldCenterX = GetCenterX();
    float playerWorldCenterY = GetCenterY();

    // Convert player's world center to screen coordinates
    SDL_FPoint playerScreenCenter = camera->WorldToScreen(playerWorldCenterX, playerWorldCenterY);

    // Mouse position is already in world coordinates, convert to screen coordinates for rendering the line end point
    SDL_FPoint mouseScreenPos = camera->WorldToScreen(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, 
                       static_cast<int>(playerScreenCenter.x), 
                       static_cast<int>(playerScreenCenter.y), 
                       static_cast<int>(mouseScreenPos.x), 
                       static_cast<int>(mouseScreenPos.y));
}

void Player::RenderMuzzlePosition(SDL_Renderer* renderer, Camera* camera) { // Added camera param
    // Convert rotation to radians for trigonometry
    float rotationRad = rotation * M_PI / 180.0f;
    // Calculate muzzle position in world coordinates
    float worldMuzzleX = GetCenterX() + (PISTOL_MUZZLE_OFFSET_X * cos(rotationRad)) - (PISTOL_MUZZLE_OFFSET_Y * sin(rotationRad));
    float worldMuzzleY = GetCenterY() + (PISTOL_MUZZLE_OFFSET_X * sin(rotationRad)) + (PISTOL_MUZZLE_OFFSET_Y * cos(rotationRad));
    
    // Convert world muzzle position to screen position
    SDL_FPoint screenMuzzlePos = camera->WorldToScreen(worldMuzzleX, worldMuzzleY);

    SDL_Rect muzzleRect = {
        static_cast<int>(screenMuzzlePos.x) - 2,
        static_cast<int>(screenMuzzlePos.y) - 2,
        4, 4
    };
    
    // Draw in bright green for visibility
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &muzzleRect);
}

void Player::Render(SDL_Renderer* renderer, Camera* camera) { // Modified to take Camera*
    // Render bullets first, passing the camera
    for (auto bullet : bullets) {
        bullet->Render(renderer, camera); // Pass camera to bullet's render method
    }

    // Get the current animation frames
    auto& currentFrames = GetCurrentAnimationFrames();
    if (!currentFrames.empty() && currentFrame < currentFrames.size()) {
        SDL_Texture* currentTexture = currentFrames[currentFrame];
        
        // Dynamically create the source rectangle based on the current texture's dimensions
        SDL_Rect currentFrameSrcRect; 
        SDL_QueryTexture(currentTexture, NULL, NULL, &currentFrameSrcRect.w, &currentFrameSrcRect.h);
        currentFrameSrcRect.x = 0;
        currentFrameSrcRect.y = 0;

        // Dynamically set destRect dimensions based on the current texture, scaled
        float scale = 0.5f; // Your desired scale factor
        destRect.w = static_cast<int>(currentFrameSrcRect.w * scale);
        destRect.h = static_cast<int>(currentFrameSrcRect.h * scale);

        // Calculate screen position for the player
        SDL_Rect screenDestRect;
        screenDestRect.x = static_cast<int>((x - destRect.w / 2.0f) - camera->GetX());
        screenDestRect.y = static_cast<int>((y - destRect.h / 2.0f) - camera->GetY());        screenDestRect.w = destRect.w; 
        screenDestRect.h = destRect.h; 

        /*
        // DEBUGGING OUTPUT START
        if (currentState == PlayerState::MELEE || currentState == PlayerState::RELOADING || currentState == PlayerState::IDLE) { // Added IDLE for comparison
            std::cout << "State: ";
            switch (currentState) {
                case PlayerState::IDLE: std::cout << "IDLE"; break;
                case PlayerState::MELEE: std::cout << "MELEE"; break;
                case PlayerState::RELOADING: std::cout << "RELOADING"; break;
                default: std::cout << (int)currentState; break;
            }
            std::cout << " Frame: " << currentFrame
                      << " TexW: " << currentFrameSrcRect.w << " TexH: " << currentFrameSrcRect.h
                      << " DestW: " << destRect.w << " DestH: " << destRect.h
                      << " ScreenDestW: " << screenDestRect.w << " ScreenDestH: " << screenDestRect.h
                      << std::endl;
        }
        // DEBUGGING OUTPUT END
        */

        SDL_Point center = {screenDestRect.w / 2, screenDestRect.h / 2};
        SDL_RenderCopyEx(renderer, currentTexture, &currentFrameSrcRect, &screenDestRect, 
                        rotation, &center, SDL_FLIP_NONE);
        
        // Debug visualization, pass camera
        RenderAimingLine(renderer, camera);
        RenderMuzzlePosition(renderer, camera);
    }
}

float Player::GetCenterX() const {
    return x; // x is already the center
}

float Player::GetCenterY() const {
    return y; // y is already the center
}

void Player::UpdateBullets(float deltaTime, Camera* camera) { // Added camera param, though not used in current bullet update logic
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet* bullet = *it;
        bullet->Update(deltaTime); // Bullet update logic doesn't currently need camera
        
        if (!bullet->IsActive()) {
            delete bullet;
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::TakeDamage(int amount) {
    currentHealth = std::max(0, currentHealth - amount);
}

void Player::Heal(int amount) {
    currentHealth = std::min(MAX_HEALTH, currentHealth + amount);
}