#ifndef POWERUP_H
#define POWERUP_H

#include <SDL.h>

#include "Structures.h"

class PowerUp {
public:
    float x, y;
    bool active;
    PowerUpType type;
    Uint32 spawnTime;
    SDL_Texture* texture;

    PowerUp(float x_, float y_, PowerUpType type_);

    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
};

#endif // !POWERUP_H
