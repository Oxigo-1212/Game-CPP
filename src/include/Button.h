#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

enum class ButtonState {
    NORMAL,
    HOVER,
    PRESSED
};

class Button {
public:
    Button(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, int width, int height);
    ~Button();    void Update(int mouseX, int mouseY, bool mouseDown);
    void Render(SDL_Renderer* renderer);
    bool IsClicked() const;
    void SetText(const std::string& text);
    void SetPosition(int x, int y);
    void Reset();  // Added Reset method

private:
    bool Contains(int x, int y) const;
    void CreateTextTexture();

    SDL_Renderer* renderer;
    TTF_Font* font;
    std::string text;
    SDL_Rect rect;
    SDL_Texture* textTexture;
    ButtonState state;
    bool clicked;

    // Colors
    SDL_Color normalColor;
    SDL_Color hoverColor;
    SDL_Color pressedColor;
    SDL_Color textColor;
};