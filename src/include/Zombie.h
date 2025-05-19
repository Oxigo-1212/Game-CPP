#pragma once
#include <SDL2/SDL.h>
#include "Player.h"
#include "Bullet.h"
#include "Camera.h"
#include <vector>

class Zombie {
private:
    SDL_Rect hitbox;
    float x, y;
    int health;
    bool isDead;
    float speed;
    bool isAttacking;
    Uint32 lastAttackTime;
    const Uint32 ATTACK_COOLDOWN = 1000; // 1 second cooldown between attacks    // Flocking behavior constants
    static constexpr float NEIGHBOR_RADIUS = 90.0f;       // Radius to consider other zombies as neighbors
    static constexpr float SEPARATION_WEIGHT = 1.5f;      // Weight for separation force
    static constexpr float ALIGNMENT_WEIGHT = 1.0f;       // Weight for alignment force
    static constexpr float COHESION_WEIGHT = 1.0f;        // Weight for cohesion force
    static constexpr float PLAYER_ATTRACTION_WEIGHT = 1.2f; // Weight for moving towards player
    static constexpr float MIN_SEPARATION = 100.0f;       // Minimum distance between zombies

    // Priority system constants
    static constexpr float CLOSE_RANGE = 200.0f;         // Distance to be considered "close" to player
    static constexpr float FORMATION_RANGE = 400.0f;      // Distance to start forming circle
    static constexpr float FORMATION_RADIUS = 300.0f;     // Desired radius of zombie formation
    static constexpr float FORMATION_WEIGHT = 0.8f;       // Weight for formation positioning

public:
    Zombie(float startX, float startY);
    ~Zombie();

    void Update(float deltaTime, Player* player, const std::vector<Zombie*>& zombies);
    void Render(SDL_Renderer* renderer, Camera* camera);
    bool CheckCollisionWithBullet(Bullet* bullet);
    bool CheckCollisionWithPlayer(Player* player);
    void TakeDamage();
    bool IsDead() const { return isDead; }
    SDL_Rect GetHitbox() const { return hitbox; }
    float GetX() const { return x; }
    float GetY() const { return y; }

private:
    void ApplyFlockingBehavior(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Separate(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Align(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Cohere(const std::vector<Zombie*>& zombies, float& dx, float& dy);
};
