#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include "Bullet.h"  // Include Bullet header

enum class PlayerState {
    IDLE,
    MOVING,
    SHOOTING,
    RELOADING,
    MELEE
};

class Player {
private:
    // Health constants
    static constexpr int STARTING_HEALTH = 100;
    static constexpr int MAX_HEALTH = 100;

    // Weapon constants
    static constexpr float PISTOL_MUZZLE_OFFSET_X = 50.0f;
    static constexpr float PISTOL_MUZZLE_OFFSET_Y = 28.0f;
    static constexpr float PISTOL_FIRE_RATE = 0.25f;  // Time between shots in seconds
    static constexpr float MELEE_COOLDOWN = 0.4f;     // Time between melee attacks
    static constexpr float RELOAD_TIME = 1.0f;        // Time to reload in seconds
    static constexpr int MAX_AMMO = 12;               // Maximum ammo capacity

    // Animation constants
    static constexpr int IDLE_FRAME_COUNT = 20;
    static constexpr int MOVE_FRAME_COUNT = 20;
    static constexpr int SHOOT_FRAME_COUNT = 3;
    static constexpr int RELOAD_FRAME_COUNT = 15;
    static constexpr int MELEE_FRAME_COUNT = 15;
    static constexpr float DEFAULT_FRAME_DURATION = 0.05f;

    SDL_Renderer* renderer;  // Store renderer for shooting
    float x, y;
    float speed;
    float rotation;  // Angle in degrees
    int mouseX, mouseY;  // Mouse coordinates
    SDL_Rect srcRect;
    SDL_Rect destRect;
    bool keyStates[SDL_NUM_SCANCODES];
    
    // Health state
    int currentHealth;
    
    // Weapon state
    int currentAmmo;
    bool isReloading;
    float reloadTimer;

    // Animation properties
    PlayerState currentState;
    std::vector<SDL_Texture*> idleFrames;
    std::vector<SDL_Texture*> moveFrames;
    std::vector<SDL_Texture*> shootFrames;
    std::vector<SDL_Texture*> reloadFrames;
    std::vector<SDL_Texture*> meleeFrames;
    int currentFrame;
    float frameTimer;
    float frameDuration;

    // Shooting properties
    float shootTimer;
    std::vector<Bullet*> bullets;

    // Melee attack properties
    float meleeTimer;
    bool isMeleeing;

public:
    Player(SDL_Renderer* renderer, float startX = 400.0f, float startY = 300.0f);
    ~Player();

    void HandleInput(SDL_Event& event);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    void UpdateMousePosition(int x, int y);
    void UpdateBullets(float deltaTime);
    std::vector<Bullet*>& GetBullets() { return bullets; }

    // Health methods
    int GetHealth() const { return currentHealth; }
    int GetMaxHealth() const { return MAX_HEALTH; }
    void TakeDamage(int amount);
    void Heal(int amount);

    // Ammo methods
    int GetAmmo() const { return currentAmmo; }
    int GetMaxAmmo() const { return MAX_AMMO; }
    
private:
    void LoadTextures(SDL_Renderer* renderer);
    void LoadAnimationSet(SDL_Renderer* renderer, std::vector<SDL_Texture*>& frames, 
                         const std::string& path, int frameCount);
    void RenderAimingLine(SDL_Renderer* renderer);
    void RenderMuzzlePosition(SDL_Renderer* renderer);
    void Shoot(SDL_Renderer* renderer);
    void UpdateAnimation(float deltaTime);
    std::vector<SDL_Texture*>& GetCurrentAnimationFrames();
    int GetCurrentAnimationFrameCount() const;
    void MeleeAttack();
};