#pragma once
#include <SDL2/SDL.h>
#include <algorithm> // For std::min/max if clamping or for lerp smoothing

class Camera {
public:
    // Constructor: Initializes the camera with a starting position and screen dimensions.
    // screenWidth and screenHeight are the dimensions of your game window.
    Camera(float startX, float startY, int screenWidth, int screenHeight);
    ~Camera();

    // Updates the camera's position.
    // targetCenterX, targetCenterY: The world coordinates the camera should try to center on (e.g., player's center).
    // deltaTime: Time elapsed since the last frame, for smooth movement.
    void Update(float targetCenterX, float targetCenterY, float deltaTime);

    // Manually sets the camera's top-left position.
    void SetPosition(float x, float y);

    // Returns the camera's top-left X coordinate.
    float GetX() const { return x; }
    // Returns the camera's top-left Y coordinate.
    float GetY() const { return y; }

    // Returns the camera's view width (screen width).
    int GetViewWidth() const { return viewWidth; }
    // Returns the camera's view height (screen height).
    int GetViewHeight() const { return viewHeight; }

    // Converts world coordinates to screen coordinates.
    // Useful for rendering objects relative to the camera.
    SDL_FPoint WorldToScreen(float worldX, float worldY) const;

    // Converts screen coordinates (e.g., mouse position) to world coordinates.
    // Useful for interacting with the game world via mouse input.
    SDL_FPoint ScreenToWorld(float screenX, float screenY) const;

private:
    float x, y;                 // Camera's top-left position in the world.
    int viewWidth, viewHeight;  // Dimensions of the camera's viewport (usually screen size).

    float followSpeed;          // Speed at which the camera follows the target. Adjust for desired smoothness.

    // Linear interpolation function for smooth movement.
    float lerp(float start, float end, float t) const;
};
