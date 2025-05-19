#include "include/Player.h"
#include "include/Bullet.h"
#include "include/Camera.h"
#include <iostream>
#include <cmath>

Player::Player(SDL_Renderer* renderer, float startX, float startY) 
    : renderer(renderer), x(startX), y(startY), speed(200.0f),
    currentFrame(0), frameTimer(0), frameDuration(DEFAULT_FRAME_DURATION),
    rotation(0.0f), mouseX(0), mouseY(0), shootTimer(0.0f), meleeTimer(0.0f),
    isMeleeing(false), currentState(PlayerState::IDLE),
    currentWeapon(WeaponType::PISTOL), isMouseDown(false), isReloading(false), 
    reloadTimer(0.0f), pistolAmmo(PISTOL_MAX_AMMO), rifleAmmo(RIFLE_MAX_AMMO), shotgunAmmo(SHOTGUN_MAX_AMMO),
    currentHealth(STARTING_HEALTH) {
    
    // Initialize key states
    for (bool& state : keyStates) {
        state = false;
    }

    // Initialize random seed for shotgun spread
    srand(static_cast<unsigned>(time(nullptr)));

    // Set up source and destination rectangles    srcRect = {0, 0, 0, 0};
    destRect = {0, 0, 48, 48}; // Player size reduced from 64x64 to 48x48 pixels

    // First load all weapon animations
    LoadTextures(renderer);

    // Then set the current weapon's animation references
    idleFrames = idleAnimations[currentWeapon];
    moveFrames = moveAnimations[currentWeapon];
    shootFrames = shootAnimations[currentWeapon];
    reloadFrames = reloadAnimations[currentWeapon];
    meleeFrames = meleeAnimations[currentWeapon];
}

Player::~Player() {
    // Clean up all textures
    auto cleanupFrames = [](std::vector<SDL_Texture*>& frames) {
        for (auto tex : frames) {
            if (tex) SDL_DestroyTexture(tex);
        }
        frames.clear();
    };

    // Clean up all weapon animations
    for (WeaponType weapon : {WeaponType::PISTOL, WeaponType::RIFLE, WeaponType::SHOTGUN}) {
        cleanupFrames(idleAnimations[weapon]);
        cleanupFrames(moveAnimations[weapon]);
        cleanupFrames(shootAnimations[weapon]);
        cleanupFrames(reloadAnimations[weapon]);
        cleanupFrames(meleeAnimations[weapon]);
    }

    // Clean up bullets
    for (auto bullet : bullets) {
        delete bullet;
    }
    bullets.clear();
}

void Player::LoadAnimationSet(SDL_Renderer* renderer, std::vector<SDL_Texture*>& frames, 
                            const std::string& path, int frameCount) {
    frames.clear();  // Clear any existing frames

    for (int i = 0; i < frameCount; i++) {
        std::string fullPath = path + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(fullPath.c_str());
        
        if (!surface) {
            std::cerr << "Failed to load surface for " << fullPath << ": " << IMG_GetError() << std::endl;
            continue;  // Skip this frame but continue loading others
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            std::cerr << "Failed to create texture for " << fullPath << ": " << SDL_GetError() << std::endl;
            continue;
        }

        frames.push_back(texture);
    }

    if (frames.empty()) {
        std::cerr << "Warning: No frames loaded for animation set: " << path << std::endl;
    }
}

void Player::LoadTextures(SDL_Renderer* renderer) {    // Preload all weapon animations with verification
    PreloadAllWeaponAnimations(renderer);
}

void Player::LoadWeaponAnimations(SDL_Renderer* renderer, WeaponType weapon) {
    if (!renderer) {
        std::cerr << "Error: NULL renderer in LoadWeaponAnimations" << std::endl;
        return;
    }

    std::string weaponPath;
    std::string weaponSuffix;
    switch (weapon) {
        case WeaponType::RIFLE:
            weaponPath = "assets/player/rifle/";
            weaponSuffix = "rifle_";
            break;
        case WeaponType::SHOTGUN:
            weaponPath = "assets/player/shotgun/";
            weaponSuffix = "shotgun_";
            break;
        case WeaponType::PISTOL:
        default:
            weaponPath = "assets/player/handgun/";
            weaponSuffix = "handgun_";
            break;
    }

    try {
        // Load all animation sets for the weapon into the maps
        LoadAnimationSet(renderer, idleAnimations[weapon], weaponPath + "idle/survivor-idle_" + weaponSuffix, IDLE_FRAME_COUNT);
        LoadAnimationSet(renderer, moveAnimations[weapon], weaponPath + "move/survivor-move_" + weaponSuffix, MOVE_FRAME_COUNT);
        LoadAnimationSet(renderer, shootAnimations[weapon], weaponPath + "shoot/survivor-shoot_" + weaponSuffix, SHOOT_FRAME_COUNT);
        LoadAnimationSet(renderer, reloadAnimations[weapon], weaponPath + "reload/survivor-reload_" + weaponSuffix, RELOAD_FRAME_COUNT);
        LoadAnimationSet(renderer, meleeAnimations[weapon], weaponPath + "meleeattack/survivor-meleeattack_" + weaponSuffix, MELEE_FRAME_COUNT);
    } catch (const std::exception& e) {
        std::cerr << "Error loading animations for " << weaponPath << ": " << e.what() << std::endl;
    }

    // If this is the current weapon, update the frame references
    if (weapon == currentWeapon) {
        UpdateAnimationReferences();
    }
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
                }
            }
        } else {
            // If current animation has no frames, revert to IDLE and set frame to 0
            currentFrame = 0;
            if (currentState != PlayerState::IDLE) { // Avoid issues if IDLE itself has 0 frames
                if (currentState == PlayerState::MELEE) {
                    isMeleeing = false; // Also reset here if melee had 0 frames
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
            if (event.key.keysym.scancode == SDL_SCANCODE_R && !isReloading) {
                // Start reload if not at max ammo
                bool needsReload = false;
                switch (currentWeapon) {
                    case WeaponType::PISTOL:
                        needsReload = pistolAmmo < PISTOL_MAX_AMMO;
                        break;
                    case WeaponType::RIFLE:
                        needsReload = rifleAmmo < RIFLE_MAX_AMMO;
                        break;
                    case WeaponType::SHOTGUN:
                        needsReload = shotgunAmmo < SHOTGUN_MAX_AMMO;
                        break;
                }
                if (needsReload) {
                    isReloading = true;
                    reloadTimer = GetCurrentReloadTime();
                    currentState = PlayerState::RELOADING;
                    currentFrame = 0;
                }
            }
            // Weapon switching
            else if (event.key.keysym.scancode == SDL_SCANCODE_1) {
                SwitchWeapon(WeaponType::PISTOL);
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_2) {
                SwitchWeapon(WeaponType::RIFLE);
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_3) {
                SwitchWeapon(WeaponType::SHOTGUN);
            }
            break;

        case SDL_KEYUP:
            keyStates[event.key.keysym.scancode] = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && !isReloading) {
                isMouseDown = true;
                // Try to shoot immediately when button is pressed
                if (shootTimer <= 0 && GetCurrentAmmo() > 0) {
                    Shoot();
                }
            }
            else if (event.button.button == SDL_BUTTON_RIGHT) {
                if (!isReloading && meleeTimer <= 0 && !isMeleeing) {
                    MeleeAttack();
                    meleeTimer = MELEE_COOLDOWN;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                isMouseDown = false;
            }
            break;

        case SDL_MOUSEMOTION:
            UpdateMousePosition(event.motion.x, event.motion.y);
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

void Player::Update(float deltaTime) {
    // Update timers
    if (shootTimer > 0) {
        shootTimer -= deltaTime;
    }
    
    if (meleeTimer > 0) {
        meleeTimer -= deltaTime;
    }

    // Handle automatic weapon fire
    if (isMouseDown && !isReloading && shootTimer <= 0) {
        if ((currentWeapon == WeaponType::RIFLE && rifleAmmo > 0) ||
            (currentWeapon == WeaponType::SHOTGUN && shotgunAmmo > 0)) {
            Shoot();
        }
    }

    // Handle reload timer
    if (isReloading) {
        reloadTimer -= deltaTime;
        if (reloadTimer <= 0) {
            isReloading = false;
            switch (currentWeapon) {
                case WeaponType::PISTOL:
                    pistolAmmo = PISTOL_MAX_AMMO;
                    break;
                case WeaponType::RIFLE:
                    rifleAmmo = RIFLE_MAX_AMMO;
                    break;
                case WeaponType::SHOTGUN:
                    shotgunAmmo = SHOTGUN_MAX_AMMO;
                    break;
            }
            if (currentState == PlayerState::RELOADING) {
                currentState = PlayerState::IDLE;
            }
        }
    }

    // Reset melee state if needed
    if (isMeleeing && currentState != PlayerState::MELEE) {
        isMeleeing = false;
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

void Player::Shoot() {
    if (isReloading) return;

    // Add bullet limit check
    static const size_t MAX_ACTIVE_BULLETS = 1000;
    if (bullets.size() >= MAX_ACTIVE_BULLETS) {
        std::cerr << "Warning: Maximum bullet limit reached" << std::endl;
        return;
    }

    bool canShoot = false;
    switch (currentWeapon) {
        case WeaponType::PISTOL:
            canShoot = pistolAmmo > 0;
            break;
        case WeaponType::RIFLE:
            canShoot = rifleAmmo > 0;
            break;
        case WeaponType::SHOTGUN:
            canShoot = shotgunAmmo > 0;
            break;
    }

    if (!canShoot) return;

    float muzzleOffsetX, muzzleOffsetY;
    switch (currentWeapon) {
        case WeaponType::RIFLE:
            muzzleOffsetX = RIFLE_MUZZLE_OFFSET_X;
            muzzleOffsetY = RIFLE_MUZZLE_OFFSET_Y;
            break;
        case WeaponType::SHOTGUN:
            muzzleOffsetX = SHOTGUN_MUZZLE_OFFSET_X;
            muzzleOffsetY = SHOTGUN_MUZZLE_OFFSET_Y;
            break;
        default: // PISTOL
            muzzleOffsetX = PISTOL_MUZZLE_OFFSET_X;
            muzzleOffsetY = PISTOL_MUZZLE_OFFSET_Y;
            break;
    }

    currentState = PlayerState::SHOOTING;
    currentFrame = 0;
    frameTimer = 0;

    float rotationRad = rotation * M_PI / 180.0f;
    float muzzleX = x + (muzzleOffsetX * cos(rotationRad)) - (muzzleOffsetY * sin(rotationRad));
    float muzzleY = y + (muzzleOffsetX * sin(rotationRad)) + (muzzleOffsetY * cos(rotationRad));
    
    if (currentWeapon == WeaponType::SHOTGUN) {
        // Create multiple pellets with spread
        for (int i = 0; i < SHOTGUN_PELLETS; i++) {
            float spreadAngle = rotation + (((float)rand() / RAND_MAX) * SHOTGUN_SPREAD - SHOTGUN_SPREAD / 2);
            Bullet* pellet = new Bullet(renderer, muzzleX, muzzleY, spreadAngle);
            bullets.push_back(pellet);
        }
    } else {
        // Single bullet for other weapons
        Bullet* bullet = new Bullet(renderer, muzzleX, muzzleY, rotation);
        bullets.push_back(bullet);
    }

    // Decrease ammo and set fire rate timer
    switch (currentWeapon) {
        case WeaponType::PISTOL:
            pistolAmmo--;
            break;
        case WeaponType::RIFLE:
            rifleAmmo--;
            break;
        case WeaponType::SHOTGUN:
            shotgunAmmo--;
            break;
    }
    shootTimer = GetCurrentFireRate();

    // Auto-reload when empty
    if (GetCurrentAmmo() == 0) {
        isReloading = true;
        reloadTimer = GetCurrentReloadTime();
        currentState = PlayerState::RELOADING;
        currentFrame = 0;
    }
}

void Player::SwitchWeapon(WeaponType weapon) {
    if (isReloading || currentWeapon == weapon) return;
    currentWeapon = weapon;
    shootTimer = 0;
    currentState = PlayerState::IDLE;
    currentFrame = 0;
    
    // Just update animation references since all animations are preloaded
    UpdateAnimationReferences();
}

int Player::GetCurrentAmmo() const {
    switch (currentWeapon) {
        case WeaponType::RIFLE:
            return rifleAmmo;
        case WeaponType::SHOTGUN:
            return shotgunAmmo;
        case WeaponType::PISTOL:
        default:
            return pistolAmmo;
    }
}

int Player::GetMaxAmmo() const {
    switch (currentWeapon) {
        case WeaponType::RIFLE:
            return RIFLE_MAX_AMMO;
        case WeaponType::SHOTGUN:
            return SHOTGUN_MAX_AMMO;
        case WeaponType::PISTOL:
        default:
            return PISTOL_MAX_AMMO;
    }
}

float Player::GetCurrentFireRate() const {
    switch (currentWeapon) {
        case WeaponType::RIFLE:
            return RIFLE_FIRE_RATE;
        case WeaponType::SHOTGUN:
            return SHOTGUN_FIRE_RATE;
        case WeaponType::PISTOL:
        default:
            return PISTOL_FIRE_RATE;
    }
}

float Player::GetCurrentReloadTime() const {
    switch (currentWeapon) {
        case WeaponType::RIFLE:
            return RIFLE_RELOAD_TIME;
        case WeaponType::SHOTGUN:
            return SHOTGUN_RELOAD_TIME;
        case WeaponType::PISTOL:
        default:
            return PISTOL_RELOAD_TIME;
    }
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

void Player::RenderAimingLine(SDL_Renderer* renderer, Camera* camera) {
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

void Player::RenderMuzzlePosition(SDL_Renderer* renderer, Camera* camera) {
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

void Player::Render(SDL_Renderer* renderer, Camera* camera) {
    // Render bullets first, passing the camera
    for (auto bullet : bullets) {
        bullet->Render(renderer, camera);
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
        screenDestRect.y = static_cast<int>((y - destRect.h / 2.0f) - camera->GetY());
        screenDestRect.w = destRect.w; 
        screenDestRect.h = destRect.h; 

        SDL_Point center = {screenDestRect.w / 2, screenDestRect.h / 2};
        SDL_RenderCopyEx(renderer, currentTexture, &currentFrameSrcRect, &screenDestRect, 
                        rotation, &center, SDL_FLIP_NONE);
          // Debug visualization for Player Hitbox - 60% of visual size
        float hitboxScale = 0.6f;  // Hitbox is 60% of the sprite size
        int hitboxWidth = static_cast<int>(destRect.w * hitboxScale);
        int hitboxHeight = static_cast<int>(destRect.h * hitboxScale);
        SDL_Rect playerHitboxRect = {
            static_cast<int>(GetX() - hitboxWidth / 2.0f - camera->GetX()),
            static_cast<int>(GetY() - hitboxHeight / 2.0f - camera->GetY()),
            hitboxWidth,
            hitboxHeight
        };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &playerHitboxRect);

        // Debug visualization, pass camera
        RenderAimingLine(renderer, camera);
        RenderMuzzlePosition(renderer, camera);
    }
}

float Player::GetCenterX() const {
    return x;
}

float Player::GetCenterY() const {
    return y;
}

void Player::UpdateBullets(float deltaTime, Camera* camera) {
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

void Player::TakeDamage(int amount) {
    currentHealth = std::max(0, currentHealth - amount);
}

void Player::Heal(int amount) {
    currentHealth = std::min(MAX_HEALTH, currentHealth + amount);
}

bool Player::VerifyAnimationLoading(const std::string& weaponPath, WeaponType weapon) {
    bool success = true;
    
    // Helper function to verify a specific animation set
    auto verifyAnimationSet = [&](const std::vector<SDL_Texture*>& frames, const std::string& animType, int expectedCount) {
        if (frames.empty() || frames.size() != expectedCount) {
            std::cerr << "Failed to load " << weaponPath << animType << " animations. ";
            std::cerr << "Expected " << expectedCount << " frames, got " << frames.size() << std::endl;
            success = false;
        }
    };

    // Verify all animation sets for this weapon
    verifyAnimationSet(idleAnimations[weapon], "idle", IDLE_FRAME_COUNT);
    verifyAnimationSet(moveAnimations[weapon], "move", MOVE_FRAME_COUNT);
    verifyAnimationSet(shootAnimations[weapon], "shoot", SHOOT_FRAME_COUNT);
    verifyAnimationSet(reloadAnimations[weapon], "reload", RELOAD_FRAME_COUNT);
    verifyAnimationSet(meleeAnimations[weapon], "melee", MELEE_FRAME_COUNT);

    return success;
}

void Player::PreloadAllWeaponAnimations(SDL_Renderer* renderer) {
    // Load pistol animations
    if (!VerifyAnimationLoading("assets/player/handgun/", WeaponType::PISTOL)) {
        LoadWeaponAnimations(renderer, WeaponType::PISTOL);
    }
    
    // Load rifle animations
    if (!VerifyAnimationLoading("assets/player/rifle/", WeaponType::RIFLE)) {
        LoadWeaponAnimations(renderer, WeaponType::RIFLE);
    }

    // Load shotgun animations
    if (!VerifyAnimationLoading("assets/player/shotgun/", WeaponType::SHOTGUN)) {
        LoadWeaponAnimations(renderer, WeaponType::SHOTGUN);
    }

    // Set initial animation references
    UpdateAnimationReferences();
}

void Player::UpdateAnimationReferences() {
    // Safety check - make sure all animation sets exist for current weapon
    if (idleAnimations.find(currentWeapon) == idleAnimations.end() ||
        moveAnimations.find(currentWeapon) == moveAnimations.end() ||
        shootAnimations.find(currentWeapon) == shootAnimations.end() ||
        reloadAnimations.find(currentWeapon) == reloadAnimations.end() ||
        meleeAnimations.find(currentWeapon) == meleeAnimations.end()) {
        std::cerr << "Warning: Missing animations for current weapon. Falling back to pistol." << std::endl;
        currentWeapon = WeaponType::PISTOL;
    }

    // Update references to point to the current weapon's animations
    idleFrames = idleAnimations[currentWeapon];
    moveFrames = moveAnimations[currentWeapon];
    shootFrames = shootAnimations[currentWeapon];
    reloadFrames = reloadAnimations[currentWeapon];
    meleeFrames = meleeAnimations[currentWeapon];
}