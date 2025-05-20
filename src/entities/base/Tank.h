#ifndef TANK_H
#define TANK_H

#include <SDL.h>

#include "utils/Constants.h"
#include "utils/Structures.h"

class Tank {
public:
    float x, y, vx, vy, angle;
    SDL_Texture* texture;
    SDL_Texture* shieldTexture;  // Texture for shield animation
    Uint32 lastShotTime;
    bool alive;
    int hp, maxHp;
    bool isShooting;
    bool isShielding;            // Shield state
    int currentFrame;
    int shieldFrame;             // Current frame for shield animation
    Uint32 lastFrameTime;
    Uint32 lastShieldFrameTime;  // Time of last shield frame update
    int width, height;
    float collisionRadius;
    float speed;
    int damage;
    EnemyType type;
    bool isPlayer; // Flag to indicate if this is the player tank
    int specialBullets;          // Count of special bullets accumulated
    bool isSpecialActive;        // Whether special ability is currently active
    float specialActivationTimer; // Timer for special ability activation
    int healthPickups;           // Count of health pickups collected
    bool isRegeneratingHealth;
    float healthRegenTimer;
    float healthRegenTickTimer;

    Tank(float x_, float y_, SDL_Texture* tex, EnemyType type_ = EnemyType::BASIC);

    void update(float deltaTime);
    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
    void renderHealthBar(SDL_Renderer* renderer, float cameraX, float cameraY);
    // Get bullet spawn position (for both regular and special bullets)
    void getBulletSpawnPosition(float& outX, float& outY);
};

#endif // !TANK_H
