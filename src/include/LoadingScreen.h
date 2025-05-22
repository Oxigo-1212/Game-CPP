#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "Constants.h"

class LoadingScreen {
public:
    LoadingScreen(SDL_Renderer* renderer);
    ~LoadingScreen();

    void Render(float progress, const std::string& message);
    void SetMessage(const std::string& message) { currentMessage = message; }

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::string currentMessage;
      // Loading bar dimensions
    static constexpr int BAR_WIDTH = 400;
    static constexpr int BAR_HEIGHT = 30;
    // Use dynamic window dimensions from Constants namespace
};