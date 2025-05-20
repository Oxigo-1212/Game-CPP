#include "include/Zombie.h"
#include <cmath>
#include <iostream>

Zombie::Zombie(SDL_Renderer* renderer, float startX, float startY) 
    : renderer(renderer), x(startX), y(startY), rotation(0.0f),
      health(STARTING_HEALTH), isDead(false), speed(100.0f), isAttacking(false), lastAttackTime(0),
      currentFrame(0), frameTimer(0.0f), frameDuration(DEFAULT_FRAME_DURATION),
      knockbackVelocityX(0.0f), knockbackVelocityY(0.0f), knockbackDuration(0.0f) {
    
    // Initialize hitbox
    hitbox.w = 50;  // width of zombie
    hitbox.h = 50;  // height of zombie
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);

    LoadTextures();
}

Zombie::~Zombie() {
    // Cleanup animation textures
    for (SDL_Texture* tex : moveFrames) {
        if (tex) SDL_DestroyTexture(tex);
    }
    for (SDL_Texture* tex : attackFrames) {
        if (tex) SDL_DestroyTexture(tex);
    }
}

void Zombie::LoadTextures() {
    // Load move animation
    LoadAnimationSet(moveFrames, "assets/zombie/move/", "zombie_move_", MOVE_FRAME_COUNT);
    // Load attack animation
    LoadAnimationSet(attackFrames, "assets/zombie/attack/", "zombie_attack_", ATTACK_FRAME_COUNT);

    // Set up source and destination rectangles
    if (!moveFrames.empty() && moveFrames[0]) {
        SDL_QueryTexture(moveFrames[0], nullptr, nullptr, &srcRect.w, &srcRect.h);
        srcRect.x = 0;
        srcRect.y = 0;
        
        // Scale the sprite to match player size (approximately 64x64)
        float scale = 0.5f;  // Adjust this to match player size
        destRect.w = static_cast<int>(srcRect.w * scale);
        destRect.h = static_cast<int>(srcRect.h * scale);
        
        // Update hitbox to match the sprite size
        hitbox.w = static_cast<int>(destRect.w * 0.6f);  // Make hitbox slightly smaller than sprite
        hitbox.h = static_cast<int>(destRect.h * 0.6f);
    }
}

void Zombie::LoadAnimationSet(std::vector<SDL_Texture*>& frames, const std::string& basePath, 
                            const std::string& prefix, int frameCount) {
    frames.clear();
    for (int i = 0; i < frameCount; ++i) {
        std::string path = basePath + prefix + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Failed to load zombie texture: " << path << " Error: " << IMG_GetError() << std::endl;
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            continue;
        }
        frames.push_back(texture);
    }
}

void Zombie::UpdateAnimation(float deltaTime) {
    frameTimer += deltaTime;
    if (frameTimer >= frameDuration) {
        frameTimer = 0;
        currentFrame++;
        
        const auto& currentFrames = isAttacking ? attackFrames : moveFrames;
        int maxFrames = isAttacking ? ATTACK_FRAME_COUNT : MOVE_FRAME_COUNT;
        
        if (currentFrame >= maxFrames) {
            currentFrame = 0;
            if (isAttacking) {
                // Optional: Reset attack state after animation completes
                // isAttacking = false;
            }
        }
    }
}

void Zombie::Update(float deltaTime, Player* player, const std::vector<Zombie*>& zombies) {
    if (isDead) return;

    // Handle knockback effect
    if (knockbackDuration > 0) {
        x += knockbackVelocityX * deltaTime;
        y += knockbackVelocityY * deltaTime;
        knockbackDuration -= deltaTime;
        
        // Update hitbox during knockback
        hitbox.x = static_cast<int>(x - hitbox.w / 2);
        hitbox.y = static_cast<int>(y - hitbox.h / 2);
        
        // If knockback is done, reset velocities
        if (knockbackDuration <= 0) {
            knockbackVelocityX = 0;
            knockbackVelocityY = 0;
        }
        
        // Still update animation even during knockback
        UpdateAnimation(deltaTime);
        return;  // Skip normal movement while being knocked back
    }

    // Get player position and calculate direction
    float playerX = player->GetX();
    float playerY = player->GetY();
    float dx = playerX - x;
    float dy = playerY - y;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

    // Calculate rotation to face the player
    rotation = (atan2(dy, dx) * 180.0f / M_PI);  // Remove the +90 if zombie sprite faces right by default

    // Calculate base direction based on priority and formation
    if (distanceToPlayer < CLOSE_RANGE) {
        // Close range: Direct attack
        if (distanceToPlayer > 0) {
            dx /= distanceToPlayer;
            dy /= distanceToPlayer;
        }
    } else if (distanceToPlayer < FORMATION_RANGE) {
        // Formation range: Try to form a circle around player
        float angleToPlayer = std::atan2(dy, dx);
        float desiredAngle = angleToPlayer;
        
        // Find a gap in the formation
        int zombieCount = 0;
        float angleSum = 0;
        for (auto other : zombies) {
            if (other != this && !other->isDead) {
                float otherDx = other->GetX() - playerX;
                float otherDy = other->GetY() - playerY;
                float otherAngle = std::atan2(otherDy, otherDx);
                angleSum += otherAngle;
                zombieCount++;
            }
        }

        if (zombieCount > 0) {
            // Try to space zombies evenly around the circle
            float averageAngle = angleSum / zombieCount;
            desiredAngle = averageAngle + (2 * M_PI / (zombieCount + 1));
        }

        // Calculate desired position on the circle
        float targetX = playerX + FORMATION_RADIUS * std::cos(desiredAngle);
        float targetY = playerY + FORMATION_RADIUS * std::sin(desiredAngle);

        // Get direction to formation position
        dx = targetX - x;
        dy = targetY - y;
        float formationDist = std::sqrt(dx * dx + dy * dy);
        if (formationDist > 0) {
            dx /= formationDist;
            dy /= formationDist;
        }
    } else {
        // Outside formation range: Standard following behavior
        if (distanceToPlayer > 0) {
            dx /= distanceToPlayer;
            dy /= distanceToPlayer;
        }
    }

    // Apply flocking behavior with appropriate weights
    ApplyFlockingBehavior(zombies, dx, dy);

    // Normalize final direction
    float finalLength = std::sqrt(dx * dx + dy * dy);
    if (finalLength > 0) {
        dx /= finalLength;
        dy /= finalLength;
    }

    // Move towards target
    x += dx * speed * deltaTime;
    y += dy * speed * deltaTime;

    // Update hitbox position to be centered on the zombie
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);

    // Update attack state
    bool wasAttacking = isAttacking;
    if (CheckCollisionWithPlayer(player)) {
        Uint32 currentTime = SDL_GetTicks();            if (currentTime - lastAttackTime >= ATTACK_COOLDOWN) {
            isAttacking = true;
            lastAttackTime = currentTime;
            player->TakeDamage(WaveConfig::ZOMBIE_BASE_DAMAGE);
            if (!wasAttacking) {
                // Reset animation when starting to attack
                currentFrame = 0;
                frameTimer = 0;
            }
        }
    } else {
        if (wasAttacking) {
            // Reset animation when stopping attack
            isAttacking = false;
            currentFrame = 0;
            frameTimer = 0;
        }
    }

    // Update animation
    UpdateAnimation(deltaTime);
}

void Zombie::Render(SDL_Renderer* renderer, Camera* camera) {
    if (isDead) return;

    // Get current animation frame
    const auto& currentFrames = isAttacking ? attackFrames : moveFrames;
    if (currentFrames.empty() || currentFrame >= currentFrames.size() || !currentFrames[currentFrame]) {
        // Fallback rendering if textures aren't loaded
        SDL_Rect screenRect = {
            static_cast<int>(hitbox.x - camera->GetX()),
            static_cast<int>(hitbox.y - camera->GetY()),
            hitbox.w,
            hitbox.h
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &screenRect);
        return;
    }

    // Update destination rectangle position for rendering
    destRect.x = static_cast<int>(x - destRect.w / 2 - camera->GetX());
    destRect.y = static_cast<int>(y - destRect.h / 2 - camera->GetY());

    // Render the current frame with rotation
    SDL_Point center = { destRect.w / 2, destRect.h / 2 };
    SDL_RenderCopyEx(renderer, currentFrames[currentFrame], &srcRect, &destRect, 
                     rotation, &center, SDL_FLIP_NONE);

    // Always render hitbox for debugging
    SDL_Rect hitboxScreen = {
        static_cast<int>(hitbox.x - camera->GetX()),
        static_cast<int>(hitbox.y - camera->GetY()),
        hitbox.w,
        hitbox.h
    };
    
    // Draw hitbox outline in red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &hitboxScreen);
    
    // Draw hit points above the zombie
    SDL_Rect healthBar = {
        hitboxScreen.x,
        hitboxScreen.y - 10,
        hitboxScreen.w,
        5
    };
    
    // Health bar background (red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);
    
    // Health bar foreground (green)
    healthBar.w = static_cast<int>((health / 5.0f) * hitboxScreen.w);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);
}

bool Zombie::CheckCollisionWithBullet(Bullet* bullet) {
    if (isDead) return false;
    
    SDL_Rect bulletHitbox = bullet->GetHitbox();
    if (SDL_HasIntersection(&hitbox, &bulletHitbox)) {
        // Calculate direction from bullet to zombie for knockback
        float dx = x - bullet->GetX();
        float dy = y - bullet->GetY();
        
        // Use bullet type to determine damage and knockback
        bool isShotgunPellet = (bullet->GetBulletType() == BulletType::SHOTGUN_PELLET);
          // Apply damage with knockback in the direction of the bullet's travel
        TakeDamageWithKnockback(dx, dy, isShotgunPellet, bullet);
        return true;
    }
    return false;
}

bool Zombie::CheckCollisionWithPlayer(Player* player) {
    if (isDead) return false;

    SDL_Rect playerDestRect = player->GetDestRect();
    return SDL_HasIntersection(&hitbox, &playerDestRect);
}

void Zombie::TakeDamage() {
    if (isDead) return;
    
    health--;
    if (health <= 0) {
        isDead = true;
    }
}

void Zombie::TakeDamageWithKnockback(float damageX, float damageY, bool isShotgunPellet, Bullet* bullet) {
    // Only apply knockback for shotgun pellets
    if (isShotgunPellet) {
        float force = WeaponConfig::Shotgun::KNOCKBACK_FORCE * WeaponConfig::Shotgun::KNOCKBACK_MULTIPLIER;
        
        // Calculate knockback direction
        float length = std::sqrt(damageX * damageX + damageY * damageY);
        if (length > 0) {
            knockbackVelocityX = (damageX / length) * force;
            knockbackVelocityY = (damageY / length) * force;
        }
        
        // Set knockback duration
        knockbackDuration = WeaponConfig::Shotgun::KNOCKBACK_DURATION;
    }

    // Apply appropriate damage based on weapon type
    if (isShotgunPellet) {
        health -= WeaponConfig::Shotgun::PELLET_DAMAGE;
    } else if (bullet && bullet->GetBulletType() == BulletType::RIFLE) {
        health -= WeaponConfig::Rifle::DAMAGE;
    } else {
        health -= WeaponConfig::Pistol::DAMAGE;
    }
    if (health <= 0) {
        isDead = true;
    }
}

void Zombie::ApplyFlockingBehavior(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    float sepX = 0, sepY = 0;
    float aliX = 0, aliY = 0;
    float cohX = 0, cohY = 0;
    
    // Apply all three flocking behaviors
    Separate(zombies, sepX, sepY);
    Align(zombies, aliX, aliY);
    Cohere(zombies, cohX, cohY);

    // Weight and combine all forces
    dx = dx * PLAYER_ATTRACTION_WEIGHT +  // Base movement towards player
         sepX * SEPARATION_WEIGHT +       // Separation
         aliX * ALIGNMENT_WEIGHT +        // Alignment
         cohX * COHESION_WEIGHT;         // Cohesion

    dy = dy * PLAYER_ATTRACTION_WEIGHT +
         sepY * SEPARATION_WEIGHT +
         aliY * ALIGNMENT_WEIGHT +
         cohY * COHESION_WEIGHT;
}

void Zombie::Separate(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    dx = dy = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = x - other->x;
            float distY = y - other->y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < MIN_SEPARATION) {
                // Add separation force inversely proportional to distance
                dx += distX / (distance + 1);  // +1 to avoid division by zero
                dy += distY / (distance + 1);
                count++;
            }
        }
    }

    // Average the separation force
    if (count > 0) {
        dx /= count;
        dy /= count;
        // Normalize
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Align(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    dx = dy = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = other->x - x;
            float distY = other->y - y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < NEIGHBOR_RADIUS) {
                // Get the normalized direction of the other zombie
                float otherDx = other->hitbox.x - other->GetX();
                float otherDy = other->hitbox.y - other->GetY();
                float length = std::sqrt(otherDx * otherDx + otherDy * otherDy);
                if (length > 0) {
                    dx += otherDx / length;
                    dy += otherDy / length;
                    count++;
                }
            }
        }
    }

    if (count > 0) {
        dx /= count;
        dy /= count;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Cohere(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    float centerX = 0, centerY = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = other->x - x;
            float distY = other->y - y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < NEIGHBOR_RADIUS) {
                centerX += other->x;
                centerY += other->y;
                count++;
            }
        }
    }

    if (count > 0) {
        // Calculate center of mass
        centerX /= count;
        centerY /= count;
        
        // Get direction to center of mass
        dx = centerX - x;
        dy = centerY - y;
        
        // Normalize
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Reset(float newX, float newY, float speedMultiplier) {
    x = newX;
    y = newY;
    rotation = 0.0f;
    health = WaveConfig::ZOMBIE_BASE_HEALTH;
    isDead = false;
    speed = WaveConfig::ZOMBIE_BASE_SPEED * speedMultiplier;
    isAttacking = false;
    lastAttackTime = 0;
    currentFrame = 0;
    frameTimer = 0.0f;
    knockbackVelocityX = 0.0f;
    knockbackVelocityY = 0.0f;
    knockbackDuration = 0.0f;
    
    // Reset hitbox position
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);
}
