#include "include/UI.h"
#include <sstream>

UI::UI(SDL_Renderer* renderer) : renderer(renderer), font(nullptr),
    textColor({255, 255, 255, 255}),         // White
    barFillColor({0, 255, 0, 255}),          // Green
    barBackgroundColor({128, 128, 128, 255}) // Gray
{
    ammoTexture = nullptr;
    healthTexture = nullptr;
}

UI::~UI() {
    Cleanup();
}

bool UI::Initialize() {
    // Initialize SDL_ttf if not already initialized
    if (TTF_WasInit() == 0) {
        if (TTF_Init() < 0) {
            SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
            return false;
        }
    }    // Load font
    font = TTF_OpenFont("assets/fonts/Call of Ops Duty.otf", FONT_SIZE);
    if (font == nullptr) {
        SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void UI::Cleanup() {
    if (ammoTexture) {
        SDL_DestroyTexture(ammoTexture);
        ammoTexture = nullptr;
    }
    if (healthTexture) {
        SDL_DestroyTexture(healthTexture);
        healthTexture = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

SDL_Texture* UI::CreateTextTexture(const std::string& text) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), textColor);
    if (!surface) {
        SDL_Log("Failed to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        SDL_Log("Failed to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        return nullptr;
    }

    return texture;
}

void UI::UpdateTextTextures(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo) {
    std::stringstream healthText, ammoText;
    
    // Update health text
    healthText << "Health: " << currentHealth << "/" << maxHealth;
    if (healthTexture) {
        SDL_DestroyTexture(healthTexture);
    }
    healthTexture = CreateTextTexture(healthText.str());
    SDL_QueryTexture(healthTexture, nullptr, nullptr, &healthRect.w, &healthRect.h);
    healthRect.x = MARGIN_X;
    healthRect.y = MARGIN_Y - healthRect.h - TEXT_SPACING;

    // Update ammo text
    ammoText << "Ammo: " << currentAmmo << "/" << maxAmmo;
    if (ammoTexture) {
        SDL_DestroyTexture(ammoTexture);
    }
    ammoTexture = CreateTextTexture(ammoText.str());
    SDL_QueryTexture(ammoTexture, nullptr, nullptr, &ammoRect.w, &ammoRect.h);
    ammoRect.x = MARGIN_X;
    ammoRect.y = MARGIN_Y + BAR_HEIGHT + TEXT_SPACING;
}

void UI::RenderHealthBar(int currentHealth, int maxHealth) {
    // Draw background
    SDL_Rect bgRect = {MARGIN_X, MARGIN_Y, BAR_WIDTH, BAR_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 
        barBackgroundColor.r, barBackgroundColor.g, barBackgroundColor.b, barBackgroundColor.a);
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw health bar
    float healthPercentage = static_cast<float>(currentHealth) / maxHealth;
    SDL_Rect fillRect = {
        MARGIN_X,
        MARGIN_Y,
        static_cast<int>(BAR_WIDTH * healthPercentage),
        BAR_HEIGHT
    };

    // Change color based on health percentage
    if (healthPercentage > 0.5f) {
        SDL_SetRenderDrawColor(renderer, barFillColor.r, barFillColor.g, barFillColor.b, barFillColor.a);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 
            static_cast<Uint8>(255 * (healthPercentage * 2)), 0, 255);
    }
    SDL_RenderFillRect(renderer, &fillRect);
}

void UI::RenderAmmoCounter(int currentAmmo, int maxAmmo) {
    if (ammoTexture) {
        SDL_RenderCopy(renderer, ammoTexture, nullptr, &ammoRect);
    }
}

void UI::Render(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo) {
    UpdateTextTextures(currentHealth, maxHealth, currentAmmo, maxAmmo);
    
    // Render health bar and text
    if (healthTexture) {
        SDL_RenderCopy(renderer, healthTexture, nullptr, &healthRect);
    }
    RenderHealthBar(currentHealth, maxHealth);
    
    // Render ammo counter
    RenderAmmoCounter(currentAmmo, maxAmmo);
}
