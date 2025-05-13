#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class UI {
private:    // Constants
    static constexpr int MARGIN_X = 20;
    static constexpr int MARGIN_Y = 50;  // Moved down from 20 to 50
    static constexpr int FONT_SIZE = 24;
    static constexpr int BAR_WIDTH = 200;
    static constexpr int BAR_HEIGHT = 20;
    static constexpr int TEXT_SPACING = 5;

    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Color textColor;
    SDL_Color barFillColor;
    SDL_Color barBackgroundColor;

    // Cache for rendered text textures
    SDL_Texture* ammoTexture;
    SDL_Rect ammoRect;
    SDL_Texture* healthTexture;
    SDL_Rect healthRect;

public:
    UI(SDL_Renderer* renderer);
    ~UI();

    bool Initialize();
    void Cleanup();
    void Render(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo);

private:
    void RenderHealthBar(int currentHealth, int maxHealth);
    void RenderAmmoCounter(int currentAmmo, int maxAmmo);
    SDL_Texture* CreateTextTexture(const std::string& text);
    void UpdateTextTextures(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo);
};
