#include "utils/ParticleSystem.h"

#include <cmath>
#include <random>
#include <chrono>

#include "utils/Constants.h"

using namespace std;

ParticleSystem::ParticleSystem(int maxParticles) {
    particles.resize(maxParticles);
    for (auto& p : particles) {
        p.active = false;
    }
}

void ParticleSystem::emit(float x, float y, float angle, int count, SDL_Color color, int life) {
    uniform_real_distribution<float> angleDist(-0.5f, 0.5f);
    uniform_real_distribution<float> speedDist(0.5f, 2.0f);
    uniform_int_distribution<int> lifeDist(life / 2, life);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    for (int i = 0; i < count; ++i) {
        // Find inactive particle
        for (auto& p : particles) {
            if (!p.active) {
                p.x = x;
                p.y = y;
                float particleAngle = angle + angleDist(rng);
                float speed = speedDist(rng);
                p.vx = cos(particleAngle) * speed;
                p.vy = sin(particleAngle) * speed;
                p.life = lifeDist(rng);
                p.maxLife = p.life;
                p.color = color;
                p.active = true;
                break;
            }
        }
    }
}

void ParticleSystem::emitCircle(float x, float y, float radius, int count, SDL_Color color, int life) {
    uniform_real_distribution<float> angleDist(0, 2 * M_PI);
    uniform_real_distribution<float> radiusDist(0.8f * radius, 1.2f * radius);
    uniform_real_distribution<float> speedDist(0.2f, 0.8f);
    uniform_int_distribution<int> lifeDist(life / 2, life);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    for (int i = 0; i < count; ++i) {
        // Find inactive particle
        for (auto& p : particles) {
            if (!p.active) {
                float angle = angleDist(rng);
                float particleRadius = radiusDist(rng);
                p.x = x + cos(angle) * particleRadius;
                p.y = y + sin(angle) * particleRadius;

                // Particles move slightly outward
                float speed = speedDist(rng);
                p.vx = cos(angle) * speed;
                p.vy = sin(angle) * speed;

                p.life = lifeDist(rng);
                p.maxLife = p.life;
                p.color = color;
                p.active = true;
                break;
            }
        }
    }
}

void ParticleSystem::update() {
    for (auto& p : particles) {
        if (p.active) {
            p.x += p.vx;
            p.y += p.vy;
            p.life--;
            if (p.life <= 0) {
                p.active = false;
            }
        }
    }
}

void ParticleSystem::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    for (const auto& p : particles) {
        if (p.active) {
            // Fade out based on remaining life
            float alpha = static_cast<float>(p.life) / p.maxLife;
            SDL_SetRenderDrawColor(
                renderer,
                p.color.r,
                p.color.g,
                p.color.b,
                static_cast<Uint8>(255 * alpha)
            );

            SDL_Rect rect = {
                static_cast<int>(p.x - cameraX),
                static_cast<int>(p.y - cameraY),
                2,
                2
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
