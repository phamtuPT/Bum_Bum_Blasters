#include "Explosion.h"

#include "Constants.h"
#include "Structures.h"
#include "ResourceManager.h"

Explosion::Explosion(float x_, float y_, bool special)
    : x(x_), y(y_), startTime(SDL_GetTicks()), active(true), isSpecial(special) {
    texture = ResourceManager::getTexture("assets/images/effect/explosion.png");
}

void Explosion::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!active) {
        return;
    }

    Uint32 currentTime = SDL_GetTicks();
    float progress = (currentTime - startTime) / static_cast<float>(EXPLOSION_DURATION);
    if (progress > 1.0f) {
        active = false;
        return;
    }

    // Use texture for explosion instead of pixel-by-pixel rendering
    if (texture) {
        // Special explosions are larger
        int size = static_cast<int>(EXPLOSION_RADIUS * 2 * (1.0f - progress * 0.5f) * (isSpecial ? 2.0f : 1.0f));
        SDL_Rect destRect = {
            static_cast<int>(x - size / 2 - cameraX),
            static_cast<int>(y - size / 2 - cameraY),
            size,
            size
        };

        // Set alpha based on progress
        SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(255 * (1.0f - progress)));

        // Special explosions have a purple tint
        if (isSpecial) {
            SDL_SetTextureColorMod(texture, 255, 100, 255);
        } else {
            SDL_SetTextureColorMod(texture, 255, 255, 255);
        }

        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    } else {
        // Fallback to original rendering if texture not available
        int red = 255;
        int green = static_cast<int>(255 * progress);
        int blue = isSpecial ? 255 : 0;
        SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        int radius = static_cast<int>(EXPLOSION_RADIUS * (1.0f - progress) * (isSpecial ? 2.0f : 1.0f));
        int centerX = static_cast<int>(x - cameraX);
        int centerY = static_cast<int>(y - cameraY);

        // Draw filled circle
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = w - radius;
                int dy = h - radius;
                if (dx * dx + dy * dy <= radius * radius) {
                    SDL_Rect pixel = {centerX + dx, centerY + dy, 1, 1};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}
