#include "include/Game.h"
#include "include/ChunkManager.h" // Ensure ChunkManager is included
#include "include/WaveConfig.h" // Include for weapon unlock wave constants
#include "include/Constants.h" // Include for dynamic window dimensions
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game() : 
    isRunning(false), 
    window(nullptr), 
    renderer(nullptr),
    currentState(GameState::MAIN_MENU),
    mainMenu(nullptr),
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
    waveManager(nullptr),
    loadingScreen(nullptr) {
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
    
    // Create window in fullscreen desktop mode to fill the entire display
    window = SDL_CreateWindow(
        "Zombie Shooter",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
      // Get the actual display dimensions after creating the fullscreen window
    int actualWidth, actualHeight;
    SDL_GetWindowSize(window, &actualWidth, &actualHeight);
    std::cout << "Display dimensions: " << actualWidth << "x" << actualHeight << std::endl;
    
    // Update the constants in the Constants namespace with the actual values
    Constants::WINDOW_WIDTH = actualWidth;
    Constants::WINDOW_HEIGHT = actualHeight;
    
    // Display message confirming fullscreen mode
    std::cout << "Game running in fullscreen mode at " << Constants::WINDOW_WIDTH << "x" << Constants::WINDOW_HEIGHT << std::endl;    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize the loading screen right after creating the renderer
    loadingScreen = std::make_unique<LoadingScreen>(renderer);
    
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }
      // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Initialize UI first since MainMenu needs it for high scores
    ui = new UI(renderer);
    if (!ui->Initialize()) {
        std::cerr << "UI initialization failed" << std::endl;
        return false;
    }
    
    // Initialize main menu with UI reference
    mainMenu = new MainMenu(renderer, ui);
    if (!mainMenu->Initialize()) {
        std::cerr << "Main menu initialization failed" << std::endl;
        return false;
    }

    // Initialize game state - only needed when switching to PLAYING state
    // InitializeGameState();

    isRunning = true;
    previousTime = SDL_GetTicks();
    return true;
}

void Game::UpdateWindowSize(int width, int height) {
    // Update the global constants
    Constants::WINDOW_WIDTH = width;
    Constants::WINDOW_HEIGHT = height;
    
    std::cout << "Window size updated to: " << width << "x" << height << std::endl;
    
    // Update camera dimensions if it exists
    if (camera) {
        camera->SetViewDimensions(width, height);
    }
    
    // No need to update UI elements as they now use the dynamic Constants values
}

void Game::ToggleFullscreen() {
    // Check current fullscreen state
    Uint32 currentFlags = SDL_GetWindowFlags(window);
    bool isCurrentlyFullscreen = (currentFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
      if (isCurrentlyFullscreen) {
        // Switch to windowed mode
        std::cout << "Switching to windowed mode" << std::endl;
        SDL_SetWindowFullscreen(window, 0);
        // Set a standard windowed size
        SDL_SetWindowSize(window, 1280, 720);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        
        // Show notification if UI exists
        if (ui) {
            ui->ShowNotification("Windowed Mode");
        }
    } else {
        // Switch to fullscreen mode
        std::cout << "Switching to fullscreen mode" << std::endl;
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        
        // Show notification if UI exists
        if (ui) {
            ui->ShowNotification("Fullscreen Mode");
        }
    }
    
    // Update window size in our constants
    int newWidth, newHeight;
    SDL_GetWindowSize(window, &newWidth, &newHeight);
    UpdateWindowSize(newWidth, newHeight);
}

void Game::InitializeGameState() {    
    try {
        // Use the existing loading screen (created in Initialize) 
        // or create it if it doesn't exist for some reason
        if (!loadingScreen) {
            loadingScreen = std::make_unique<LoadingScreen>(renderer);
        }
        loadingScreen->Render(0.0f, "Initializing game components...");
        // Create and initialize UI first (doesn't depend on other components)
        ui = new UI(renderer);
        if (!ui->Initialize()) {
            throw std::runtime_error("UI initialization failed");
        }
        loadingScreen->Render(0.1f, "Creating user interface...");
        
        // Create temporary wave manager just for player initialization
        // Will be replaced with the real one later
        waveManager = new WaveManager();
          // Create player instance
        player = new Player(renderer, waveManager, ui);
        loadingScreen->Render(0.2f, "Creating player...");// Create camera instance
        camera = new Camera(player->GetX() - Constants::WINDOW_WIDTH / 2.0f + player->GetDestRect().w / 2.0f, 
                          player->GetY() - Constants::WINDOW_HEIGHT / 2.0f + player->GetDestRect().h / 2.0f, 
                          Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
        loadingScreen->Render(0.4f, "Creating camera...");
        
        // Initialize chunk manager
        chunkManager = new ChunkManager(renderer, player, "assets/maps/grasstiles.csv", "assets/tilesets/Grass 13  .png");
        loadingScreen->Render(0.5f, "Loading map chunks...");
        
        // Initialize zombie pool with loading updates
        std::cout << "Game: Creating zombie pool..." << std::endl;
        loadingScreen->Render(0.6f, "Creating zombie pool...");        // Create zombies gradually to show progress
        zombiePool = new ZombiePool(renderer, ZOMBIE_POOL_SIZE);
        for(size_t i = 0; i < ZOMBIE_POOL_SIZE; i++) {        zombiePool->AddZombie(); // Add zombies one by one
            if(i % 10 == 0 || i == ZOMBIE_POOL_SIZE - 1) { // Update progress every 10 zombies and at the end
                float progress = 0.6f + (0.3f * static_cast<float>(i + 1) / ZOMBIE_POOL_SIZE);
                loadingScreen->Render(progress, "Creating zombie pool... " + 
                    std::to_string(i + 1) + "/" + std::to_string(ZOMBIE_POOL_SIZE));
            }
        }
        
        loadingScreen->Render(0.9f, "Initializing wave manager...");
        
        // Replace temporary wave manager with final one now that all systems are ready
        WaveManager* finalWaveManager = new WaveManager();
        
        // Update player with the final wave manager
        if (player) {
            player->SetWaveManager(finalWaveManager);
        }
        
        // Delete temporary wave manager and replace with final one
        delete waveManager;
        waveManager = finalWaveManager;
        
        // Start first wave
        waveManager->StartNextWave();
        
        loadingScreen->Render(1.0f, "Game initialization complete!");
        
        // Short delay to show completion
        SDL_Delay(500);

        // Clear loading screen
        loadingScreen.reset();
    } 
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize game state: " << e.what() << std::endl;
        CleanupGameState();  // Clean up any partially initialized state
    }
}

void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        
        // Handle F11 key for toggling fullscreen mode in all game states
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F11) {
            ToggleFullscreen();
        }
        
        // Handle window resize events in all game states
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            int newWidth = event.window.data1;
            int newHeight = event.window.data2;
            std::cout << "Window resized to: " << newWidth << "x" << newHeight << std::endl;
            
            // Update the global constants
            UpdateWindowSize(newWidth, newHeight);
        }
        
        switch (currentState) {
            case GameState::MAIN_MENU:
                // Handle main menu events
                mainMenu->HandleEvents(event);
                break;
                
            case GameState::PLAYING:
                // Handle gameplay events
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && event.type == SDL_KEYDOWN) {
                    currentState = GameState::PAUSED;
                } else if (event.type == SDL_MOUSEMOTION && camera && player) {
                    // Convert screen mouse coordinates to world coordinates for the player
                    SDL_FPoint worldMousePos = camera->ScreenToWorld(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
                    player->UpdateMousePosition(static_cast<int>(worldMousePos.x), static_cast<int>(worldMousePos.y));
                } else if (player) {
                    player->HandleInput(event);
                }
                break;                case GameState::PAUSED:
                // Handle pause menu events
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && event.type == SDL_KEYDOWN) {
                    currentState = GameState::PLAYING;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_M && event.type == SDL_KEYDOWN) {
                    // Return to main menu when M is pressed
                    CleanupGameState();
                    currentState = GameState::MAIN_MENU;
                    
                    // Reset the main menu flags
                    if (mainMenu) {
                        mainMenu->Reset();
                    }
                }
                break;
                
                case GameState::GAME_OVER:            // Handle game over events
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    // Restart the game directly
                    CleanupGameState();
                    currentState = GameState::LOADING;
                    InitializeGameState();
                    currentState = GameState::PLAYING;
                } 
                else if (event.key.keysym.scancode == SDL_SCANCODE_M || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    // Return to main menu
                    CleanupGameState();
                    currentState = GameState::MAIN_MENU;
                    
                    // Reset the main menu flags
                    if (mainMenu) {
                        mainMenu->Reset();
                    }
                }
            }
            break;
                
            case GameState::HIGH_SCORES:
                // Handle high score screen events
                if (event.type == SDL_KEYDOWN && 
                    (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE || 
                     event.key.keysym.scancode == SDL_SCANCODE_M)) {
                    // Return to main menu
                    currentState = GameState::MAIN_MENU;
                    
                    // Reset menu flags
                    if (mainMenu) {
                        mainMenu->Reset();
                    }
                }
                break;
                
            default:
                break;
        }
    }
}

void Game::Update(float deltaTime) {    switch (currentState) {
        case GameState::MAIN_MENU:
            // Update main menu
            mainMenu->Update(deltaTime);
            
            // Check if we should start the game or exit
            if (mainMenu->ShouldStartGame()) {
                // Initialize game state when transitioning from menu to game
                currentState = GameState::LOADING;
                // Start loading the game in the next frame
                InitializeGameState();
                currentState = GameState::PLAYING;
            }
            
            if (mainMenu->ShouldExitGame()) {
                isRunning = false;
            }
              if (mainMenu->ShouldShowScores()) {
                // Reload high scores before transitioning to high scores screen
                if (ui) {
                    ui->ReloadHighScores();
                    std::cout << "Reloaded high scores before showing scores screen" << std::endl;
                }
                
                // Transition to high scores screen
                currentState = GameState::HIGH_SCORES;
                // Reset the flag to prevent repeated transitions
                mainMenu->Reset();
            }
            break;
        case GameState::PLAYING:
            // Regular game update logic
            if (player) {
                player->Update(deltaTime);
                // Check if player died
                if (player->IsDead()) {
                    std::cout << "Game Over!" << std::endl;
                    
                    // Save high score immediately when game ends
                    if (ui && waveManager) {
                        int waveReached = waveManager->GetCurrentWave();
                        ui->SaveHighScore(waveReached);
                        std::cout << "High score saved at game over: Wave " << waveReached << std::endl;
                    }
                    
                    currentState = GameState::GAME_OVER;
                    return; // Exit the function since we're changing states
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
                
                // Check for weapon unlocks and display notifications
                if (ui && waveManager->HasNewWeaponUnlock()) {
                    int currentWave = waveManager->GetCurrentWave();
                    if (currentWave == WaveConfig::RIFLE_UNLOCK_WAVE) {
                        ui->ShowNotification("Rifle Unlocked! Press 2 to equip");
                    }
                    else if (currentWave == WaveConfig::SHOTGUN_UNLOCK_WAVE) {
                        ui->ShowNotification("Shotgun Unlocked! Press 3 to equip");
                    }
                    waveManager->AcknowledgeWeaponUnlock();
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
                ui->UpdateNotification(deltaTime);            }

            // Update zombies through the pool
            if (zombiePool) {
                zombiePool->Update(deltaTime, player);                // Check collision with player's bullets
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
                    
                    for (Zombie* zombie : zombiePool->GetActiveZombies()) {
                        if (!zombie->IsDead() && zombie->CheckCollisionWithBullet(bullet)) {
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
              // Sync debug state between player and zombies
            if (player && zombiePool) {
                // Check if player's debug visualization state has changed
                static bool lastDebugState = false;
                bool currentDebugState = player->IsShowingDebugVisuals();
                
                if (currentDebugState != lastDebugState) {
                    // State changed, update zombies
                    zombiePool->SetDebugHitboxForAll(currentDebugState);
                    lastDebugState = currentDebugState;
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
            break;
            
        case GameState::PAUSED:
            // In pause state, we don't update the game
            break;
            
        case GameState::GAME_OVER:
            // Game over state, minimal updates
            break;
            
        default:
            break;
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background with full opacity
    SDL_RenderClear(renderer);

    switch (currentState) {
        case GameState::MAIN_MENU:
            // Render the main menu
            mainMenu->Render();
            break;
            
        case GameState::LOADING:
            // Render loading screen
            if (loadingScreen) {
                loadingScreen->Render(0.5f, "Loading game...");
            }
            break;
            
        case GameState::PLAYING:
            // Render the game world
            // Render tilemap first (background), adjusted by camera
            if (chunkManager) {
                chunkManager->Render(camera);
            }
            
            // Render zombies using the pool
            if (zombiePool) {
                zombiePool->Render(renderer, camera);
            }

            // Render player on top of tilemap, adjusted by camera
            if (player) {
                player->Render(renderer, camera);
            }
            
            // Render UI with player's current health and ammo
            if (ui && player) {
                ui->Render(player->GetHealth(), player->GetMaxHealth(), player->GetCurrentAmmo(), player->GetMaxAmmo());
            }
            break;
              case GameState::PAUSED:
            // First render the game world (frozen)
            if (chunkManager) {
                chunkManager->Render(camera);
            }
            
            if (zombiePool) {
                zombiePool->Render(renderer, camera);
            }
            
            if (player) {
                player->Render(renderer, camera);
            }
            
            if (ui && player) {
                ui->Render(player->GetHealth(), player->GetMaxHealth(), player->GetCurrentAmmo(), player->GetMaxAmmo());
                // Render pause screen overlay and text
                ui->RenderPauseScreen();
            }
            break;        case GameState::GAME_OVER:
            // Render game over screen with stats
            if (ui && waveManager) {
                // Get stats for the game over screen
                int waveReached = waveManager->GetCurrentWave();
                
                // Note: High score is already saved when transitioning to GAME_OVER state
                // We don't need to save it again here
                
                // Use the updated method with only wave information
                ui->RenderGameOverScreen(waveReached);
                // In this design:
                // - UI class manages loading and saving high scores
                // - MainMenu uses the UI's high scores for display
                // This eliminates redundancy while maintaining the same functionality
            } else {
                // Fallback if UI isn't initialized - clear the screen with black
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                
                // Display a basic game over message without the UI
                if (renderer && TTF_WasInit()) {
                    TTF_Font* font = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 48);
                    if (font) {
                        SDL_Color textColor = {255, 0, 0, 255}; // Red
                        SDL_Surface* surface = TTF_RenderText_Blended(font, "GAME OVER", textColor);
                        if (surface) {
                            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                            if (texture) {
                                SDL_Rect textRect = {
                                    (Constants::WINDOW_WIDTH - surface->w) / 2,
                                    (Constants::WINDOW_HEIGHT - surface->h) / 2,
                                    surface->w,
                                    surface->h
                                };
                                SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                                SDL_DestroyTexture(texture);
                            }
                            SDL_FreeSurface(surface);
                        }
                        TTF_CloseFont(font);
                    }
                }
            }
            break;              case GameState::HIGH_SCORES:
            // Render high scores screen using the main menu
            if (mainMenu) {
                mainMenu->RenderHighScores();
            } else {
                // Fallback if main menu isn't initialized - clear the screen with black
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                
                // Display a basic message
                if (renderer && TTF_WasInit()) {
                    TTF_Font* font = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 48);
                    if (font) {
                        SDL_Color textColor = {255, 215, 0, 255}; // Gold
                        SDL_Surface* surface = TTF_RenderText_Blended(font, "HIGH SCORES", textColor);
                        if (surface) {
                            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                            if (texture) {
                                SDL_Rect textRect = {
                                    (Constants::WINDOW_WIDTH - surface->w) / 2,
                                    Constants::WINDOW_HEIGHT / 3,
                                    surface->w,
                                    surface->h
                                };
                                SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                                SDL_DestroyTexture(texture);
                            }
                            SDL_FreeSurface(surface);
                        }
                        TTF_CloseFont(font);
                    }
                }
            }
            break;
            
        default:
            break;
    }

    SDL_RenderPresent(renderer);
}

void Game::Run() {
    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        // Cap deltaTime to prevent physics issues after long pauses
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }        // Process input
        HandleEvents();
        
        // Handle state-specific updates
        switch (currentState) {
            case GameState::MAIN_MENU:
                // Update handled in Update method below
                Update(deltaTime);
                break;
                
            case GameState::PLAYING:
                // Update game with fixed time step
                accumulator += deltaTime;
                while (accumulator >= FIXED_TIME_STEP) {
                    Update(FIXED_TIME_STEP);
                    accumulator -= FIXED_TIME_STEP;
                }
                break;
                
            case GameState::PAUSED:
                // No updates when paused
                break;
                
            case GameState::GAME_OVER:
                // Minimal updates for game over screen
                break;
                
            default:
                break;
        }

        // Render the current state
        Render();        // Cap frame rate
        int frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frameTime);
        }
    }
}

void Game::Cleanup() {    
    // Clean up main menu
    if (mainMenu) {
        delete mainMenu;
        mainMenu = nullptr;
    }
    
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

void Game::CleanupGameState() {
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

    if (chunkManager) {
        delete chunkManager;
        chunkManager = nullptr;
    }

    if (camera) {
        delete camera;
        camera = nullptr;
    }

    if (zombiePool) {
        delete zombiePool;
        zombiePool = nullptr;
    }
}