#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Player.h"
#include "Bullet.h"
#include "Camera.h"
#include "WeaponConfig.h"
#include <vector>
#include <string>

// Enable hitbox visualization
#define DEBUG_HITBOX

class Zombie {
private:
    SDL_Rect hitbox;
    float x, y;    float rotation;  // Angle in degrees
    int health;
    bool isDead;
    float speed;
    bool isAttacking;
    Uint32 lastAttackTime;
    const Uint32 ATTACK_COOLDOWN = 1000; // 1 second cooldown between attacks    // Damage and health constants
    static constexpr int STARTING_HEALTH = 8;      // Zombie starting health
    
    // Debug visualization
    bool showDebugHitbox;

    // Knockback properties
    float knockbackVelocityX;
    float knockbackVelocityY;
    float knockbackDuration;

    // Animation members
    SDL_Renderer* renderer;
    std::vector<SDL_Texture*> moveFrames;
    std::vector<SDL_Texture*> attackFrames;
    int currentFrame;
    float frameTimer;
    float frameDuration;
    SDL_Rect srcRect;
    SDL_Rect destRect;

    // Animation constants
    static const int MOVE_FRAME_COUNT = 17;  // Number of frames in move animation
    static const int ATTACK_FRAME_COUNT = 9;  // Number of frames in attack animation
    static constexpr float DEFAULT_FRAME_DURATION = 0.1f;  // Duration per frame in seconds

    // Flocking behavior constants
    static constexpr float NEIGHBOR_RADIUS = 90.0f;
    static constexpr float SEPARATION_WEIGHT = 1.5f;
    static constexpr float ALIGNMENT_WEIGHT = 1.0f;
    static constexpr float COHESION_WEIGHT = 1.0f;
    static constexpr float PLAYER_ATTRACTION_WEIGHT = 1.2f;
    static constexpr float MIN_SEPARATION = 100.0f;    // Priority system constants
    static constexpr float CLOSE_RANGE = 300.0f;  // Increased from 200
    static constexpr float FORMATION_RANGE = 450.0f;  // Increased from 400
    static constexpr float FORMATION_RADIUS = 300.0f;
    static constexpr float FORMATION_WEIGHT = 0.8f;

    // Collision constants
   

public:
    Zombie(SDL_Renderer* renderer, float startX, float startY);
    ~Zombie();

    void Update(float deltaTime, Player* player, const std::vector<Zombie*>& zombies);
    void Render(SDL_Renderer* renderer, Camera* camera);
    bool CheckCollisionWithBullet(Bullet* bullet);
    bool CheckCollisionWithPlayer(Player* player);
    bool IsDead() const { return isDead; }
    SDL_Rect GetHitbox() const { return hitbox; }
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetRotation() const { return rotation; }    void Reset(float newX, float newY, float speedMultiplier = 1.0f);    // Reset zombie position and stats
    void TakeDamage(float damageX, float damageY, bool isShotgunPellet, Bullet* bullet);
    
    // Debug visualization methods
    void SetDebugHitbox(bool show) { showDebugHitbox = show; }
    bool IsShowingDebugHitbox() const { return showDebugHitbox; }

private:
    void ApplyFlockingBehavior(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Separate(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Align(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    void Cohere(const std::vector<Zombie*>& zombies, float& dx, float& dy);
    
    // Animation methods
    void LoadTextures();
    void LoadAnimationSet(std::vector<SDL_Texture*>& frames, const std::string& basePath, const std::string& prefix, int frameCount);
    void UpdateAnimation(float deltaTime);
};
