#include "Tank.h"

#include <cmath>

Tank::Tank(float x_, float y_, SDL_Texture* tex, EnemyType type_)
    : x(x_), y(y_), vx(0), vy(0), angle(0),
      texture(tex), shieldTexture(nullptr), lastShotTime(0), alive(true),
      hp(100), maxHp(100), isShooting(false), isShielding(false),
      currentFrame(0), shieldFrame(0), lastFrameTime(0), lastShieldFrameTime(0),
      width(150), height(50), collisionRadius(30),
      speed(1.0f), damage(10), type(type_), isPlayer(false),
      specialBullets(0), isSpecialActive(false), specialActivationTimer(0),
      healthPickups(0), isRegeneratingHealth(false), healthRegenTimer(0), healthRegenTickTimer(0) {

    if (type == EnemyType::FAST) {
        speed = 1.5f;
        hp = 70;
        maxHp = 70;
        damage = 5;
        width = 80;
        height = 40;
        collisionRadius = 25;
    } else if (type == EnemyType::HEAVY) {
        speed = 0.7f;
        hp = 150;
        maxHp = 150;
        damage = 15;
        width = 100;
        height = 50;
        collisionRadius = 35;
    }
}

void Tank::update(float deltaTime) {
    if (!alive) {
        return;
    }

    float maxSpeed = speed * 3.0f;
    if (vx > maxSpeed) vx = maxSpeed;
    if (vx < -maxSpeed) vx = -maxSpeed;
    if (vy > maxSpeed) vy = maxSpeed;
    if (vy < -maxSpeed) vy = -maxSpeed;

    x += vx * deltaTime * 60.0f;
    y += vy * deltaTime * 60.0f;


    vx *= 0.95f;
    vy *= 0.95f;


    if (abs(vx) < 0.01f) vx = 0;
    if (abs(vy) < 0.01f) vy = 0;


    Uint32 currentTime = SDL_GetTicks();
    if (isShielding && currentTime - lastShieldFrameTime >= TANK_FRAME_DELAY) {
        shieldFrame = (shieldFrame + 1) % TANK_FRAME_COUNT;
        lastShieldFrameTime = currentTime;
    }


    if (isRegeneratingHealth) {
        healthRegenTimer -= deltaTime;
        healthRegenTickTimer -= deltaTime;

        if (healthRegenTickTimer <= 0) {

            healthRegenTickTimer = HEALTH_REGEN_TICK;
        }

        if (healthRegenTimer <= 0) {

            isRegeneratingHealth = false;
        }
    }
}

void Tank::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!alive) {
        return;
    }

    Uint32 currentTime = SDL_GetTicks();

    SDL_Texture* currentTexture = texture;
    int frame = 0;

    if (isShielding && shieldTexture != nullptr) {
        currentTexture = shieldTexture;
        frame = shieldFrame;
    } else if (isShooting) {
        if (currentTime - lastFrameTime >= TANK_FRAME_DELAY) {
            currentFrame++;
            if (currentFrame >= TANK_FRAME_COUNT) {
                currentFrame = 0;
                isShooting = false;
            }
            lastFrameTime = currentTime;
        }
        frame = currentFrame;
    }

    SDL_Rect srcRect = {frame * TANK_FRAME_WIDTH, 0, TANK_FRAME_WIDTH, TANK_FRAME_HEIGHT};
    SDL_Rect destRect = {
        static_cast<int>(x - width / 2 - cameraX),
        static_cast<int>(y - height / 2 - cameraY),
        width, height
    };

    // Điều chỉnh tâm quay - đặt ở giữa xe tăng
    SDL_Point center;
    center.x = width / 2; // Tâm quay ở giữa chiều rộng
    center.y = height / 2; // Tâm quay ở giữa chiều cao

    SDL_RenderCopyEx(renderer, currentTexture, &srcRect, &destRect, angle * 180.0 / M_PI, &center, SDL_FLIP_NONE);
}

void Tank::renderHealthBar(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!alive || isPlayer) {
        return; 
    }

    int barWidth = width;
    int barHeight = 5;
    int offsetY = -(height / 2 + 10);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect bgRect = {
        static_cast<int>(x - barWidth / 2 - cameraX),
        static_cast<int>(y + offsetY - cameraY),
        barWidth, barHeight
    };
    SDL_RenderFillRect(renderer, &bgRect);
    float hpRatio = static_cast<float>(hp) / maxHp;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect hpRect = {
        static_cast<int>(x - barWidth / 2 - cameraX),
        static_cast<int>(y + offsetY - cameraY),
        static_cast<int>(barWidth * hpRatio), barHeight
    };
    SDL_RenderFillRect(renderer, &hpRect);
}


void Tank::getBulletSpawnPosition(float& outX, float& outY) {
    outX = x + collisionRadius * cos(angle);
    outY = y + collisionRadius * sin(angle);
}
