#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "Button.h"
#include "UI.h" // Added UI.h include

class MainMenu {
public:
    MainMenu(SDL_Renderer* renderer, UI* uiRef = nullptr);
    ~MainMenu();
    
    bool Initialize();
    void HandleEvents(const SDL_Event& event);
    void Update(float deltaTime);
    void Render();
    void RenderHighScores(); // New method to render high scores
    bool ShouldStartGame() const;
    bool ShouldExitGame() const;    bool ShouldShowScores() const;
    void Reset(); // Reset menu state    // Set the scale factor for high scores display
    void SetHighScoreScale(float scale);
    
    // Update button positions when window size changes
    void UpdateLayout();
    
private:
    // Using the ScoreEntry struct from UI class instead of duplicate definition
    using ScoreEntry = UI::ScoreEntry;
    
    void LoadHighScores();
    // Helper method to get scaled position values
    int Scale(int value) const;
    
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* buttonFont;
    TTF_Font* scoreFont;  // Separate font for high scores
    
    SDL_Texture* titleTexture;
    SDL_Texture* backgroundTexture;
    
    Button* startButton;
    Button* scoresButton; // Changed from optionsButton
    Button* exitButton;
      bool startGame;
    bool exitGame;
    bool showScores; // Added to track scores button
    
    UI* ui; // Reference to UI instance for high scores
    std::vector<ScoreEntry> highScores;
    float scoreScale;     // Scale factor for high score UI
    
    // Button dimensions
    static constexpr int BUTTON_WIDTH = 200;
    static constexpr int BUTTON_HEIGHT = 50;
    static constexpr int BUTTON_PADDING = 20;
};