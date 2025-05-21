#include "include/Button.h"
#include <iostream>

Button::Button(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, int width, int height)
    : renderer(renderer), font(font), text(text), clicked(false), state(ButtonState::NORMAL), textTexture(nullptr)
{
    rect = {x, y, width, height};
    
    // Default colors
    normalColor = {70, 70, 70, 255};      // Dark gray
    hoverColor = {120, 120, 120, 255};    // Medium gray
    pressedColor = {50, 50, 50, 255};     // Darker gray
    textColor = {255, 255, 255, 255};     // White
    
    CreateTextTexture();
}

Button::~Button() {
    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }
}

void Button::CreateTextTexture() {
    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }
    
    if (!font || text.empty()) {
        return;
    }
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), textColor);
    if (!surface) {
        std::cerr << "Failed to render text surface: " << TTF_GetError() << std::endl;
        return;
    }
    
    textTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!textTexture) {
        std::cerr << "Failed to create texture from text surface: " << SDL_GetError() << std::endl;
    }
}

bool Button::Contains(int x, int y) const {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void Button::Update(int mouseX, int mouseY, bool mouseDown) {
    clicked = false;
    
    if (Contains(mouseX, mouseY)) {
        if (mouseDown) {
            state = ButtonState::PRESSED;
        } else {
            if (state == ButtonState::PRESSED) {
                clicked = true;
            }
            state = ButtonState::HOVER;
        }
    } else {
        state = ButtonState::NORMAL;
    }
}

void Button::Render(SDL_Renderer* renderer) {
    // Draw button background based on state
    switch (state) {
        case ButtonState::NORMAL:
            SDL_SetRenderDrawColor(renderer, normalColor.r, normalColor.g, normalColor.b, normalColor.a);
            break;
        case ButtonState::HOVER:
            SDL_SetRenderDrawColor(renderer, hoverColor.r, hoverColor.g, hoverColor.b, hoverColor.a);
            break;
        case ButtonState::PRESSED:
            SDL_SetRenderDrawColor(renderer, pressedColor.r, pressedColor.g, pressedColor.b, pressedColor.a);
            break;
    }
    
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Draw button text
    if (textTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
        
        SDL_Rect textRect = {
            rect.x + (rect.w - textWidth) / 2,
            rect.y + (rect.h - textHeight) / 2,
            textWidth,
            textHeight
        };
        
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    }
}

bool Button::IsClicked() const {
    return clicked;
}

void Button::SetText(const std::string& newText) {
    text = newText;
    CreateTextTexture();
}

void Button::SetPosition(int x, int y) {
    rect.x = x;
    rect.y = y;
}

void Button::Reset() {
    // Reset button state to normal and clear clicked flag
    state = ButtonState::NORMAL;
    clicked = false;
}