#include "include/Game.h"
#include <iostream>

Game::Game() : 
    isRunning(false), 
    window(nullptr), 
    renderer(nullptr),
    previousTime(0),
    accumulator(0.0f),    FPS(60),
    FRAME_TIME(1000 / FPS),
    FIXED_TIME_STEP(1.0f / 60.0f),
    player(nullptr),
    ui(nullptr),
    tilemap(nullptr),
    camera(nullptr) { // Initialize camera to nullptr
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

    // Create player instance
    player = new Player(renderer);    // Create and initialize UI
    ui = new UI(renderer);
    if (!ui->Initialize()) {
        std::cerr << "UI initialization failed" << std::endl;
        return false;
    }

    // Create and load tilemap
    tilemap = new TileMap(renderer);
    if (!tilemap->LoadTileset("assets/tilesets/Grass 13  .png")) {
        std::cerr << "Failed to load tileset" << std::endl;
        return false;
    }
    if (!tilemap->LoadMap("assets/maps/grasstiles.csv")) {
        std::cerr << "Failed to load map" << std::endl;
        return false;
    }

    // Create camera instance
    camera = new Camera(player->GetX() - SCREEN_WIDTH / 2.0f + player->GetDestRect().w / 2.0f, 
                        player->GetY() - SCREEN_HEIGHT / 2.0f + player->GetDestRect().h / 2.0f, 
                        SCREEN_WIDTH, SCREEN_HEIGHT);

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
    player->Update(deltaTime);
    // Update camera to follow the player's center
    SDL_Rect playerDest = player->GetDestRect();
    camera->Update(player->GetX() + playerDest.w / 2.0f, 
                   player->GetY() + playerDest.h / 2.0f, 
                   deltaTime);
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render tilemap first (background), adjusted by camera
    tilemap->Render(camera);

    // Render player on top of tilemap, adjusted by camera
    player->Render(renderer, camera);
    
    // Render UI with player's current health and ammo
    ui->Render(player->GetHealth(), player->GetMaxHealth(), player->GetAmmo(), player->GetMaxAmmo());

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
    if (player) {
        delete player;
        player = nullptr;
    }

    if (ui) {
        delete ui;
        ui = nullptr;
    }

    if (tilemap) {
        delete tilemap;
        tilemap = nullptr;
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