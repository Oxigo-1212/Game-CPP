#pragma once
#include <vector>
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

private:
    SDL_Renderer* renderer;
    std::vector<Zombie*> pool;
    std::vector<Zombie*> activeZombies;
    std::vector<bool> isInUse;
};