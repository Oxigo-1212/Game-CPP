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
    static constexpr float NOTIFICATION_DURATION = 3.0f;  // How long notifications stay on screen
    static constexpr int WAVE_INFO_Y = 20;  // Wave info appears at top of screen
    static constexpr int WINDOW_WIDTH = 1280;  // Match your game window width
    static constexpr int WINDOW_HEIGHT = 720;  // Match your game window height

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
    SDL_Texture* waveInfoTexture;
    SDL_Rect waveInfoRect;

    // Notification system
    std::string notificationText;
    float notificationTimer;
    SDL_Texture* notificationTexture;
    SDL_Rect notificationRect;

public:
    UI(SDL_Renderer* renderer);
    ~UI();
    bool Initialize();
    void Cleanup();
    void Render(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo);
    void ShowNotification(const std::string& text);
    void UpdateNotification(float deltaTime);
    void UpdateWaveInfo(int currentWave, int zombiesRemaining, float spawnTimer);
    
    // Game state UI methods
    void RenderPauseScreen();
    void RenderGameOverScreen(int waveReached);

private:
    void RenderHealthBar(int currentHealth, int maxHealth);
    void RenderAmmoCounter(int currentAmmo, int maxAmmo);
    void RenderNotification();
    void RenderWaveInfo();
    void UpdateTextTextures(int currentHealth, int maxHealth, int currentAmmo, int maxAmmo);
    SDL_Texture* CreateTextTexture(const std::string& text);
    SDL_Texture* CreateTextTexture(const std::string& text, SDL_Color color, int fontSize = FONT_SIZE);
};
