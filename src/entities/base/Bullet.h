#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>

#include "utils/Constants.h"
#include "utils/Structures.h"

class Bullet {
public:
    float x, y, vx, vy;
    bool active;
    bool fromEnemy;
    int damage;
    bool isSpecial;

    Bullet(float x_, float y_, float vx_, float vy_, bool enemy, int damage_ = 10, bool special = false);

    void update(float deltaTime);
    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
};

#endif // !BULLET_H
