#pragma once
#include <vector>
#include <queue>
#include "Zombie.h"
#include "Player.h"
#include "Camera.h"

class ZombiePool {
public:
    explicit ZombiePool(SDL_Renderer* renderer, size_t poolSize);
    ~ZombiePool();

    Zombie* GetZombie();
    void ReturnZombie(Zombie* zombie);
    void Update(float deltaTime, Player* player);
    void Render(SDL_Renderer* renderer, Camera* camera);
    const std::vector<Zombie*>& GetActiveZombies() const { return activeZombies; }
    size_t GetActiveCount() const { return activeZombies.size(); }

    // New methods for enhanced pooling
    void RecycleDistantZombies(Player* player, float maxDistance);
    void OptimizeZombieDistribution(Player* player);
    void PrewarmPool();

private:
    static constexpr float RECYCLE_DISTANCE = 1200.0f;  // Distance at which zombies get recycled
    static constexpr float OPTIMAL_DISTANCE = 800.0f;   // Optimal distance to maintain zombies
    static constexpr float MIN_RECYCLE_DISTANCE = 600.0f; // Minimum distance for recycling during high load
    
    SDL_Renderer* renderer;
    std::vector<Zombie*> pool;
    std::vector<Zombie*> activeZombies;
    std::vector<bool> isInUse;
    std::queue<Zombie*> recycledZombies;  // Queue for quick access to recycled zombies
    
    bool IsZombieTooFar(const Zombie* zombie, const Player* player, float maxDistance) const;
    void UpdateZombieDistances(Player* player);
    SDL_Point GetOptimalSpawnPosition(Player* player) const;
};