#include "include/ZombiePool.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

ZombiePool::ZombiePool(SDL_Renderer* renderer, size_t poolSize) 
    : renderer(renderer) {
    // Reserve space for our vectors
    pool.reserve(poolSize);
    activeZombies.reserve(poolSize);
    isInUse.reserve(poolSize);
}

void ZombiePool::AddZombie() {
    // Create zombie off-screen initially
    Zombie* zombie = new Zombie(renderer, -1000.0f, -1000.0f);
    pool.push_back(zombie);
    isInUse.push_back(false);  // Mark as not in use initially
}

ZombiePool::~ZombiePool() {
    try {
        for (Zombie* zombie : pool) {
            delete zombie;
        }
        pool.clear();
        activeZombies.clear();
        isInUse.clear();
    } catch (...) {
        std::cerr << "ZombiePool: Error during cleanup" << std::endl;
    }
}

Zombie* ZombiePool::GetZombie() {
    // First check recycled zombies
    if (!recycledZombies.empty()) {
        Zombie* zombie = recycledZombies.front();
        recycledZombies.pop();
        activeZombies.push_back(zombie);
        return zombie;
    }

    // If no recycled zombies, get one from the pool
    try {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!isInUse[i] && pool[i] != nullptr) {
                isInUse[i] = true;
                activeZombies.push_back(pool[i]);
                
                // Log pool usage stats
                size_t activeCount = std::count(isInUse.begin(), isInUse.end(), true);
                if (activeCount > pool.size() * 0.8f) {
                    std::cout << "ZombiePool: High usage warning - " << activeCount << "/" 
                              << pool.size() << " zombies active" << std::endl;
                    // Trigger aggressive recycling when pool is nearly full
                    return pool[i];
                }
                
                return pool[i];
            }
        }
        std::cerr << "ZombiePool: No available zombies in pool" << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Error in GetZombie: " << e.what() << std::endl;
        return nullptr;
    }
}

void ZombiePool::ReturnZombie(Zombie* zombie) {
    if (!zombie) {
        std::cerr << "ZombiePool: Attempted to return null zombie" << std::endl;
        return;
    }

    try {
        // Find and mark the zombie as not in use
        bool found = false;
        for (size_t i = 0; i < pool.size(); ++i) {
            // Check if the pointer to the zombie is in the pool and is in use
            if (pool[i] == zombie) {
                isInUse[i] = false;
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << "ZombiePool: Attempted to return zombie not from this pool" << std::endl;
            return;
        }

        // Remove from active zombies using the erase-remove idiom
        activeZombies.erase(
            std::remove(activeZombies.begin(), activeZombies.end(), zombie),
            activeZombies.end()
        );

    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Error in ReturnZombie: " << e.what() << std::endl;
    }
}

void ZombiePool::Update(float deltaTime, Player* player) {
    if (!player) {
        std::cerr << "ZombiePool: Null player in Update" << std::endl;
        return;
    }

    try {
        // Update zombie distances and recycle if needed
        UpdateZombieDistances(player);
        
        // Copy to prevent modification during iteration
        std::vector<Zombie*> currentActive = activeZombies;
        
        for (Zombie* zombie : currentActive) {
            if (zombie && !zombie->IsDead()) {
                zombie->Update(deltaTime, player, currentActive);
                
                // If zombie died or is too far, recycle it
                if (zombie->IsDead() || IsZombieTooFar(zombie, player, RECYCLE_DISTANCE)) {
                    ReturnZombie(zombie);
                }
            }
        }

        // Optimize zombie distribution periodically
        static float optimizeTimer = 0.0f;
        optimizeTimer += deltaTime;
        if (optimizeTimer >= 1.0f) {  // Optimize every second
            OptimizeZombieDistribution(player);
            optimizeTimer = 0.0f;
        }
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Error in Update: " << e.what() << std::endl;
    }
}

void ZombiePool::Render(SDL_Renderer* renderer, Camera* camera) {
    if (!renderer || !camera) {
        std::cerr << "ZombiePool: Null renderer or camera in Render" << std::endl;
        return;
    }

    try {
        for (Zombie* zombie : activeZombies) {
            if (zombie && !zombie->IsDead()) {
                zombie->Render(renderer, camera);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Error in Render: " << e.what() << std::endl;
    }
}

void ZombiePool::RecycleDistantZombies(Player* player, float maxDistance) {
    std::vector<Zombie*> zombiesToRecycle;
    
    for (Zombie* zombie : activeZombies) {
        if (IsZombieTooFar(zombie, player, maxDistance)) {
            zombiesToRecycle.push_back(zombie);
        }
    }
    
    for (Zombie* zombie : zombiesToRecycle) {
        ReturnZombie(zombie);
        recycledZombies.push(zombie);
    }
}

void ZombiePool::OptimizeZombieDistribution(Player* player) {
    size_t activeCount = activeZombies.size();
    
    // If we're using too many zombies, recycle distant ones
    if (activeCount > pool.size() * 0.8f) {
        RecycleDistantZombies(player, MIN_RECYCLE_DISTANCE);
    }
    
    // Ensure zombies are well-distributed around the player
    std::vector<Zombie*> poorlyPlaced;
    for (Zombie* zombie : activeZombies) {
        if (IsZombieTooFar(zombie, player, OPTIMAL_DISTANCE)) {
            poorlyPlaced.push_back(zombie);
        }
    }
    
    // Reposition poorly placed zombies
    for (Zombie* zombie : poorlyPlaced) {
        SDL_Point newPos = GetOptimalSpawnPosition(player);
        zombie->Reset(static_cast<float>(newPos.x), static_cast<float>(newPos.y));
    }
}

bool ZombiePool::IsZombieTooFar(const Zombie* zombie, const Player* player, float maxDistance) const {
    if (!zombie || !player) return true;
    
    float dx = zombie->GetX() - player->GetX();
    float dy = zombie->GetY() - player->GetY();
    float distSquared = dx * dx + dy * dy;
    
    return distSquared > maxDistance * maxDistance; //Optimize by comparing squared distances
}

void ZombiePool::UpdateZombieDistances(Player* player) {
    // Recycle zombies that are too far from the player
    RecycleDistantZombies(player, RECYCLE_DISTANCE);
}

SDL_Point ZombiePool::GetOptimalSpawnPosition(Player* player) const {
    // Generate a position at optimal distance from player
    //rand() / RAND_MAX gives a float between 0 and 1
    // Multiply by 2 * PI to get a random angle
    float randomAngle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    // Calculate distance based on optimal distance and some randomness
    // This will create a range between 0.6 * OPTIMAL_DISTANCE and 1.4 * OPTIMAL_DISTANCE
    float distance = OPTIMAL_DISTANCE * (1.0f + 0.4f * static_cast<float>(rand()) / RAND_MAX);
    
    SDL_Point pos;
    pos.x = static_cast<int>(player->GetX() + cos(randomAngle) * distance);
    pos.y = static_cast<int>(player->GetY() + sin(randomAngle) * distance);
    
    return pos;
}

void ZombiePool::PrewarmPool() {
    // Create some initial zombies to prevent stutter when first spawning
    for (size_t i = 0; i < pool.size() / 4; ++i) {
        Zombie* zombie = GetZombie();
        if (zombie) {
            ReturnZombie(zombie);
            recycledZombies.push(zombie);
        }
    }
}