#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <map>
#include "Bullet.h"  // Include Bullet header
#include "Camera.h"  // Include Camera header
#include "WeaponConfig.h"  // Include weapon configuration
#include "WaveManager.h"  // Add WaveManager include
#include "UI.h"  // Include UI header

enum class WeaponType {
    PISTOL,
    RIFLE,
    SHOTGUN
};

enum class PlayerState {
    IDLE,
    MOVING,
    SHOOTING,
    RELOADING
};

class Player {
private:    // Health constants
    static constexpr int STARTING_HEALTH = 100;
    static constexpr int MAX_HEALTH = 100;// Animation constants
    static constexpr int IDLE_FRAME_COUNT = 20;
    static constexpr int MOVE_FRAME_COUNT = 20;
    static constexpr int SHOOT_FRAME_COUNT = 3;
    static constexpr int RELOAD_FRAME_COUNT = 15;
    static constexpr float DEFAULT_FRAME_DURATION = 0.05f;

    SDL_Renderer* renderer;  // Store renderer for shooting
    UI* ui;  // UI reference for notifications
    float x, y;
    float speed;
    float rotation;  // Angle in degrees
    float mouseX, mouseY;  // Mouse coordinates in world space
    SDL_Rect srcRect;
    SDL_Rect destRect;
    bool keyStates[SDL_NUM_SCANCODES];
    
    // Health state
    int currentHealth;
    
    // Weapon state
    WeaponType currentWeapon;
    bool isMouseDown;
    bool isReloading;
    float reloadTimer;
    float shootTimer;
    int pistolAmmo;
    int rifleAmmo;
    int shotgunAmmo;

    // Weapon offset constants (visual only)
    static constexpr float PISTOL_MUZZLE_OFFSET_X = 50.0f;
    static constexpr float PISTOL_MUZZLE_OFFSET_Y = 28.0f;
    static constexpr float RIFLE_MUZZLE_OFFSET_X = 50.0f;
    static constexpr float RIFLE_MUZZLE_OFFSET_Y = 28.0f;
    static constexpr float SHOTGUN_MUZZLE_OFFSET_X = 50.0f;
    static constexpr float SHOTGUN_MUZZLE_OFFSET_Y = 28.0f;

    // Animation properties
    PlayerState currentState;
    std::vector<SDL_Texture*> idleFrames;
    std::vector<SDL_Texture*> moveFrames;
    std::vector<SDL_Texture*> shootFrames;
    std::vector<SDL_Texture*> reloadFrames;
    int currentFrame;
    float frameTimer;
    float frameDuration;

    // Animation storage
    std::map<WeaponType, std::vector<SDL_Texture*>> idleAnimations;
    std::map<WeaponType, std::vector<SDL_Texture*>> moveAnimations;
    std::map<WeaponType, std::vector<SDL_Texture*>> shootAnimations;
    std::map<WeaponType, std::vector<SDL_Texture*>> reloadAnimations;

    // Collection of bullets
    std::vector<Bullet*> bullets;

    // WaveManager reference
    WaveManager* waveManager;

    // Helper functions for animation
    bool VerifyAnimationLoading(const std::string& weaponPath, WeaponType weapon);
    void PreloadAllWeaponAnimations(SDL_Renderer* renderer);
    void LoadWeaponAnimations(SDL_Renderer* renderer, WeaponType weapon);
    void UpdateAnimationReferences();

public:
    Player(SDL_Renderer* renderer, WaveManager* waveManager, UI* ui, float startX = 400.0f, float startY = 300.0f);
    ~Player();

    void HandleInput(SDL_Event& event);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer, Camera* camera);
    void UpdateMousePosition(int worldMouseX, int worldMouseY);
    void UpdateBullets(float deltaTime, Camera* camera);
    std::vector<Bullet*>& GetBullets() { return bullets; }

    // Health methods
    int GetHealth() const { return currentHealth; }
    int GetMaxHealth() const { return MAX_HEALTH; }
    void TakeDamage(int amount);
    void Heal(int amount);

    // Weapon methods
    void SwitchWeapon(WeaponType weapon);
    int GetCurrentAmmo() const;
    int GetMaxAmmo() const;
    float GetCurrentFireRate() const;
    float GetCurrentReloadTime() const;
    
    // Position methods
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetRotation() const { return rotation; }
    float GetCenterX() const;
    float GetCenterY() const;
    SDL_Rect GetDestRect() const {
        // Return a smaller hitbox (60% of visual size) for collision detection
        SDL_Rect hitbox = destRect;
        float hitboxScale = 0.6f;
        int hitboxWidth = static_cast<int>(destRect.w * hitboxScale);
        int hitboxHeight = static_cast<int>(destRect.h * hitboxScale);
        hitbox.x = static_cast<int>(x - hitboxWidth / 2.0f);
        hitbox.y = static_cast<int>(y - hitboxHeight / 2.0f);
        hitbox.w = hitboxWidth;
        hitbox.h = hitboxHeight;
        return hitbox;
    }
    bool IsDead() const { return currentHealth <= 0; }
    
private:
    void LoadTextures(SDL_Renderer* renderer);
    void LoadAnimationSet(SDL_Renderer* renderer, std::vector<SDL_Texture*>& frames, 
                         const std::string& path, int frameCount);
    void RenderAimingLine(SDL_Renderer* renderer, Camera* camera);
    void RenderMuzzlePosition(SDL_Renderer* renderer, Camera* camera);
    void Shoot();
    void UpdateAnimation(float deltaTime);
    std::vector<SDL_Texture*>& GetCurrentAnimationFrames();
    int GetCurrentAnimationFrameCount() const;
};