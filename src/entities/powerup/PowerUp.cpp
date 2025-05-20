#include "entities/powerup/PowerUp.h"

#include <cmath>

#include "managers/ResourceManager.h"

PowerUp::PowerUp(float x_, float y_, PowerUpType type_)
    : x(x_), y(y_), active(true), type(type_), spawnTime(SDL_GetTicks()) {
    if (type_ == PowerUpType::HEALTH_PICKUP) {
        texture = ResourceManager::getTexture("assets/images/item/health_pickup.png");
    } else {
        texture = ResourceManager::getTexture("assets/images/item/powerup.png");
    }
}

void PowerUp::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!active) {
        return;
    }

    // Make power-up pulse
    Uint32 currentTime = SDL_GetTicks();
    float scale = 1.0f + 0.1f * sin((currentTime - spawnTime) / 200.0f);

    int size = static_cast<int>(30 * scale);
    SDL_Rect destRect = {
        static_cast<int>(x - size / 2 - cameraX),
        static_cast<int>(y - size / 2 - cameraY),
        size,
        size
    };

    // Different colors for different power-up types
    if (type == PowerUpType::HEALTH_PICKUP) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for health pickup
    } else {
        SDL_SetRenderDrawColor(
            renderer,
            type == PowerUpType::HEALTH ? 255 : 0,
            type == PowerUpType::SPEED ? 255 : 0,
            type == PowerUpType::DAMAGE ? 255 : 0,
            255
        );
    }

    if (texture) {
        // Set color mod for health pickup
        if (type == PowerUpType::HEALTH_PICKUP) {
            SDL_SetTextureColorMod(texture, 255, 100, 100);
        } else {
            SDL_SetTextureColorMod(texture, 255, 255, 255);
        }

        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    } else {
        // Fallback if texture not available
        SDL_RenderFillRect(renderer, &destRect);
    }
}
