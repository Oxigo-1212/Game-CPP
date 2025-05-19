#include "include/ZombiePool.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

ZombiePool::ZombiePool(SDL_Renderer* renderer, size_t poolSize) : renderer(renderer) {
    if (!renderer) {
        throw std::runtime_error("ZombiePool: Null renderer provided");
    }
    
    if (poolSize == 0 || poolSize > 1000) { // Sanity check on pool size
        throw std::runtime_error("ZombiePool: Invalid pool size: " + std::to_string(poolSize));
    }

    std::cout << "ZombiePool: Starting initialization with " << poolSize << " zombies..." << std::endl;
    
    try {
        // Pre-allocate vectors with exact size to prevent reallocation
        pool.resize(poolSize, nullptr);
        isInUse.resize(poolSize, false);
        activeZombies.reserve(poolSize);
        
        size_t successCount = 0;
        // Create all zombies
        for (size_t i = 0; i < poolSize; ++i) {
            try {
                Zombie* zombie = new Zombie(renderer, 0.0f, 0.0f);
                if (!zombie) {
                    throw std::runtime_error("Failed to allocate zombie");
                }
                pool[i] = zombie;
                successCount++;
                
                if ((i + 1) % 10 == 0 || i == poolSize - 1) {
                    std::cout << "ZombiePool: Progress - " << (i + 1) << "/" << poolSize 
                              << " zombies created (" 
                              << static_cast<int>((i + 1) * 100.0f / poolSize) 
                              << "%)" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "ZombiePool: Failed to create zombie " << i << ": " << e.what() << std::endl;
                pool[i] = nullptr; // Mark this slot as permanently unavailable
            }
        }

        if (successCount == 0) {
            throw std::runtime_error("ZombiePool: Failed to create any zombies");
        }

        std::cout << "ZombiePool: Successfully created " << successCount << "/" << poolSize 
                  << " zombies (" << static_cast<int>(successCount * 100.0f / poolSize) 
                  << "%)" << std::endl;
                  
    } catch (const std::exception& e) {
        // Clean up any zombies that were created before the error
        for (Zombie* zombie : pool) {
            delete zombie;
        }
        pool.clear();
        isInUse.clear();
        throw; // Re-throw the exception after cleanup
    }
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
    try {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!isInUse[i] && pool[i] != nullptr) {
                isInUse[i] = true;
                activeZombies.push_back(pool[i]);
                
                // Log pool usage stats
                size_t activeCount = std::count(isInUse.begin(), isInUse.end(), true);
                if (activeCount > pool.size() * 0.8f) { // Warning at 80% capacity
                    std::cout << "ZombiePool: High usage warning - " << activeCount << "/" 
                              << pool.size() << " zombies active" << std::endl;
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
        // Copy the active zombies vector to prevent modification during iteration
        std::vector<Zombie*> currentActive = activeZombies;
        
        for (Zombie* zombie : currentActive) {
            if (zombie && !zombie->IsDead()) {
                zombie->Update(deltaTime, player, currentActive);
                
                // If zombie died during update, return it to the pool
                if (zombie->IsDead()) {
                    ReturnZombie(zombie);
                }
            }
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