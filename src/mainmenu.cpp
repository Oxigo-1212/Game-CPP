#include "include/MainMenu.h"
#include <iostream>

MainMenu::MainMenu(SDL_Renderer* renderer) : 
    renderer(renderer),
    titleFont(nullptr),
    buttonFont(nullptr),
    titleTexture(nullptr),
    backgroundTexture(nullptr),
    startButton(nullptr),
    optionsButton(nullptr),
    exitButton(nullptr),
    startGame(false),
    exitGame(false) {
}

MainMenu::~MainMenu() {
    // Clean up title texture
    if (titleTexture) {
        SDL_DestroyTexture(titleTexture);
        titleTexture = nullptr;
    }
    
    // Clean up background texture
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
    
    // Clean up fonts
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    
    if (buttonFont) {
        TTF_CloseFont(buttonFont);
        buttonFont = nullptr;
    }
    
    // Clean up buttons
    if (startButton) {
        delete startButton;
        startButton = nullptr;
    }
    
    if (optionsButton) {
        delete optionsButton;
        optionsButton = nullptr;
    }
    
    if (exitButton) {
        delete exitButton;
        exitButton = nullptr;
    }
}

bool MainMenu::Initialize() {
    // Load fonts
    titleFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 72);
    if (!titleFont) {
        std::cerr << "Failed to load title font: " << TTF_GetError() << std::endl;
        return false;
    }

    buttonFont = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 32);
    if (!buttonFont) {
        std::cerr << "Failed to load button font: " << TTF_GetError() << std::endl;
        return false;
    }

    // Create title texture
    SDL_Color titleColor = {255, 255, 255, 255};
    SDL_Surface* titleSurface = TTF_RenderText_Blended(titleFont, "Zombie Shooter", titleColor);
    if (!titleSurface) {
        std::cerr << "Failed to create title surface: " << TTF_GetError() << std::endl;
        return false;
    }

    titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_FreeSurface(titleSurface);

    if (!titleTexture) {
        std::cerr << "Failed to create title texture: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create buttons - centered horizontally, stacked vertically
    int buttonX = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
    int buttonY = SCREEN_HEIGHT / 2; // Start buttons from middle of screen
    
    startButton = new Button(renderer, buttonFont, "Start Game", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    buttonY += BUTTON_HEIGHT + BUTTON_PADDING;
    optionsButton = new Button(renderer, buttonFont, "Options", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    buttonY += BUTTON_HEIGHT + BUTTON_PADDING;
    exitButton = new Button(renderer, buttonFont, "Exit", buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT);
    
    // Optional: Load background texture
    /*
    SDL_Surface* bgSurface = IMG_Load("assets/menu_background.png");
    if (bgSurface) {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }
    */
    
    return true;
}

void MainMenu::HandleEvents(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        int mouseX, mouseY;
        bool mouseDown = (event.type == SDL_MOUSEBUTTONDOWN);
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Update buttons with mouse state
        startButton->Update(mouseX, mouseY, mouseDown);
        optionsButton->Update(mouseX, mouseY, mouseDown);
        exitButton->Update(mouseX, mouseY, mouseDown);
    }
}

void MainMenu::Update(float deltaTime) {
    // Check for button clicks
    if (startButton->IsClicked()) {
        startGame = true;
    }
    
    if (exitButton->IsClicked()) {
        exitGame = true;
    }
    
    // Options button functionality can be added later
}

void MainMenu::Render() {
    // Clear the screen with a dark color
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Dark blue-ish color
    SDL_RenderClear(renderer);
    
    // Draw background if available
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }
    
    // Draw title
    if (titleTexture) {
        int titleWidth, titleHeight;
        SDL_QueryTexture(titleTexture, nullptr, nullptr, &titleWidth, &titleHeight);
        SDL_Rect titleRect = {
            (SCREEN_WIDTH - titleWidth) / 2,  // Center horizontally
            SCREEN_HEIGHT / 4 - titleHeight / 2,  // Position at 1/4 of screen height
            titleWidth,
            titleHeight
        };
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
    }
    
    // Draw buttons
    startButton->Render(renderer);
    optionsButton->Render(renderer);
    exitButton->Render(renderer);
    
    // Present the final rendered image
    SDL_RenderPresent(renderer);
}

bool MainMenu::ShouldStartGame() const {
    return startGame;
}

bool MainMenu::ShouldExitGame() const {
    return exitGame;
}

void MainMenu::Reset() {
    // Reset the menu flags so we can go back to main menu properly
    startGame = false;
    exitGame = false;

    // Reset button states if needed
    if (startButton) {
        startButton->Reset();
    }
    if (optionsButton) {
        optionsButton->Reset();
    }
    if (exitButton) {
        exitButton->Reset();
    }
}