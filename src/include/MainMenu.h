#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include "Button.h"

class MainMenu {
public:
    MainMenu(SDL_Renderer* renderer);
    ~MainMenu();
    
    bool Initialize();
    void HandleEvents(const SDL_Event& event);
    void Update(float deltaTime);
    void Render();
      bool ShouldStartGame() const;
    bool ShouldExitGame() const;
    void Reset(); // Reset menu state
    
private:
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* buttonFont;
    
    SDL_Texture* titleTexture;
    SDL_Texture* backgroundTexture;
    
    Button* startButton;
    Button* optionsButton;
    Button* exitButton;
    
    bool startGame;
    bool exitGame;
    
    // Screen dimensions for positioning
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;
    
    // Button dimensions
    static constexpr int BUTTON_WIDTH = 200;
    static constexpr int BUTTON_HEIGHT = 50;
    static constexpr int BUTTON_PADDING = 20;
};