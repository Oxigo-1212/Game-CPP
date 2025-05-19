#include "include/Zombie.h"
#include <cmath>

Zombie::Zombie(float startX, float startY) 
    : x(startX), y(startY), health(5), isDead(false), speed(100.0f), isAttacking(false), lastAttackTime(0) {
    // Initialize hitbox
    hitbox.w = 50;  // width of zombie
    hitbox.h = 50;  // height of zombie
    hitbox.x = static_cast<int>(x - hitbox.w / 2); // Center the hitbox on x
    hitbox.y = static_cast<int>(y - hitbox.h / 2); // Center the hitbox on y
}

Zombie::~Zombie() {
}

void Zombie::Update(float deltaTime, Player* player, const std::vector<Zombie*>& zombies) {
    if (isDead) return;

    // Get player position and calculate direction
    float playerX = player->GetX();
    float playerY = player->GetY();
      // Calculate distance to player
    float dx = playerX - x;
    float dy = playerY - y;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

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

    // Update hitbox position
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);

    // Check if can attack
    if (CheckCollisionWithPlayer(player)) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastAttackTime >= ATTACK_COOLDOWN) {
            isAttacking = true;
            lastAttackTime = currentTime;
            player->TakeDamage(10);
        }
    } else {
        isAttacking = false;
    }
}

void Zombie::Render(SDL_Renderer* renderer, Camera* camera) {
    if (isDead) return;

    // Convert world coordinates to screen coordinates
    SDL_Rect screenRect = {
        static_cast<int>(hitbox.x - camera->GetX()),
        static_cast<int>(hitbox.y - camera->GetY()),
        hitbox.w,
        hitbox.h
    };

    // Set color to blue for the zombie
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &screenRect);

    // If attacking, draw a red border
    if (isAttacking) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &screenRect);
    }
}

bool Zombie::CheckCollisionWithBullet(Bullet* bullet) {
    if (isDead) return false;
    
    SDL_Rect bulletHitbox = bullet->GetHitbox();
    return SDL_HasIntersection(&hitbox, &bulletHitbox);
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
