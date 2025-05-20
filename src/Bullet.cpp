#include "Bullet.h"


Bullet::Bullet(float x_, float y_, float vx_, float vy_, bool enemy, int damage_, bool special)
    : x(x_), y(y_), vx(vx_), vy(vy_),
      active(true), fromEnemy(enemy), damage(damage_), isSpecial(special) {
}

void Bullet::update(float deltaTime) {
    if (!active) {
        return;
    }

    x += vx * deltaTime * 60.0f;
    y += vy * deltaTime * 60.0f;
    if (x < BORDER_OFFSET || x > MAP_WIDTH - BORDER_OFFSET || y < BORDER_OFFSET || y > MAP_HEIGHT - BORDER_OFFSET) {
        active = false;
    }
}

void Bullet::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!active) {
        return;
    }

    if (isSpecial) {
        // Special bullets are larger and purple
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_Rect rect = {static_cast<int>(x - 5 - cameraX), static_cast<int>(y - 5 - cameraY), 10, 10};
        SDL_RenderFillRect(renderer, &rect);
    } else {
        // Regular bullets
        SDL_SetRenderDrawColor(renderer, fromEnemy ? 0 : 255, 0, fromEnemy ? 255 : 0, 255);
        SDL_Rect rect = {static_cast<int>(x - 3 - cameraX), static_cast<int>(y - 3 - cameraY), 6, 6};
        SDL_RenderFillRect(renderer, &rect);
    }
}
