#include "KillNotification.h"

#include "Constants.h"

KillNotification::KillNotification(const string& text_) : text(text_), startTime(SDL_GetTicks()), active(true) {}

bool KillNotification::update() {
    if (!active) {
        return false;
    }

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - startTime >= KILL_NOTIFICATION_DURATION) {
        active = false;
        return false;
    }
    return true;
}

void KillNotification::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!active) {
        return;
    }

    Uint32 currentTime = SDL_GetTicks();
    float progress = (currentTime - startTime) / static_cast<float>(KILL_NOTIFICATION_DURATION);

    // Fade out near the end
    Uint8 alpha = progress > 0.7f ? static_cast<Uint8>(255 * (1.0f - (progress - 0.7f) / 0.3f)) : 255;

    SDL_Color textColor = {255, 0, 0, alpha};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (!textSurface) {
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_SetTextureAlphaMod(textTexture, alpha);

    SDL_Rect textRect = {
        WINDOW_WIDTH / 2 - textSurface->w / 2,
        50,
        textSurface->w,
        textSurface->h
    };

    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}