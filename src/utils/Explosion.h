#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <SDL.h>

class Explosion {
public:
    float x, y;
    Uint32 startTime;
    bool active;
    SDL_Texture* texture;
    bool isSpecial;

    Explosion(float x_, float y_, bool special = false);

    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
};

#endif // !EXPLOSION_H
