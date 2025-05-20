#include "include/Game.h"
#include "include/ChunkManager.h" // Ensure ChunkManager is included
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game() : 
    isRunning(false), 
    window(nullptr), 
    renderer(nullptr),
    previousTime(0),
    accumulator(0.0f),
    FPS(60),
    FRAME_TIME(1000 / FPS),
    FIXED_TIME_STEP(1.0f / 60.0f),
    player(nullptr),
    ui(nullptr),
    camera(nullptr),
    chunkManager(nullptr),
    zombiePool(nullptr),
    waveManager(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Game::~Game() {
    Cleanup();
}

bool Game::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }

    // Create WaveManager first since Player needs it
    waveManager = new WaveManager();
    
    // Create and initialize UI
    ui = new UI(renderer);
    if (!ui->Initialize()) {
        std::cerr << "UI initialization failed" << std::endl;
        return false;
    }
    
     // Create player instance with WaveManager and UI
    player = new Player(renderer, waveManager, ui);

    // Create camera instance BEFORE ChunkManager if ChunkManager needs player/camera info indirectly
    // For now, player is enough for camera initial position.
    camera = new Camera(player->GetX() - Constants::WINDOW_WIDTH / 2.0f + player->GetDestRect().w / 2.0f, 
                        player->GetY() - Constants::WINDOW_HEIGHT / 2.0f + player->GetDestRect().h / 2.0f, 
                        Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);

    // Create ChunkManager instance
    // Ensure player is initialized before passing to ChunkManager
    chunkManager = new ChunkManager(renderer, player, "assets/maps/grasstiles.csv", "assets/tilesets/Grass 13  .png");

    // Initialize the zombie pool
    try {
        std::cout << "Game: Creating zombie pool..." << std::endl;
        zombiePool = new ZombiePool(renderer, ZOMBIE_POOL_SIZE);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create zombie pool: " << e.what() << std::endl;
        return false;
    }

    // Start first wave
    waveManager->StartNextWave();

    isRunning = true;
    previousTime = SDL_GetTicks();
    return true;
}

void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && event.type == SDL_KEYDOWN) {
            isRunning = false;
        } else if (event.type == SDL_MOUSEMOTION) {
            // Convert screen mouse coordinates to world coordinates for the player
            SDL_FPoint worldMousePos = camera->ScreenToWorld(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
            player->UpdateMousePosition(static_cast<int>(worldMousePos.x), static_cast<int>(worldMousePos.y));
        } else {
            player->HandleInput(event);
        }
    }
}

void Game::Update(float deltaTime) {
    if (player) {
        player->Update(deltaTime);
        // Check if player died
        if (player->IsDead()) {
            std::cout << "Game Over!" << std::endl;
            isRunning = false;
            return;
        }
    }

    // Update wave manager and UI
    if (waveManager) {
        waveManager->Update(deltaTime);
        
        // Update UI with wave info
        if (ui) {
            ui->UpdateWaveInfo(
                waveManager->GetCurrentWave(),
                waveManager->GetZombiesRemaining(),
                waveManager->GetWaveDelay()  // This will get either the wave delay timer or 0
            );
        }
        
        // Spawn zombies if needed
        if (waveManager->ShouldSpawnZombie()) {
            // Spawn entire group at once
            int groupSize = waveManager->GetCurrentGroupSize();
            for (int i = 0; i < groupSize; i++) {
                SpawnZombie();
            }
        }
    }

    // Update UI notifications
    if (ui) {
        ui->UpdateNotification(deltaTime);
    }

    // Update zombies through the pool
    if (zombiePool) {
        zombiePool->Update(deltaTime, player);

        // Check collision with player's bullets
        auto& bullets = player->GetBullets();
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
            Bullet* bullet = *bulletIt;
            if (!bullet->IsActive()) {
                ++bulletIt;
                continue;
            }

            // Let the zombie pool handle bullet collisions and death
            bool hitZombie = false;
            SDL_Rect bulletRect = bullet->GetHitbox();
            SDL_Point bulletPos = { static_cast<int>(bullet->GetX()), static_cast<int>(bullet->GetY()) };
            
            for (Zombie* zombie : zombiePool->GetActiveZombies()) {                if (!zombie->IsDead() && zombie->CheckCollisionWithBullet(bullet)) {
                    // Note: TakeDamage is now handled inside CheckCollisionWithBullet
                    bullet->Deactivate();
                    hitZombie = true;
                    break;
                }
            }
            
            if (hitZombie) {
                bulletIt = bullets.erase(bulletIt);
                delete bullet;
            } else {
                ++bulletIt;
            }
        }
    }

    if (chunkManager) { // Update ChunkManager
        chunkManager->Update(deltaTime);
    }
    
    // Update camera to follow the player's center
    if (player && camera) { 
        // Camera follows the logical player position, which is now wrapped.
        camera->Update(player->GetX(), player->GetY(), deltaTime);
    }
}

SDL_Point Game::GetRandomSpawnPosition() const {
    if (!player) return {0, 0};

    static std::vector<SDL_Point> spawnPoints;
    static int currentSpawnPoint = 0;

    // Generate new spawn points if we're starting a new group
    if (spawnPoints.empty()) {
        float playerX = player->GetX();
        float playerY = player->GetY();

        // Create several spawn points around the player
        for (int i = 0; i < WaveConfig::SPAWN_POINTS; i++) {
            // Evenly distribute spawn points around the player
            float angle = (2.0f * M_PI * i) / WaveConfig::SPAWN_POINTS;
            
            // Get random distance between MIN and MAX spawn distance
            float distance = MIN_SPAWN_DISTANCE + 
                (static_cast<float>(rand()) / RAND_MAX * (MAX_SPAWN_DISTANCE - MIN_SPAWN_DISTANCE));
            
            spawnPoints.push_back({
                static_cast<int>(playerX + cos(angle) * distance),
                static_cast<int>(playerY + sin(angle) * distance)
            });
        }
    }    // Pick a spawn point and get a random position around it
    SDL_Point basePoint = spawnPoints[currentSpawnPoint];
    
    // Get a random offset within the group spawn radius
    float groupAngle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
    float groupRadius = static_cast<float>(rand()) / RAND_MAX * WaveConfig::GROUP_SPAWN_RADIUS;
    
    SDL_Point groupPos = {
        static_cast<int>(basePoint.x + cos(groupAngle) * groupRadius),
        static_cast<int>(basePoint.y + sin(groupAngle) * groupRadius)
    };
    
    // Cycle through spawn points
    currentSpawnPoint = (currentSpawnPoint + 1) % WaveConfig::SPAWN_POINTS;
    
    // Clear spawn points when we've used them all
    if (currentSpawnPoint == 0) {
        spawnPoints.clear();
    }
    
    return groupPos;
}

void Game::SpawnZombie() {
    if (!zombiePool || !waveManager) return;

    SDL_Point spawnPos = GetRandomSpawnPosition();
    Zombie* zombie = zombiePool->GetZombie();
    if (zombie) {
        // Add random speed variation
        float speedMultiplier = 1.0f - WaveConfig::SPEED_VARIATION + 
            (static_cast<float>(rand()) / RAND_MAX * (WaveConfig::SPEED_VARIATION * 2));
            
        zombie->Reset(static_cast<float>(spawnPos.x), static_cast<float>(spawnPos.y), speedMultiplier);
        waveManager->OnZombieSpawned();
    } else {
        std::cerr << "Failed to get zombie from pool" << std::endl;
    }
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render tilemap first (background), adjusted by camera
    if (chunkManager) {
        chunkManager->Render(camera);
    }    // Render zombies using the pool
    if (zombiePool) {
        zombiePool->Render(renderer, camera);
    }

    // Render player on top of tilemap, adjusted by camera
    player->Render(renderer, camera);
    
    // Render UI with player's current health and ammo
    ui->Render(player->GetHealth(), player->GetMaxHealth(), player->GetCurrentAmmo(), player->GetMaxAmmo());

    SDL_RenderPresent(renderer);
}

void Game::Run() {
    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        // Add deltaTime to accumulator
        accumulator += deltaTime;

        // Handle events
        HandleEvents();

        // Update with fixed time step
        while (accumulator >= FIXED_TIME_STEP) {
            Update(FIXED_TIME_STEP);
            accumulator -= FIXED_TIME_STEP;
        }

        // Render
        Render();

        // Cap frame rate
        int frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frameTime);
        }
    }
}

void Game::Cleanup() {    
    // Clean up zombie pool
    if (zombiePool) {
        try {
            delete zombiePool;
        } catch (const std::exception& e) {
            std::cerr << "Error cleaning up zombie pool: " << e.what() << std::endl;
        }
        zombiePool = nullptr;
    }

    if (waveManager) {
        delete waveManager;
        waveManager = nullptr;
    }

    if (player) {
        delete player;
        player = nullptr;
    }

    if (ui) {
        delete ui;
        ui = nullptr;
    }

    if (chunkManager) { // Add ChunkManager cleanup
        delete chunkManager;
        chunkManager = nullptr;
    }

    if (camera) { // Add camera cleanup
        delete camera;
        camera = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    IMG_Quit();
    SDL_Quit();
}