#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <SDL.h>
#include <vector>

#include "utils/Structures.h"

using namespace std;

class ParticleSystem {
private:
    vector<Particle> particles;

public:
    ParticleSystem(int maxParticles = 1000);

    void emit(float x, float y, float angle, int count, SDL_Color color, int life = 30);
    // New method to emit particles in a circle (for shield effect)
    void emitCircle(float x, float y, float radius, int count, SDL_Color color, int life = 30);
    void update();
    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
};

#endif // !PARTICLESYSTEM_H
