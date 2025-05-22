#include "include/LoadingScreen.h"
#include "include/Constants.h"
#include <iostream>

LoadingScreen::LoadingScreen(SDL_Renderer* renderer) : renderer(renderer), font(nullptr), currentMessage("Loading...") {
    font = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }
}

LoadingScreen::~LoadingScreen() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

void LoadingScreen::Render(float progress, const std::string& message) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);    // Draw loading bar background
    // Create loading bar background rectangle (dark gray color: RGB 64, 64, 64)
    SDL_Rect barBg = {
        (Constants::WINDOW_WIDTH - BAR_WIDTH) / 2,
        (Constants::WINDOW_HEIGHT - BAR_HEIGHT) / 2,
        BAR_WIDTH,
        BAR_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255); 
    SDL_RenderFillRect(renderer, &barBg);

    // Draw loading bar progress
    SDL_Rect barFg = {
        (Constants::WINDOW_WIDTH - BAR_WIDTH) / 2,
        (Constants::WINDOW_HEIGHT - BAR_HEIGHT) / 2,
        static_cast<int>(BAR_WIDTH * progress),
        BAR_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &barFg);

    // Render message text
    if (font) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, message.c_str(), textColor);
        if (textSurface) {            
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                    (Constants::WINDOW_WIDTH - textSurface->w) / 2,
                    (Constants::WINDOW_HEIGHT - BAR_HEIGHT) / 2 - 40,
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }

    SDL_RenderPresent(renderer);
}