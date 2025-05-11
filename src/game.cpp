#include "include/Game.h"
#include <iostream>

Game::Game() : 
    isRunning(false), 
    window(nullptr), 
    renderer(nullptr),
    previousTime(0),
    accumulator(0.0f),
    FPS(60),
    FRAME_TIME(1000 / FPS),
    FIXED_TIME_STEP(1.0f / 60.0f),
    player(nullptr) {
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
        800, 600,
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
    player = new Player(renderer);

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
            // Update player's aim direction
            player->UpdateMousePosition(event.motion.x, event.motion.y);
        } else {
            player->HandleInput(event);
        }
    }
}

void Game::Update(float deltaTime) {
    player->Update(deltaTime);
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    player->Render(renderer);

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