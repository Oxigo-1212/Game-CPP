#include "include/Camera.h" // Adjusted path based on typical project structure
#include <algorithm> // For std::min, std::max

// Constructor
Camera::Camera(float startX, float startY, int screenW, int screenH)
    : x(startX), y(startY), viewWidth(screenW), viewHeight(screenH),
      followSpeed(5.0f) // Adjust this value to change camera follow responsiveness.
{}

Camera::~Camera() {}

// Linear interpolation - smoothen camera movement
float Camera::lerp(float start, float end, float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return start + t * (end - start);
}

void Camera::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

//pass in the player coordinates
void Camera::Update(float targetCenterX, float targetCenterY, float deltaTime) {
    // Calculate the desired top-left position for the camera to center the target.
    float desiredX = targetCenterX - viewWidth / 2.0f;
    float desiredY = targetCenterY - viewHeight / 2.0f;

    // Smoothly interpolate the camera's current position towards the desired position.
    // The factor std::min(followSpeed * deltaTime, 1.0f) ensures that the camera
    // doesn't overshoot if deltaTime is large or followSpeed is very high.
    // A small clamp on t for lerp is good practice.
    float t = followSpeed * deltaTime;
    t = std::max(0.0f, std::min(t, 1.0f)); // Clamp t between 0 and 1

    x = lerp(x, desiredX, t);
    y = lerp(y, desiredY, t);

    
}

SDL_FPoint Camera::WorldToScreen(float worldX, float worldY) const {
    return {worldX - x, worldY - y};
}

SDL_FPoint Camera::ScreenToWorld(float screenX, float screenY) const {
    return {screenX + x, screenY + y};
}

void Camera::SetViewDimensions(int width, int height) {
    viewWidth = width;
    viewHeight = height;
}
