#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include "Player.h"
#include "UI.h"
#include "TileMap.h"
#include "Camera.h"
#include "ChunkManager.h"
#include "Zombie.h"
#include "ZombiePool.h"
#include "WaveManager.h"
#include "LoadingScreen.h"
#include "GameState.h"
#include "MainMenu.h"

class Game {
private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    // Game state
    GameState currentState;
    MainMenu* mainMenu;
    
    // Timing variables
    const int FPS;
    const int FRAME_TIME;
    Uint32 previousTime;
    float accumulator;
    const float FIXED_TIME_STEP;

    // Wave management
    int currentWave;
    int zombiesRemainingInWave;
    float spawnTimer;
    float timeBetweenSpawns;
    float waveDelay;
    float waveTimer;
    bool waveInProgress;
    WaveManager* waveManager;  // Added WaveManager pointer

    // Game objects
    Player* player;
    UI* ui;
    Camera* camera; // Added camera member
    ChunkManager* chunkManager; // Added ChunkManager member
    ZombiePool* zombiePool; // Added ZombiePool member
    std::vector<Zombie*> zombies; // Added zombies container
    std::unique_ptr<LoadingScreen> loadingScreen; // Added LoadingScreen member

    // Screen dimensions - consider moving to a Constants.h or config file
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

    // Game constants
    static constexpr int ZOMBIE_POOL_SIZE = 250; // Size of zombie pool

    // Wave constants
    static constexpr float INITIAL_SPAWN_DELAY = 2.0f; // Time between zombie spawns in seconds
    static constexpr float SPAWN_DELAY_DECREASE = 0.1f; // How much to decrease spawn delay each wave
    static constexpr float MIN_SPAWN_DELAY = 0.5f; // Minimum time between spawns
    static constexpr float WAVE_DELAY = 10.0f; // Time between waves
    static constexpr int BASE_ZOMBIES_PER_WAVE = 5; // Starting number of zombies
    static constexpr float MIN_SPAWN_DISTANCE = 400.0f; // Minimum distance from player to spawn
    static constexpr float MAX_SPAWN_DISTANCE = 800.0f; // Maximum distance from player to spawn

public:
    Game();
    ~Game();
    
    bool Initialize();
    void HandleEvents();
    void Update(float deltaTime);
    void Render();
    void Run();
    void Cleanup();

private:
    void SpawnZombie();
    void UpdateWaveState(float deltaTime);
    SDL_Point GetRandomSpawnPosition() const;
    void InitializeGameState();  // Added declaration
    void CleanupGameState();     // Added declaration
};