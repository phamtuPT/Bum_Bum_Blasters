#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <memory>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <array>
#include <random>
#include <chrono>

using namespace std;

// Constants
constexpr int WINDOW_WIDTH = 1500;
constexpr int WINDOW_HEIGHT = 750;
constexpr int MAP_WIDTH = 2000;
constexpr int MAP_HEIGHT = 1600;
constexpr float BULLET_SPEED = 10.0f;
constexpr float RECOIL_FORCE = 10.0f;
constexpr float BOUNCE_FACTOR = 0.9f;
constexpr int BORDER_OFFSET = 200;
constexpr int ENEMY_SPAWN_INTERVAL = 2000;
constexpr int ENEMY_COUNT_MAX = 5; // Increased max enemies
constexpr float ENEMY_SPEED = 1.0f;
constexpr Uint32 ENEMY_SHOOT_DELAY = 3500;
constexpr float TANK_COLLISION_FORCE = 1.5f;
constexpr int EXPLOSION_DURATION = 500;
constexpr int EXPLOSION_RADIUS = 30;

// Minimap constants
constexpr int MINIMAP_WIDTH = 200;
constexpr int MINIMAP_HEIGHT = 150;
constexpr int MINIMAP_X = 10;
constexpr int MINIMAP_Y = WINDOW_HEIGHT - MINIMAP_HEIGHT - 10;
constexpr float MINIMAP_SCALE = static_cast<float>(MINIMAP_WIDTH) / MAP_WIDTH;

// Shield constants
constexpr int SHIELD_DURATION = 10000;
constexpr int SHIELD_COOLDOWN = 30000;
constexpr int SHIELD_RADIUS = 40;

// Rapid Fire constants
constexpr int RAPID_FIRE_DURATION = 5000;
constexpr int RAPID_FIRE_COOLDOWN = 15000;
constexpr int RAPID_FIRE_INTERVAL = 100;

// Animation constants
constexpr int TANK_FRAME_COUNT = 5;
constexpr int TANK_FRAME_WIDTH = 445;
constexpr int TANK_FRAME_HEIGHT = 115;
constexpr int TANK_FRAME_DELAY = 50;

// Menu constants
constexpr int BUTTON_WIDTH = 200;
constexpr int BUTTON_HEIGHT = 50;
constexpr int BUTTON_SPACING = 20;

// Screen shake constants
constexpr int SCREEN_SHAKE_DURATION = 300;
constexpr float SCREEN_SHAKE_INTENSITY = 5.0f;

// Special bullet constants
constexpr int SPECIAL_BULLETS_REQUIRED = 10;  // Regular bullets needed to earn one special bullet
constexpr int MAX_SPECIAL_BULLETS = 5;        // Maximum special bullets that can be accumulated
constexpr float SPECIAL_ACTIVATION_TIME = 2.5f; // Time in seconds to hold right mouse button
constexpr float SPECIAL_ZOOM_FACTOR = 1.5f;   // How much to zoom out camera
constexpr int SPECIAL_LINE_LENGTH = 1000;     // Maximum length of targeting line

// Kill notification constants
constexpr int KILL_NOTIFICATION_DURATION = 2000; // 2 seconds

// Health pickup constants
constexpr int HEALTH_PICKUP_HEAL_AMOUNT = 50;
constexpr int HEALTH_PICKUP_SPAWN_INTERVAL = 20000; // 20 seconds

// Thêm hằng số cho thời gian hồi máu
constexpr float HEALTH_REGEN_TIME = 2.5f;  // 2.5 seconds
constexpr float HEALTH_REGEN_TICK = 0.1f;  // 0.1 second intervals

// Game states
enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER };

// Enemy types
enum class EnemyType { BASIC, FAST, HEAVY };

// Power-up types
enum class PowerUpType { HEALTH, SPEED, DAMAGE, SHIELD, RAPID_FIRE, HEALTH_PICKUP };

// Random number generator
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// Menu button types
enum class MenuButton {
    START,
    STATS,
    TUTORIAL,
    SETTINGS,
    EXIT
};

// Menu button structure
struct MenuButtonInfo {
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* hoverTexture;
    bool isHovered;
    MenuButton type;
};

// Resource Manager
class ResourceManager {
private:
    static unordered_map<string, SDL_Texture*> textures;
    static unordered_map<string, Mix_Chunk*> sounds;
    static unordered_map<string, Mix_Music*> music;

public:
    static void init(SDL_Renderer* renderer) {
        // Pre-load common textures
        loadTexture("assets/images/tank/player/tank_shoot_spritesheet.png", renderer);
        loadTexture("assets/images/tank/npc/enemy_tank.png", renderer);
        loadTexture("assets/images/effect/explosion.png", renderer);
        loadTexture("assets/images/item/powerup.png", renderer);
        loadTexture("assets/images/item/health_pickup.png", renderer);

        // Load shield texture (recolored tank spritesheet)
        // Note: You need to create this texture with different colors
        loadTexture("assets/images/tank/player/tank_shield_spritesheet.png", renderer);

        // Pre-load sounds
        loadSound("assets/sounds/shoot.mp3");
        loadSound("assets/sounds/rapid_fire.mp3");
        loadSound("assets/sounds/explosion.mp3");
        loadSound("assets/sounds/powerup.mp3");
        loadSound("assets/sounds/shield_activate.mp3");  // New sound for shield activation
        loadSound("assets/sounds/shield_deactivate.mp3"); // New sound for shield deactivation
        loadSound("assets/sounds/heal.mp3"); // New sound for healing

        // Pre-load music
        loadMusic("assets/sounds/background_music.mp3");
    }

    static void cleanup() {
        for (auto& pair : textures) {
            SDL_DestroyTexture(pair.second);
        }
        textures.clear();

        for (auto& pair : sounds) {
            Mix_FreeChunk(pair.second);
        }
        sounds.clear();

        for (auto& pair : music) {
            Mix_FreeMusic(pair.second);
        }
        music.clear();
    }

    static SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
        // Check if texture is already loaded
        auto it = textures.find(path);
        if (it != textures.end()) {
            return it->second;
        }

        // Load new texture
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            cerr << "Unable to create texture from " << path << "! SDL_Error: " << SDL_GetError() << endl;
            return nullptr;
        }

        textures[path] = texture;
        return texture;
    }

    // New function to create a recolored texture
    static SDL_Texture* createRecoloredTexture(const string& sourcePath, const string& newPath,
                                              SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b) {
        // Check if the recolored texture already exists
        auto it = textures.find(newPath);
        if (it != textures.end()) {
            return it->second;
        }

        // Load source texture to surface
        SDL_Surface* sourceSurface = IMG_Load(sourcePath.c_str());
        if (!sourceSurface) {
            cerr << "Unable to load image " << sourcePath << "! SDL_image Error: " << IMG_GetError() << endl;
            return nullptr;
        }

        // Create a copy of the surface
        SDL_Surface* newSurface = SDL_CreateRGBSurfaceWithFormat(0, sourceSurface->w, sourceSurface->h,
                                                                32, sourceSurface->format->format);
        if (!newSurface) {
            cerr << "Unable to create surface! SDL Error: " << SDL_GetError() << endl;
            SDL_FreeSurface(sourceSurface);
            return nullptr;
        }

        // Copy pixels
        SDL_BlitSurface(sourceSurface, NULL, newSurface, NULL);

        // Lock surface for pixel manipulation
        if (SDL_LockSurface(newSurface) < 0) {
            cerr << "Unable to lock surface! SDL Error: " << SDL_GetError() << endl;
            SDL_FreeSurface(sourceSurface);
            SDL_FreeSurface(newSurface);
            return nullptr;
        }

        // Modify pixels - simple color shift
        Uint32* pixels = static_cast<Uint32*>(newSurface->pixels);
        int pixelCount = newSurface->w * newSurface->h;

        SDL_PixelFormat* format = newSurface->format;

        for (int i = 0; i < pixelCount; ++i) {
            Uint8 sr, sg, sb, sa;
            SDL_GetRGBA(pixels[i], format, &sr, &sg, &sb, &sa);

            // Only modify non-transparent pixels
            if (sa > 0) {
                // Simple color shift - you can implement more complex recoloring logic
                Uint8 nr = min(255, static_cast<int>(sr * r / 255));
                Uint8 ng = min(255, static_cast<int>(sg * g / 255));
                Uint8 nb = min(255, static_cast<int>(sb * b / 255));

                pixels[i] = SDL_MapRGBA(format, nr, ng, nb, sa);
            }
        }

        SDL_UnlockSurface(newSurface);

        // Create texture from the modified surface
        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, newSurface);

        // Clean up surfaces
        SDL_FreeSurface(sourceSurface);
        SDL_FreeSurface(newSurface);

        if (!newTexture) {
            cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << endl;
            return nullptr;
        }

        // Store and return the new texture
        textures[newPath] = newTexture;
        return newTexture;
    }

    static SDL_Texture* getTexture(const string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) {
            return it->second;
        }
        return nullptr;
    }

    static Mix_Chunk* loadSound(const string& path) {
        // Check if sound is already loaded
        auto it = sounds.find(path);
        if (it != sounds.end()) {
            return it->second;
        }

        // Load new sound
        Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
        if (!sound) {
            cerr << "Unable to load sound " << path << "! SDL_mixer Error: " << Mix_GetError() << endl;
            return nullptr;
        }

        sounds[path] = sound;
        return sound;
    }

    static Mix_Chunk* getSound(const string& path) {
        auto it = sounds.find(path);
        if (it != sounds.end()) {
            return it->second;
        }
        return nullptr;
    }

    static Mix_Music* loadMusic(const string& path) {
        // Check if music is already loaded
        auto it = music.find(path);
        if (it != music.end()) {
            return it->second;
        }

        // Load new music
        Mix_Music* mus = Mix_LoadMUS(path.c_str());
        if (!mus) {
            cerr << "Unable to load music " << path << "! SDL_mixer Error: " << Mix_GetError() << endl;
            return nullptr;
        }

        music[path] = mus;
        return mus;
    }

    static Mix_Music* getMusic(const string& path) {
        auto it = music.find(path);
        if (it != music.end()) {
            return it->second;
        }
        return nullptr;
    }
};

// Initialize static members
unordered_map<string, SDL_Texture*> ResourceManager::textures;
unordered_map<string, Mix_Chunk*> ResourceManager::sounds;
unordered_map<string, Mix_Music*> ResourceManager::music;

// Tank Class
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

    Tank(float x_, float y_, SDL_Texture* tex, EnemyType type_ = EnemyType::BASIC)
        : x(x_), y(y_), vx(0), vy(0), angle(0),
        texture(tex), shieldTexture(nullptr), lastShotTime(0), alive(true),
        hp(100), maxHp(100), isShooting(false), isShielding(false),
        currentFrame(0), shieldFrame(0), lastFrameTime(0), lastShieldFrameTime(0),
        width(150), height(50), collisionRadius(30),
        speed(1.0f), damage(10), type(type_), isPlayer(false),
        specialBullets(0), isSpecialActive(false), specialActivationTimer(0),
        healthPickups(0), isRegeneratingHealth(false), healthRegenTimer(0), healthRegenTickTimer(0) {

        // Adjust properties based on enemy type
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

    void update(float deltaTime) {
        if (!alive) return;

        // Limit maximum velocity to prevent jitter
        float maxSpeed = speed * 3.0f;
        if (vx > maxSpeed) vx = maxSpeed;
        if (vx < -maxSpeed) vx = -maxSpeed;
        if (vy > maxSpeed) vy = maxSpeed;
        if (vy < -maxSpeed) vy = -maxSpeed;

        x += vx * deltaTime * 60.0f;
        y += vy * deltaTime * 60.0f;

        // Smooth deceleration
        vx *= 0.95f;
        vy *= 0.95f;

        // Set velocity to 0 if it's very small
        if (abs(vx) < 0.01f) vx = 0;
        if (abs(vy) < 0.01f) vy = 0;

        // Update shield animation if active
        Uint32 currentTime = SDL_GetTicks();
        if (isShielding && currentTime - lastShieldFrameTime >= TANK_FRAME_DELAY) {
            shieldFrame = (shieldFrame + 1) % TANK_FRAME_COUNT;
            lastShieldFrameTime = currentTime;
        }

        // Update health regeneration if active
        if (isRegeneratingHealth) {
            healthRegenTimer -= deltaTime;
            healthRegenTickTimer -= deltaTime;

            if (healthRegenTickTimer <= 0) {
                // Reset tick timer
                healthRegenTickTimer = HEALTH_REGEN_TICK;
            }

            if (healthRegenTimer <= 0) {
                // Regeneration complete
                isRegeneratingHealth = false;
            }
        }
    }

    void render(SDL_Renderer* renderer, float cameraX, float cameraY) {
        if (!alive) return;

        Uint32 currentTime = SDL_GetTicks();

        // Determine which texture and frame to use
        SDL_Texture* currentTexture = texture;
        int frame = 0;

        if (isShielding && shieldTexture != nullptr) {
            // Use shield texture with animation frames
            currentTexture = shieldTexture;
            frame = shieldFrame;
        } else if (isShooting) {
            // Normal shooting animation
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

        SDL_Rect srcRect = { frame * TANK_FRAME_WIDTH, 0, TANK_FRAME_WIDTH, TANK_FRAME_HEIGHT };
        SDL_Rect destRect = { static_cast<int>(x - width / 2 - cameraX),
                            static_cast<int>(y - height / 2 - cameraY),
                            width, height };

        // Điều chỉnh tâm quay - đặt ở giữa xe tăng
        SDL_Point center;
        center.x = width / 2;     // Tâm quay ở giữa chiều rộng
        center.y = height / 2;    // Tâm quay ở giữa chiều cao

        SDL_RenderCopyEx(renderer, currentTexture, &srcRect, &destRect, angle * 180.0 / M_PI, &center, SDL_FLIP_NONE);

        // Debug: Draw green outline around tank
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &destRect);

        // Debug: Draw red dot at rotation center
        int rotCenterX = destRect.x + center.x;
        int rotCenterY = destRect.y + center.y;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rotCenter = {rotCenterX - 2, rotCenterY - 2, 4, 4};
        SDL_RenderFillRect(renderer, &rotCenter);

        // Debug: Draw blue dot at tank center
        int tankCenterX = destRect.x + width / 2;
        int tankCenterY = destRect.y + height / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect tankCenter = {tankCenterX - 2, tankCenterY - 2, 4, 4};
        SDL_RenderFillRect(renderer, &tankCenter);
    }

    void renderHealthBar(SDL_Renderer* renderer, float cameraX, float cameraY) {
        if (!alive || isPlayer) return; // Don't show health bar for player

        int barWidth = width;
        int barHeight = 5;
        int offsetY = -(height / 2 + 10);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect bgRect = { static_cast<int>(x - barWidth / 2 - cameraX),
                            static_cast<int>(y + offsetY - cameraY),
                            barWidth, barHeight };
        SDL_RenderFillRect(renderer, &bgRect);
        float hpRatio = static_cast<float>(hp) / maxHp;
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect hpRect = { static_cast<int>(x - barWidth / 2 - cameraX),
                            static_cast<int>(y + offsetY - cameraY),
                            static_cast<int>(barWidth * hpRatio), barHeight };
        SDL_RenderFillRect(renderer, &hpRect);
    }

    // Get bullet spawn position (for both regular and special bullets)
    void getBulletSpawnPosition(float& outX, float& outY) {
        // Calculate position at the front of the tank barrel
        outX = x + collisionRadius * cos(angle);
        outY = y + collisionRadius * sin(angle);
    }
};

// Bullet Class
class Bullet {
public:
    float x, y, vx, vy;
    bool active;
    bool fromEnemy;
    int damage;
    bool isSpecial;

    Bullet(float x_, float y_, float vx_, float vy_, bool enemy, int damage_ = 10, bool special = false)
        : x(x_), y(y_), vx(vx_), vy(vy_),
          active(true), fromEnemy(enemy), damage(damage_), isSpecial(special) {}

    void update(float deltaTime) {
        if (!active) return;
        x += vx * deltaTime * 60.0f;
        y += vy * deltaTime * 60.0f;
        if (x < BORDER_OFFSET || x > MAP_WIDTH - BORDER_OFFSET ||
            y < BORDER_OFFSET || y > MAP_HEIGHT - BORDER_OFFSET) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer, float cameraX, float cameraY) {
        if (!active) return;

        if (isSpecial) {
            // Special bullets are larger and purple
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            SDL_Rect rect = { static_cast<int>(x - 5 - cameraX), static_cast<int>(y - 5 - cameraY), 10, 10 };
            SDL_RenderFillRect(renderer, &rect);
        } else {
            // Regular bullets
            SDL_SetRenderDrawColor(renderer, fromEnemy ? 0 : 255, 0, fromEnemy ? 255 : 0, 255);
            SDL_Rect rect = { static_cast<int>(x - 3 - cameraX), static_cast<int>(y - 3 - cameraY), 6, 6 };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

// Explosion Class
class Explosion {
public:
    float x, y;
    Uint32 startTime;
    bool active;
    SDL_Texture* texture;
    bool isSpecial;

    Explosion(float x_, float y_, bool special = false)
        : x(x_), y(y_), startTime(SDL_GetTicks()), active(true), isSpecial(special) {
        texture = ResourceManager::getTexture("assets/images/effect/explosion.png");
    }

    void render(SDL_Renderer* renderer, float cameraX, float cameraY) {
        if (!active) return;
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
};

// PowerUp Class
class PowerUp {
public:
    float x, y;
    bool active;
    PowerUpType type;
    Uint32 spawnTime;
    SDL_Texture* texture;

    PowerUp(float x_, float y_, PowerUpType type_)
        : x(x_), y(y_), active(true), type(type_), spawnTime(SDL_GetTicks()) {

        if (type_ == PowerUpType::HEALTH_PICKUP) {
            texture = ResourceManager::getTexture("assets/images/item/health_pickup.png");
        } else {
            texture = ResourceManager::getTexture("assets/images/item/powerup.png");
        }
    }

    void render(SDL_Renderer* renderer, float cameraX, float cameraY) {
        if (!active) return;

        // Make power-up pulse
        Uint32 currentTime = SDL_GetTicks();
        float scale = 1.0f + 0.1f * sin((currentTime - spawnTime) / 200.0f);

        int size = static_cast<int>(30 * scale);
        SDL_Rect destRect = {
            static_cast<int>(x - size / 2 - cameraX),
            static_cast<int>(y - size / 2 - cameraY),
            size,
            size
        };

        // Different colors for different power-up types
        if (type == PowerUpType::HEALTH_PICKUP) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for health pickup
        } else {
            SDL_SetRenderDrawColor(renderer,
                type == PowerUpType::HEALTH ? 255 : 0,
                type == PowerUpType::SPEED ? 255 : 0,
                type == PowerUpType::DAMAGE ? 255 : 0,
                255);
        }

        if (texture) {
            // Set color mod for health pickup
            if (type == PowerUpType::HEALTH_PICKUP) {
                SDL_SetTextureColorMod(texture, 255, 100, 100);
            } else {
                SDL_SetTextureColorMod(texture, 255, 255, 255);
            }

            SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        }

        else {
            // Fallback if texture not available
            SDL_RenderFillRect(renderer, &destRect);
        }
    }
};

// Particle System
class ParticleSystem {
private:
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        int maxLife;
        SDL_Color color;
        bool active;
    };

    vector<Particle> particles;

public:
    ParticleSystem(int maxParticles = 1000) {
        particles.resize(maxParticles);
        for (auto& p : particles) {
            p.active = false;
        }
    }

    void emit(float x, float y, float angle, int count, SDL_Color color, int life = 30) {
        uniform_real_distribution<float> angleDist(-0.5f, 0.5f);
        uniform_real_distribution<float> speedDist(0.5f, 2.0f);
        uniform_int_distribution<int> lifeDist(life / 2, life);

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

    // New method to emit particles in a circle (for shield effect)
    void emitCircle(float x, float y, float radius, int count, SDL_Color color, int life = 30) {
        uniform_real_distribution<float> angleDist(0, 2 * M_PI);
        uniform_real_distribution<float> radiusDist(0.8f * radius, 1.2f * radius);
        uniform_real_distribution<float> speedDist(0.2f, 0.8f);
        uniform_int_distribution<int> lifeDist(life / 2, life);

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

    void update() {
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

    void render(SDL_Renderer* renderer, float cameraX, float cameraY) {
        for (const auto& p : particles) {
            if (p.active) {
                // Fade out based on remaining life
                float alpha = static_cast<float>(p.life) / p.maxLife;
                SDL_SetRenderDrawColor(renderer,
                    p.color.r,
                    p.color.g,
                    p.color.b,
                    static_cast<Uint8>(255 * alpha));

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
};

// Kill Notification class
class KillNotification {
public:
    string text;
    Uint32 startTime;
    bool active;

    KillNotification(const string& text_)
        : text(text_), startTime(SDL_GetTicks()), active(true) {}

    bool update() {
        if (!active) return false;

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - startTime >= KILL_NOTIFICATION_DURATION) {
            active = false;
            return false;
        }
        return true;
    }

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        if (!active) return;

        Uint32 currentTime = SDL_GetTicks();
        float progress = (currentTime - startTime) / static_cast<float>(KILL_NOTIFICATION_DURATION);

        // Fade out near the end
        Uint8 alpha = progress > 0.7f ? static_cast<Uint8>(255 * (1.0f - (progress - 0.7f) / 0.3f)) : 255;

        SDL_Color textColor = {255, 0, 0, alpha};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
        if (!textSurface) return;

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
};

// Game Class
class Game {
private:
    GameState state;
    SDL_Renderer* renderer;
    TTF_Font* font;
    Mix_Music* backgroundMusic;
    Mix_Chunk* shootSound;
    Mix_Chunk* rapidFireSound;
    Mix_Chunk* explosionSound;
    Mix_Chunk* powerupSound;
    Mix_Chunk* shieldActivateSound;   // New sound for shield activation
    Mix_Chunk* shieldDeactivateSound; // New sound for shield deactivation
    Mix_Chunk* healSound;             // New sound for healing
    Mix_Chunk* buttonHoverSound;      // New: Sound for button hover
    SDL_Texture* playerTexture;
    SDL_Texture* playerShieldTexture; // New texture for shield animation
    SDL_Texture* enemyTexture;
    Tank player;
    vector<Tank> enemies;
    vector<Bullet> bullets;
    vector<Explosion> explosions;
    vector<PowerUp> powerups;
    vector<KillNotification> killNotifications;
    ParticleSystem particles;
    float cameraX, cameraY;
    bool rightMouseHeld;
    float normalCameraZoom;
    float currentCameraZoom;

    struct {
        int bulletsFired;
        int tanksDestroyed;
        int score;
        int level;
    } stats;
    struct {
        bool active;
        Uint32 startTime;
        Uint32 lastShotTime;
        Uint32 lastActivationTime;
        int cooldownRemaining;
    } rapidFire;
    struct {
        bool active;
        float intensity;
        Uint32 startTime;
        Uint32 duration;
    } screenShake;

    // Thêm cấu trúc cho thông báo hồi máu vào class Game
    struct {
        bool active;
        float timeLeft;
        int amountHealed;
    } healthRegenInfo;

    Uint32 lastSpawnTime;
    Uint32 lastPowerUpTime;
    Uint32 lastHealthPickupTime;
    Uint32 shieldStartTime;
    Uint32 lastShieldTime;
    int shieldCooldownRemaining;
    bool mouseHeld;
    bool paused;
    int difficulty;
    float gameTime;
    int circleX, circleY;

    // Menu properties
    SDL_Texture* menuBackgroundTexture;
    vector<MenuButtonInfo> menuButtons;
    MenuButton currentHoveredButton;

    // Add new struct for button animation
    struct ButtonAnimation {
        float scale;
        float targetScale;
        Uint32 lastUpdateTime;
    };
    unordered_map<MenuButton, ButtonAnimation> buttonAnimations;

    void handleSpecialAbility(float deltaTime) {
        if (!player.alive || state != GameState::PLAYING) return;

        // Check for cancel with 'A' key
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        if (player.isSpecialActive && keyState[SDL_SCANCODE_A]) {
            // Cancel special ability
            player.isSpecialActive = false;
            player.specialActivationTimer = 0;
            currentCameraZoom = normalCameraZoom;
            return;
        }

        if (rightMouseHeld && player.specialBullets > 0) {
            // Immediately activate special mode when right mouse is pressed
            if (!player.isSpecialActive) {
                player.isSpecialActive = true;
            }

            player.specialActivationTimer += deltaTime;

            // Limit the maximum activation time to 3 seconds
            player.specialActivationTimer = min(player.specialActivationTimer, 3.0f);

            // Gradually zoom out camera based on activation time
            float zoomProgress = player.specialActivationTimer / 3.0f;
            currentCameraZoom = normalCameraZoom + (SPECIAL_ZOOM_FACTOR - normalCameraZoom) * zoomProgress;

        } else if (player.isSpecialActive) {
            // Right mouse was released after being in special mode

            // Only fire if held for at least 0.5 seconds
            if (player.specialActivationTimer >= 0.5f) {
                // Get bullet spawn position (same as regular bullets)
                float bulletX, bulletY;
                player.getBulletSpawnPosition(bulletX, bulletY);

                // Fire special bullet
                Bullet bullet(bulletX, bulletY,
                            BULLET_SPEED * 1.5f * cos(player.angle),
                            BULLET_SPEED * 1.5f * sin(player.angle),
                            false,
                            1000, // Very high damage to one-shot enemies
                            true); // Mark as special bullet

                bullets.push_back(bullet);
                player.specialBullets--;

                // Play special sound
                if (shootSound) {
                    Mix_PlayChannel(-1, shootSound, 0);
                }

                // Add special muzzle flash particles
                SDL_Color specialColor = {255, 0, 255, 255}; // Purple for special
                particles.emit(
                    bulletX,
                    bulletY,
                    player.angle,
                    30, // More particles
                    specialColor
                );

                // Screen shake for special shot
                activateScreenShake(5.0f, 300);
            }

            // Reset special mode
            player.specialActivationTimer = 0;
            player.isSpecialActive = false;
            currentCameraZoom = normalCameraZoom;
        }
    }

    void renderSpecialTargetingLine() {
        if (!player.isSpecialActive) return;

        // Calculate line length based on activation time (max at 3 seconds)
        float lineProgress = min(player.specialActivationTimer / 3.0f, 1.0f);
        int lineLength = static_cast<int>(SPECIAL_LINE_LENGTH * lineProgress);

        // Get bullet spawn position (same as regular bullets)
        float bulletX, bulletY;
        player.getBulletSpawnPosition(bulletX, bulletY);

        // Convert to screen coordinates
        int startX = static_cast<int>(bulletX - cameraX);
        int startY = static_cast<int>(bulletY - cameraY);

        // Calculate end point - vertical bisector along tank's length
        int endX = startX + static_cast<int>(cos(player.angle) * lineLength);
        int endY = startY + static_cast<int>(sin(player.angle) * lineLength);

        // Draw line
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Purple for special
        SDL_RenderDrawLine(renderer, startX, startY, endX, endY);

        // Draw small circles along the line for better visibility
        int numCircles = static_cast<int>(lineLength / 50);
        for (int i = 0; i < numCircles; i++) {
            int circleX = startX + static_cast<int>(cos(player.angle) * i * 50);
            int circleY = startY + static_cast<int>(sin(player.angle) * i * 50);

            SDL_Rect circleRect = {circleX - 2, circleY - 2, 4, 4};
            SDL_RenderFillRect(renderer, &circleRect);
        }
    }

public:
    Game(SDL_Renderer* rend, TTF_Font* f)
        : renderer(rend), font(f),
          state(GameState::MENU),
          menuBackgroundTexture(nullptr),
          player(MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f, nullptr),
          cameraX(0), cameraY(0),
          lastSpawnTime(0),
          lastPowerUpTime(0),
          lastHealthPickupTime(0),
          shieldStartTime(0),
          lastShieldTime(0),
          shieldCooldownRemaining(0),
          mouseHeld(false),
          rightMouseHeld(false),
          normalCameraZoom(1.0f),
          currentCameraZoom(1.0f),
          paused(false),
          difficulty(1),
          gameTime(0.0f) {

        stats = {0, 0, 0, 1};
        rapidFire = {false, 0, 0, 0, 0};
        screenShake = {false, 0.0f, 0, 0};
        healthRegenInfo = {false, 0, 0};

        // Initialize resources
        ResourceManager::init(renderer);

        playerTexture = ResourceManager::getTexture("assets/images/tank/player/tank_shoot_spritesheet.png");
        enemyTexture = ResourceManager::getTexture("assets/images/tank/npc/enemy_tank.png");
        shootSound = ResourceManager::getSound("assets/sounds/shoot.mp3");
        rapidFireSound = ResourceManager::getSound("assets/sounds/rapid_fire.mp3");
        explosionSound = ResourceManager::getSound("assets/sounds/explosion.mp3");
        powerupSound = ResourceManager::getSound("assets/sounds/powerup.mp3");
        shieldActivateSound = ResourceManager::getSound("assets/sounds/shield_activate.mp3");
        shieldDeactivateSound = ResourceManager::getSound("assets/sounds/shield_deactivate.mp3");
        healSound = ResourceManager::getSound("assets/sounds/heal.mp3");
        backgroundMusic = ResourceManager::getMusic("assets/sounds/background_music.mp3");
        buttonHoverSound = ResourceManager::getSound("assets/sounds/button_hover.mp3");

        // Create a recolored version of the tank texture for shield effect
        // This creates a green-tinted version of the tank
        playerShieldTexture = ResourceManager::createRecoloredTexture(
            "assets/images/tank/player/tank_shoot_spritesheet.png",
            "assets/images/tank/player/tank_shield_spritesheet.png",
            renderer,
            100, 255, 100  // Green tint
        );

        player.texture = playerTexture;
        player.shieldTexture = playerShieldTexture;
        player.isPlayer = true;  // Set player flag

        // Start background music
        if (backgroundMusic) {
            Mix_PlayMusic(backgroundMusic, -1);
        }

        initializeMenu();
    }

    ~Game() {
        // Cleanup menu resources
        if (menuBackgroundTexture) {
            SDL_DestroyTexture(menuBackgroundTexture);
        }
        for (auto& button : menuButtons) {
            if (button.texture) {
                SDL_DestroyTexture(button.texture);
            }
            if (button.hoverTexture) {
                SDL_DestroyTexture(button.hoverTexture);
            }
        }
        menuButtons.clear();

        // Resources are cleaned up by ResourceManager
        ResourceManager::cleanup();
    }

    void handleEvents(SDL_Event& e, bool& quit) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (state == GameState::PLAYING) {
                state = GameState::PAUSED;
                paused = true;
            } else if (state == GameState::PAUSED) {
                state = GameState::PLAYING;
                paused = false;
            }
        }

        if (state == GameState::MENU) {
            handleMenuEvents(e, quit);
        } else if (state == GameState::PLAYING && player.alive) {
            handleGameEvents(e);
        } else if (state == GameState::PAUSED) {
            handlePauseEvents(e);
        } else if (state == GameState::GAME_OVER && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
            state = GameState::PLAYING;
            reset();
        }
    }

    void update(float deltaTime) {
        if (state != GameState::PLAYING || paused) return;

        gameTime += deltaTime;

        // Update player
        player.update(deltaTime);
        handleWallBounce(player);
        handleRapidFire(deltaTime);
        handleSpecialAbility(deltaTime);

        // Update health regeneration info
        updateHealthRegenInfo(deltaTime);

        // Update difficulty based on time
        updateDifficulty();

        // Spawn enemies
        Uint32 currentTime = SDL_GetTicks();
        int maxEnemies = ENEMY_COUNT_MAX + (difficulty - 1);
        if (currentTime - lastSpawnTime >= ENEMY_SPAWN_INTERVAL / difficulty && enemies.size() < maxEnemies) {
            spawnEnemy();
            lastSpawnTime = currentTime;
        }

        // Spawn power-ups
        if (currentTime - lastPowerUpTime >= 15000 && powerups.size() < 3) {
            spawnPowerUp();
            lastPowerUpTime = currentTime;
        }

        // Spawn health pickups
        if (currentTime - lastHealthPickupTime >= HEALTH_PICKUP_SPAWN_INTERVAL) {
            spawnHealthPickup();
            lastHealthPickupTime = currentTime;
        }

        // Update enemies
        for (auto& enemy : enemies) {
            if (enemy.alive) {
                updateEnemyBehavior(enemy, deltaTime);
                handleWallBounce(enemy);
                enemyShoot(enemy);
                enemy.update(deltaTime);
            }
        }

        // Update bullets
        for (auto& bullet : bullets) {
            bullet.update(deltaTime);
        }

        // Update particles
        particles.update();

        // Update kill notifications
        for (auto& notification : killNotifications) {
            notification.update();
        }
        killNotifications.erase(
            remove_if(killNotifications.begin(), killNotifications.end(),
            [](const KillNotification& n) { return !n.active; }),
        killNotifications.end()
        );

        // Handle collisions
        handleCollisions();

        // Clean up inactive objects
        cleanup();

        // Update camera
        updateCamera();

        // Update shield cooldown
        if (shieldCooldownRemaining > 0) {
            shieldCooldownRemaining = max(0, static_cast<int>(SHIELD_COOLDOWN - (currentTime - lastShieldTime)));
        }

        // Check if shield has expired
        if (player.isShielding && currentTime - shieldStartTime >= SHIELD_DURATION) {
            player.isShielding = false;

            // Play shield deactivation sound
            if (shieldDeactivateSound) {
                Mix_PlayChannel(-1, shieldDeactivateSound, 0);
            }

            // Add shield deactivation particles
            SDL_Color shieldColor = {0, 255, 255, 255};
            particles.emitCircle(player.x, player.y, SHIELD_RADIUS, 50, shieldColor, 60);
        }

        // Update screen shake
        if (screenShake.active) {
            if (currentTime - screenShake.startTime >= screenShake.duration) {
                screenShake.active = false;
            }
        }

        // Check game over condition
        if (!player.alive) {
            state = GameState::GAME_OVER;
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); // Darker background
        SDL_RenderClear(renderer);

        if (state == GameState::MENU) {
            renderMenu();
        } else if (state == GameState::PLAYING || state == GameState::PAUSED) {
            renderGame();
            if (state == GameState::PAUSED) {
                renderPauseMenu();
            }
        } else if (state == GameState::GAME_OVER) {
            renderGame();
            renderGameOver();
        }

        SDL_RenderPresent(renderer);
    }

private:
    void handleMenuEvents(SDL_Event& e, bool& quit) {
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.motion.x;
            int mouseY = e.motion.y;

            // Check hover state for each button
            for (auto& button : menuButtons) {
                bool wasHovered = button.isHovered;
                button.isHovered = (mouseX >= button.rect.x &&
                                  mouseX <= button.rect.x + button.rect.w &&
                                  mouseY >= button.rect.y &&
                                  mouseY <= button.rect.y + button.rect.h);

                // If button just started being hovered
                if (!wasHovered && button.isHovered) {
                    // Play hover sound
                    if (buttonHoverSound) {
                        Mix_PlayChannel(-1, buttonHoverSound, 0);
                    }
                    // Set target scale for grow animation
                    buttonAnimations[button.type].targetScale = 1.2f;
                }
                // If button just stopped being hovered
                else if (wasHovered && !button.isHovered) {
                    // Set target scale for shrink animation
                    buttonAnimations[button.type].targetScale = 1.0f;
                }
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            for (const auto& button : menuButtons) {
                if (mouseX >= button.rect.x &&
                    mouseX <= button.rect.x + button.rect.w &&
                    mouseY >= button.rect.y &&
                    mouseY <= button.rect.y + button.rect.h) {

                    switch (button.type) {
                        case MenuButton::START:
                            state = GameState::PLAYING;
                            reset();
                            break;
                        case MenuButton::STATS:
                            // TODO: Implement stats screen
                            break;
                        case MenuButton::TUTORIAL:
                            // TODO: Implement tutorial screen
                            break;
                        case MenuButton::SETTINGS:
                            // TODO: Implement settings screen
                            break;
                        case MenuButton::EXIT:
                            quit = true;
                            break;
                    }
                    break;
                }
            }
        }
    }

    void handlePauseEvents(SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Resume button
            if (mouseX >= WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2 &&
                mouseX <= WINDOW_WIDTH / 2 + BUTTON_WIDTH / 2 &&
                mouseY >= WINDOW_HEIGHT / 2 - BUTTON_HEIGHT &&
                mouseY <= WINDOW_HEIGHT / 2) {
                state = GameState::PLAYING;
                paused = false;
            }
            // Main Menu button
            else if (mouseX >= WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2 &&
                     mouseX <= WINDOW_WIDTH / 2 + BUTTON_WIDTH / 2 &&
                     mouseY >= WINDOW_HEIGHT / 2 + BUTTON_SPACING &&
                     mouseY <= WINDOW_HEIGHT / 2 + BUTTON_HEIGHT + BUTTON_SPACING) {
                state = GameState::MENU;
            }
        }
    }

    void handleGameEvents(SDL_Event& e) {
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            if (mouseX >= 0 && mouseX < WINDOW_WIDTH && mouseY >= 0 && mouseY < WINDOW_HEIGHT) {
                float worldX = mouseX + cameraX;
                float worldY = mouseY + cameraY;
                float dx = worldX - player.x;
                float dy = worldY - player.y;
                if (dx != 0 || dy != 0) {
                    player.angle = atan2(dy, dx);
                }
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                mouseHeld = true;
                if (!rapidFire.active) {
                    shoot();
                }
            } else if (e.button.button == SDL_BUTTON_RIGHT && player.specialBullets > 0) {
                rightMouseHeld = true;
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                mouseHeld = false;
            } else if (e.button.button == SDL_BUTTON_RIGHT) {
                rightMouseHeld = false;
            }
        } else if (e.type == SDL_KEYDOWN) {
            // Movement with WASD
            if (e.key.keysym.sym == SDLK_w) {
                player.vy = -player.speed * 2.0f;
            } else if (e.key.keysym.sym == SDLK_s) {
                player.vy = player.speed * 2.0f;
            } else if (e.key.keysym.sym == SDLK_a) {
                player.vx = -player.speed * 2.0f;

                // If 'A' is pressed while special ability is active, it will be handled in handleSpecialAbility

            } else if (e.key.keysym.sym == SDLK_d) {
                player.vx = player.speed * 2.0f;
            }
            // Shield ability
            else if (e.key.keysym.sym == SDLK_e && shieldCooldownRemaining == 0) {
                activateShield();
            }
            // Rapid fire ability
            else if (e.key.keysym.sym == SDLK_q && rapidFire.cooldownRemaining == 0) {
                rapidFire.active = true;
                rapidFire.startTime = SDL_GetTicks();
                rapidFire.lastShotTime = 0;
                rapidFire.lastActivationTime = rapidFire.startTime;
                rapidFire.cooldownRemaining = RAPID_FIRE_COOLDOWN;
            }
            // Use health pickup
            else if (e.key.keysym.sym == SDLK_t) {
                cout << "S key pressed. Health packs: " << player.healthPickups << endl;
                if (player.healthPickups > 0) {
                    useHealthPickup();
                    cout << "Used health pack. New HP: " << player.hp << "/" << player.maxHp << endl;
                }
            }
        } else if (e.type == SDL_KEYUP) {
            // Stop movement when keys are released
            if (e.key.keysym.sym == SDLK_w && player.vy < 0) {
                player.vy = 0;
            } else if (e.key.keysym.sym == SDLK_s && player.vy > 0) {
                player.vy = 0;
            } else if (e.key.keysym.sym == SDLK_a && player.vx < 0) {
                player.vx = 0;
            } else if (e.key.keysym.sym == SDLK_d && player.vx > 0) {
                player.vx = 0;
            }
        }
    }

    void useHealthPickup() {
        if (player.healthPickups <= 0 || player.hp >= player.maxHp) return;

        int healAmount = min(player.maxHp - player.hp, HEALTH_PICKUP_HEAL_AMOUNT);
        player.hp += healAmount;
        player.healthPickups--;

        // Activate health regeneration effect
        player.isRegeneratingHealth = true;
        player.healthRegenTimer = HEALTH_REGEN_TIME;
        player.healthRegenTickTimer = HEALTH_REGEN_TICK;

        // Set health regen info for display
        healthRegenInfo.active = true;
        healthRegenInfo.timeLeft = HEALTH_REGEN_TIME;
        healthRegenInfo.amountHealed = healAmount;

        // Play heal sound
        if (healSound) {
            Mix_PlayChannel(-1, healSound, 0);
        }

        // Add healing particles
        SDL_Color healColor = {0, 255, 0, 255};
        particles.emitCircle(player.x, player.y, 40, 30, healColor, 60);

        // Add notification
        killNotifications.push_back(KillNotification("HEALTH +" + to_string(healAmount)));

        cout << "Used health pack. Healed: " << healAmount << " New HP: " << player.hp << "/" << player.maxHp << endl;
    }

    void activateShield() {
        shieldStartTime = SDL_GetTicks();
        lastShieldTime = shieldStartTime;
        shieldCooldownRemaining = SHIELD_COOLDOWN;

        // Set shield state and initialize animation
        player.isShielding = true;
        player.shieldFrame = 0;
        player.lastShieldFrameTime = shieldStartTime;

        // Play shield activation sound
        if (shieldActivateSound) {
            Mix_PlayChannel(-1, shieldActivateSound, 0);
        }

        // Add shield activation particles
        SDL_Color shieldColor = {0, 255, 255, 255};
        particles.emitCircle(player.x, player.y, SHIELD_RADIUS, 50, shieldColor, 60);
    }

    void activateScreenShake(float intensity, Uint32 duration) {
        screenShake.active = true;
        screenShake.intensity = intensity;
        screenShake.startTime = SDL_GetTicks();
        screenShake.duration = duration;
    }

    bool isMouseInsideBorder(int mouseX, int mouseY) {
        float worldX = mouseX + cameraX;
        float worldY = mouseY + cameraY;
        return worldX >= BORDER_OFFSET && worldX <= MAP_WIDTH - BORDER_OFFSET &&
               worldY >= BORDER_OFFSET && worldY <= MAP_HEIGHT - BORDER_OFFSET;
    }

    void updateCamera() {
        // Target camera position (centered on player)
        float targetCameraX = player.x - (WINDOW_WIDTH / currentCameraZoom) / 2.0f;
        float targetCameraY = player.y - (WINDOW_HEIGHT / currentCameraZoom) / 2.0f;

        // Apply screen shake if active
        if (screenShake.active) {
            Uint32 currentTime = SDL_GetTicks();
            float progress = (currentTime - screenShake.startTime) / static_cast<float>(screenShake.duration);
            float intensity = screenShake.intensity * (1.0f - progress);

            uniform_real_distribution<float> shakeDist(-1.0f, 1.0f);
            targetCameraX += shakeDist(rng) * intensity;
            targetCameraY += shakeDist(rng) * intensity;
        }

        // Clamp camera to map bounds
        cameraX = max(0.0f, min(targetCameraX, static_cast<float>(MAP_WIDTH - WINDOW_WIDTH / currentCameraZoom)));
        cameraY = max(0.0f, min(targetCameraY, static_cast<float>(MAP_HEIGHT - WINDOW_HEIGHT / currentCameraZoom)));
    }

    void updateDifficulty() {
        // Increase difficulty based on game time
        difficulty = 1 + static_cast<int>(gameTime / 60.0f); // Increase every minute
        difficulty = min(difficulty, 5); // Cap at level 5

        // Update level in stats if changed
        if (stats.level != difficulty) {
            stats.level = difficulty;
        }
    }

    void handleWallBounce(Tank& tank) {
        if (!tank.alive) return;
        float borderLeft = static_cast<float>(BORDER_OFFSET);
        float borderRight = static_cast<float>(MAP_WIDTH - BORDER_OFFSET);
        float borderTop = static_cast<float>(BORDER_OFFSET);
        float borderBottom = static_cast<float>(MAP_HEIGHT - BORDER_OFFSET);
        bool bounced = false;

        if (tank.x <= borderLeft + tank.collisionRadius) {
            tank.vx = abs(tank.vx) * BOUNCE_FACTOR;
            tank.x = borderLeft + tank.collisionRadius;
            bounced = true;
        } else if (tank.x >= borderRight - tank.collisionRadius) {
            tank.vx = -abs(tank.vx) * BOUNCE_FACTOR;
            tank.x = borderRight - tank.collisionRadius;
            bounced = true;
        }

        if (tank.y <= borderTop + tank.collisionRadius) {
            tank.vy = abs(tank.vy) * BOUNCE_FACTOR;
            tank.y = borderTop + tank.collisionRadius;
            bounced = true;
        } else if (tank.y >= borderBottom - tank.collisionRadius) {
            tank.vy = -abs(tank.vy) * BOUNCE_FACTOR;
            tank.y = borderBottom - tank.collisionRadius;
            bounced = true;
        }

        if (bounced) {
            tank.x = max(borderLeft + tank.collisionRadius, min(tank.x, borderRight - tank.collisionRadius));
            tank.y = max(borderTop + tank.collisionRadius, min(tank.y, borderBottom - tank.collisionRadius));

            // Add bounce particles
            SDL_Color color = {200, 200, 200, 255};
            particles.emit(tank.x, tank.y, atan2(tank.vy, tank.vx) + M_PI, 10, color);

            // Add screen shake for player bounce
            if (&tank == &player) {
                activateScreenShake(3.0f, 100);
            }
        }
    }

    void shoot() {
        float bulletX, bulletY;
        player.getBulletSpawnPosition(bulletX, bulletY);

        Bullet bullet(bulletX, bulletY,
                      BULLET_SPEED * cos(player.angle),
                      BULLET_SPEED * sin(player.angle),
                      false,
                      player.damage);
        bullets.push_back(bullet);
        stats.bulletsFired++;
        player.vx -= RECOIL_FORCE * cos(player.angle);
        player.vy -= RECOIL_FORCE * sin(player.angle);

        if (shootSound) {
            Mix_PlayChannel(-1, shootSound, 0);
        }

        player.isShooting = true;
        player.currentFrame = 0;
        player.lastFrameTime = SDL_GetTicks();

        // Add muzzle flash particles
        SDL_Color color = {255, 200, 0, 255};
        particles.emit(
            bulletX,
            bulletY,
            player.angle,
            15,
            color
        );

        // Small screen shake when shooting
        activateScreenShake(1.0f, 50);

        // Award special bullets
        if (stats.bulletsFired % SPECIAL_BULLETS_REQUIRED == 0) {
            if (player.specialBullets < MAX_SPECIAL_BULLETS) {
                player.specialBullets++;

                // Notification effect for earning special bullet
                SDL_Color specialColor = {255, 0, 255, 255};
                particles.emitCircle(player.x, player.y, 50, 30, specialColor, 60);

                // Play special sound if available
                if (powerupSound) {
                    Mix_PlayChannel(-1, powerupSound, 0);
                }
            }
        }
    }

    void handleRapidFire(float deltaTime) {
        Uint32 currentTime = SDL_GetTicks();
        if (rapidFire.active && currentTime - rapidFire.startTime < RAPID_FIRE_DURATION) {
            if (mouseHeld && currentTime - rapidFire.lastShotTime >= RAPID_FIRE_INTERVAL) {
                float bulletX, bulletY;
                player.getBulletSpawnPosition(bulletX, bulletY);

                Bullet bullet(bulletX, bulletY,
                              BULLET_SPEED * cos(player.angle),
                              BULLET_SPEED * sin(player.angle),
                              false,
                              player.damage);
                bullets.push_back(bullet);
                stats.bulletsFired++;
                player.vx -= RECOIL_FORCE * cos(player.angle) * 0.5f; // Reduced recoil for rapid fire
                player.vy -= RECOIL_FORCE * sin(player.angle) * 0.5f;

                if (rapidFireSound) {
                    Mix_PlayChannel(-1, rapidFireSound, 0);
                }

                player.isShooting = true;
                player.currentFrame = 0;
                player.lastFrameTime = currentTime;
                rapidFire.lastShotTime = currentTime;

                // Add muzzle flash particles
                SDL_Color color = {255, 100, 0, 255};
                particles.emit(
                    bulletX,
                    bulletY,
                    player.angle,
                    8,
                    color
                );

                // Very small screen shake for rapid fire
                activateScreenShake(0.5f, 30);
            }
        } else if (rapidFire.active) {
            rapidFire.active = false;
        }

        if (rapidFire.cooldownRemaining > 0) {
            rapidFire.cooldownRemaining = max(0, static_cast<int>(RAPID_FIRE_COOLDOWN - (currentTime - rapidFire.lastActivationTime)));
        }
    }

    void spawnEnemy() {
        float viewLeft = cameraX;
        float viewRight = cameraX + WINDOW_WIDTH;
        float viewTop = cameraY;
        float viewBottom = cameraY + WINDOW_HEIGHT;
        vector<int> validEdges;

        if (viewTop > 0) validEdges.push_back(0);
        if (viewRight < MAP_WIDTH) validEdges.push_back(1);
        if (viewBottom < MAP_HEIGHT) validEdges.push_back(2);
        if (viewLeft > 0) validEdges.push_back(3);

        if (validEdges.empty()) validEdges = {0, 1, 2, 3};

        array<float, 4> distances = {
            abs(player.y - BORDER_OFFSET),
            abs(player.x - (MAP_WIDTH - BORDER_OFFSET)),
            abs(player.y - (MAP_HEIGHT - BORDER_OFFSET)),
            abs(player.x - BORDER_OFFSET)
        };

        float maxDistance = -1.0f;
        int chosenEdge = validEdges[0];
        for (int edge : validEdges) {
            if (distances[edge] > maxDistance) {
                maxDistance = distances[edge];
                chosenEdge = edge;
            }
        }

        float x, y;
        switch (chosenEdge) {
            case 0: x = BORDER_OFFSET + rand() % (MAP_WIDTH - 2 * BORDER_OFFSET); y = BORDER_OFFSET - 30.0f; break;
            case 1: x = MAP_WIDTH - BORDER_OFFSET + 30.0f; y = BORDER_OFFSET + rand() % (MAP_HEIGHT - 2 * BORDER_OFFSET); break;
            case 2: x = BORDER_OFFSET + rand() % (MAP_WIDTH - 2 * BORDER_OFFSET); y = MAP_HEIGHT - BORDER_OFFSET + 30.0f; break;
            case 3: x = BORDER_OFFSET - 30.0f; y = BORDER_OFFSET + rand() % (MAP_HEIGHT - 2 * BORDER_OFFSET); break;
        }

        // Randomly choose enemy type based on difficulty
        uniform_int_distribution<int> typeDist(0, 99);
        int roll = typeDist(rng);
        EnemyType enemyType;

        if (difficulty >= 3 && roll < 20) {
            enemyType = EnemyType::HEAVY;
        } else if (difficulty >= 2 && roll < 40) {
            enemyType = EnemyType::FAST;
        } else {
            enemyType = EnemyType::BASIC;
        }

        Tank enemy(x, y, enemyTexture, enemyType);
        enemies.push_back(enemy);
    }

    void spawnPowerUp() {
        // Random position within the map bounds
        uniform_int_distribution<int> xDist(BORDER_OFFSET + 50, MAP_WIDTH - BORDER_OFFSET - 50);
        uniform_int_distribution<int> yDist(BORDER_OFFSET + 50, MAP_HEIGHT - BORDER_OFFSET - 50);
        float x = static_cast<float>(xDist(rng));
        float y = static_cast<float>(yDist(rng));

        // Random power-up type
        uniform_int_distribution<int> typeDist(0, 4);
        PowerUpType type = static_cast<PowerUpType>(typeDist(rng));

        PowerUp powerup(x, y, type);
        powerups.push_back(powerup);

        lastPowerUpTime = SDL_GetTicks();
    }

    void spawnHealthPickup() {
        // Random position within the map bounds
        uniform_int_distribution<int> xDist(BORDER_OFFSET + 50, MAP_WIDTH - BORDER_OFFSET - 50);
        uniform_int_distribution<int> yDist(BORDER_OFFSET + 50, MAP_HEIGHT - BORDER_OFFSET - 50);
        float x = static_cast<float>(xDist(rng));
        float y = static_cast<float>(yDist(rng));

        PowerUp healthPickup(x, y, PowerUpType::HEALTH_PICKUP);
        powerups.push_back(healthPickup);

        lastHealthPickupTime = SDL_GetTicks();
    }

    void updateEnemyBehavior(Tank& enemy, float deltaTime) {
        if (!enemy.alive || !player.alive) return;

        // Add smooth movement
        float smoothingFactor = 0.05f; // Lower value = smoother movement

        if (enemy.type == EnemyType::FAST) {
            float dx = player.x - enemy.x;
            float dy = player.y - enemy.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance > 300) {
                // Smoother movement when approaching player
                float targetVx = dx / distance * enemy.speed;
                float targetVy = dy / distance * enemy.speed;

                // Smooth velocity instead of abrupt changes
                enemy.vx += (targetVx - enemy.vx) * smoothingFactor;
                enemy.vy += (targetVy - enemy.vy) * smoothingFactor;
            } else {
                // Smoother circular movement
                float circleAngle = atan2(dy, dx) + M_PI / 2;
                float targetVx = cos(circleAngle) * enemy.speed;
                float targetVy = sin(circleAngle) * enemy.speed;

                enemy.vx += (targetVx - enemy.vx) * smoothingFactor;
                enemy.vy += (targetVy - enemy.vy) * smoothingFactor;
            }

            // Smoother rotation
            float targetAngle = atan2(dy, dx);
            float angleDiff = targetAngle - enemy.angle;

            // Normalize angle to [-PI, PI]
            while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
            while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

            enemy.angle += angleDiff * smoothingFactor * 2.0f;
        } else if (enemy.type == EnemyType::HEAVY) {
            float dx = player.x - enemy.x;
            float dy = player.y - enemy.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance > 0) {
                dx /= distance;
                dy /= distance;

                // Smooth velocity
                float targetVx = dx * enemy.speed;
                float targetVy = dy * enemy.speed;

                enemy.vx += (targetVx - enemy.vx) * smoothingFactor * 0.5f; // Heavy tank moves slower
                enemy.vy += (targetVy - enemy.vy) * smoothingFactor * 0.5f;

                // Smoother rotation
                float targetAngle = atan2(dy, dx);
                float angleDiff = targetAngle - enemy.angle;

                // Normalize angle
                while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
                while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

                enemy.angle += angleDiff * smoothingFactor;
            }
        } else {
            // Basic tank
            float dx = player.x - enemy.x;
            float dy = player.y - enemy.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance > 0) {
                dx /= distance;
                dy /= distance;

                // Reduce randomness to avoid jitter
                static int frameCount = 0;
                frameCount++;

                if (frameCount % 30 == 0) { // Only change direction occasionally
                    uniform_real_distribution<float> randDist(-0.1f, 0.1f);
                    dx += randDist(rng);
                    dy += randDist(rng);

                    // Normalize again
                    float newDist = sqrt(dx * dx + dy * dy);
                    if (newDist > 0) {
                        dx /= newDist;
                        dy /= newDist;
                    }
                }

                // Smooth velocity
                float targetVx = dx * enemy.speed;
                float targetVy = dy * enemy.speed;

                enemy.vx += (targetVx - enemy.vx) * smoothingFactor;
                enemy.vy += (targetVy - enemy.vy) * smoothingFactor;

                // Smoother rotation
                float targetAngle = atan2(dy, dx);
                float angleDiff = targetAngle - enemy.angle;

                // Normalize angle
                while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
                while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

                enemy.angle += angleDiff * smoothingFactor;
            }
        }
    }

    void enemyShoot(Tank& enemy) {
        if (!enemy.alive || !player.alive) return;

        Uint32 currentTime = SDL_GetTicks();
        Uint32 shootDelay = ENEMY_SHOOT_DELAY;

        // Adjust shoot delay based on enemy type
        if (enemy.type == EnemyType::FAST) {
            shootDelay = ENEMY_SHOOT_DELAY - 1000;
        } else if (enemy.type == EnemyType::HEAVY) {
            shootDelay = ENEMY_SHOOT_DELAY + 1000;
        }

        if (currentTime - enemy.lastShotTime >= shootDelay) {
            float dx = player.x - enemy.x;
            float dy = player.y - enemy.y;
            float length = sqrt(dx * dx + dy * dy);

            if (length != 0) {
                dx /= length;
                dy /= length;
            }

            Bullet bullet(enemy.x + enemy.collisionRadius * dx,
                          enemy.y + enemy.collisionRadius * dy,
                          BULLET_SPEED * dx,
                          BULLET_SPEED * dy,
                          true,
                          enemy.damage);
            bullets.push_back(bullet);
            enemy.lastShotTime = currentTime;

            // Add muzzle flash particles
            SDL_Color color = {255, 0, 0, 255};
            particles.emit(
                enemy.x + enemy.collisionRadius * dx,
                enemy.y + enemy.collisionRadius * dy,
                atan2(dy, dx),
                10,
                color
            );
        }
    }

    void handleCollisions() {
        // Bullet collisions
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;

            if (!bullet.fromEnemy) {
                // Player bullets hitting enemies
                for (auto& enemy : enemies) {
                    if (!enemy.alive) continue;
                    if (sqrt(pow(bullet.x - enemy.x, 2) + pow(bullet.y - enemy.y, 2)) < enemy.collisionRadius) {
                        bullet.active = false;
                        enemy.hp -= bullet.damage;

                        // Hit particles
                        SDL_Color hitColor = {255, 200, 0, 255};
                        particles.emit(bullet.x, bullet.y, atan2(bullet.vy, bullet.vx) + M_PI, 15, hitColor);

                        if (enemy.hp <= 0) {
                            enemy.alive = false;
                            Explosion explosion(enemy.x, enemy.y, bullet.isSpecial);
                            explosions.push_back(explosion);

                            if (explosionSound) {
                                Mix_PlayChannel(-1, explosionSound, 0);
                            }

                            // Screen shake on enemy destruction
                            activateScreenShake(bullet.isSpecial ? 6.0f : 4.0f, bullet.isSpecial ? 300 : 200);

                            stats.tanksDestroyed++;
                            stats.score += enemy.type == EnemyType::BASIC ? 100 :
                                          (enemy.type == EnemyType::FAST ? 150 : 200);

                            // Add kill notification
                            killNotifications.push_back(KillNotification("KILL"));

                            // Increase max health for every 5 enemies killed
                            if (stats.tanksDestroyed % 5 == 0) {
                                player.maxHp += 50;

                                // Add notification for max health increase
                                killNotifications.push_back(KillNotification("MAX HP +50"));

                                // Visual effect for max HP increase
                                SDL_Color hpColor = {0, 255, 0, 255};
                                particles.emitCircle(player.x, player.y, 60, 40, hpColor, 80);
                            }

                            // Chance to drop power-up
                            uniform_int_distribution<int> dropDist(0, 100);
                            if (dropDist(rng) < 30) { // 30% chance
                                uniform_int_distribution<int> typeDist(0, 4);
                                PowerUpType type = static_cast<PowerUpType>(typeDist(rng));
                                PowerUp powerup(enemy.x, enemy.y, type);
                                powerups.push_back(powerup);
                            }
                        }
                        break;
                    }
                }
            } else if (player.alive) {
                // Enemy bullets hitting player
                if (sqrt(pow(bullet.x - player.x, 2) + pow(bullet.y - player.y, 2)) < player.collisionRadius) {
                    if (player.isShielding) {
                        // Shield deflects bullet
                        bullet.active = false;

                        // Shield particles
                        SDL_Color shieldColor = {0, 255, 255, 255};
                        particles.emit(bullet.x, bullet.y, atan2(bullet.vy, bullet.vx) + M_PI, 20, shieldColor);
                    } else {
                        // Player is not shielded
                        bullet.active = false;
                        player.hp -= bullet.damage;

                        // Hit particles
                        SDL_Color hitColor = {255, 0, 0, 255};
                        particles.emit(bullet.x, bullet.y, atan2(bullet.vy, bullet.vx) + M_PI, 15, hitColor);

                        // Screen shake when player is hit
                        activateScreenShake(3.0f, 150);

                        if (player.hp <= 0) {
                            player.alive = false;
                            Explosion explosion(player.x, player.y);
                            explosions.push_back(explosion);

                            if (explosionSound) {
                                Mix_PlayChannel(-1, explosionSound, 0);
                            }

                            // Major screen shake on player death
                            activateScreenShake(10.0f, 500);
                        }
                    }
                }
            }
        }

        // Tank-tank collisions
        // Player-enemy collisions
        for (auto& enemy : enemies) {
            if (player.alive && enemy.alive) {
                float dx = enemy.x - player.x;
                float dy = enemy.y - player.y;
                float distance = sqrt(dx * dx + dy * dy);
                float minDistance = player.collisionRadius + enemy.collisionRadius;

                if (distance < minDistance) {
                    if (distance < 0.1f) {
                        // Avoid division by very small numbers
                        dx = 1.0f;
                        dy = 0.0f;
                        distance = 1.0f;
                    } else {
                        dx /= distance;
                        dy /= distance;
                    }

                    // Reduce push force to avoid jitter
                    float pushForce = TANK_COLLISION_FORCE * 0.7f;
                    player.vx -= dx * pushForce;
                    player.vy -= dy * pushForce;
                    enemy.vx += dx * pushForce;
                    enemy.vy += dy * pushForce;

                    float overlap = (minDistance - distance) / 2.0f;
                    if (overlap > 0) {
                        // Limit maximum push to avoid jitter
                        float maxPush = 2.0f;
                        overlap = min(overlap, maxPush);

                        player.x -= dx * overlap;
                        player.y -= dy * overlap;
                        enemy.x += dx * overlap;
                        enemy.y += dy * overlap;
                    }

                    // Collision particles
                    SDL_Color collisionColor = {150, 150, 150, 255};
                    particles.emit(player.x + dx * player.collisionRadius,
                                  player.y + dy * player.collisionRadius,
                                  atan2(dy, dx),
                                  10,
                                  collisionColor);

                    // Small screen shake on collision
                    activateScreenShake(2.0f, 100);
                }
            }
        }

        // Enemy-enemy collisions
        for (size_t i = 0; i < enemies.size(); ++i) {
            for (size_t j = i + 1; j < enemies.size(); ++j) {
                if (enemies[i].alive && enemies[j].alive) {
                    float dx = enemies[j].x - enemies[i].x;
                    float dy = enemies[j].y - enemies[i].y;
                    float distance = sqrt(dx * dx + dy * dy);
                    float minDistance = enemies[i].collisionRadius + enemies[j].collisionRadius;

                    if (distance < minDistance) {
                        if (distance < 0.1f) {
                            // Avoid division by very small numbers
                            dx = 1.0f;
                            dy = 0.0f;
                            distance = 1.0f;
                        } else {
                            dx /= distance;
                            dy /= distance;
                        }

                        // Reduce push force to avoid jitter
                        float pushForce = TANK_COLLISION_FORCE * 0.5f;
                        enemies[i].vx -= dx * pushForce;
                        enemies[i].vy -= dy * pushForce;
                        enemies[j].vx += dx * pushForce;
                        enemies[j].vy += dy * pushForce;

                        float overlap = (minDistance - distance) / 2.0f;
                        if (overlap > 0) {
                            // Limit maximum push to avoid jitter
                            float maxPush = 2.0f;
                            overlap = min(overlap, maxPush);

                            enemies[i].x -= dx * overlap;
                            enemies[i].y -= dy * overlap;
                            enemies[j].x += dx * overlap;
                            enemies[j].y += dy * overlap;
                        }
                    }
                }
            }
        }

        // Power-up collisions
        for (auto& powerup : powerups) {
            if (powerup.active && player.alive) {
                if (sqrt(pow(powerup.x - player.x, 2) + pow(powerup.y - player.y, 2)) < player.collisionRadius + 15) {
                    if (powerup.type == PowerUpType::HEALTH_PICKUP) {
                        player.healthPickups++;
                        powerup.active = false;

                        // Play pickup sound
                        if (powerupSound) {
                            Mix_PlayChannel(-1, powerupSound, 0);
                        }

                        // Health pickup particles
                        SDL_Color healthColor = {255, 0, 0, 255};
                        particles.emit(powerup.x, powerup.y, 0, 20, healthColor, 40);

                        // Add notification
                        killNotifications.push_back(KillNotification("HEALTH PACK +1"));
                    } else {
                        applyPowerUp(powerup);
                        powerup.active = false;

                        if (powerupSound) {
                            Mix_PlayChannel(-1, powerupSound, 0);
                        }

                        // Power-up particles
                        SDL_Color powerupColor = {0, 255, 0, 255};
                        particles.emit(powerup.x, powerup.y, 0, 30, powerupColor, 60);
                    }
                }
            }
        }
    }

    void applyPowerUp(const PowerUp& powerup) {
        switch (powerup.type) {
            case PowerUpType::HEALTH:
                player.hp = min(player.maxHp, player.hp + 30);
                break;

            case PowerUpType::SPEED:
                player.speed *= 1.2f;
                break;

            case PowerUpType::DAMAGE:
                player.damage += 5;
                break;

            case PowerUpType::SHIELD:
                activateShield();
                break;

            case PowerUpType::RAPID_FIRE:
                rapidFire.active = true;
                rapidFire.startTime = SDL_GetTicks();
                rapidFire.lastShotTime = 0;
                rapidFire.lastActivationTime = rapidFire.startTime;
                rapidFire.cooldownRemaining = 0; // Reset cooldown
                break;

            default:
                break;
        }
    }

    void cleanup() {
        // Remove inactive bullets
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return !b.active; }), bullets.end());

        // Remove dead enemies
        enemies.erase(remove_if(enemies.begin(), enemies.end(),
            [](const Tank& e) { return !e.alive; }), enemies.end());

        // Remove finished explosions
        explosions.erase(remove_if(explosions.begin(), explosions.end(),
            [](const Explosion& e) { return !e.active; }), explosions.end());

        // Remove collected power-ups
        powerups.erase(remove_if(powerups.begin(), powerups.end(),
            [](const PowerUp& p) { return !p.active; }), powerups.end());
    }

    void reset() {
        player = Tank(MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f, playerTexture);
        player.shieldTexture = playerShieldTexture;
        player.isPlayer = true;  // Set player flag
        player.specialBullets = 0;
        player.healthPickups = 0;
        enemies.clear();
        bullets.clear();
        explosions.clear();
        powerups.clear();
        killNotifications.clear();
        stats = {0, 0, 0, 1};
        rapidFire = {false, 0, 0, 0, 0};
        screenShake = {false, 0.0f, 0, 0};
        lastSpawnTime = SDL_GetTicks();
        lastPowerUpTime = SDL_GetTicks();
        lastHealthPickupTime = SDL_GetTicks();
        shieldStartTime = 0;
        shieldCooldownRemaining = 0;
        difficulty = 1;
        gameTime = 0.0f;
        normalCameraZoom = 1.0f;
        currentCameraZoom = 1.0f;
    }

    void renderGame() {
        // Render game border
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect borderRect = { BORDER_OFFSET - static_cast<int>(cameraX),
                            BORDER_OFFSET - static_cast<int>(cameraY),
                            MAP_WIDTH - 2 * BORDER_OFFSET,
                            MAP_HEIGHT - 2 * BORDER_OFFSET };
        SDL_RenderDrawRect(renderer, &borderRect);

        // Render bullets
        for (auto& bullet : bullets) {
            bullet.render(renderer, cameraX, cameraY);
        }

        // Render power-ups
        for (auto& powerup : powerups) {
            powerup.render(renderer, cameraX, cameraY);
        }

        // Render particles
        particles.render(renderer, cameraX, cameraY);

        // Render special targeting line if active
        if (player.isSpecialActive) {
            renderSpecialTargetingLine();
        }

        // Render player
        player.render(renderer, cameraX, cameraY);

        // Render enemies
        for (auto& enemy : enemies) {
            enemy.render(renderer, cameraX, cameraY);
            enemy.renderHealthBar(renderer, cameraX, cameraY);
        }

        // Render explosions
        for (auto& explosion : explosions) {
            explosion.render(renderer, cameraX, cameraY);
        }

        // Render kill notifications
        for (auto& notification : killNotifications) {
            notification.render(renderer, font);
        }

        // Render HUD
        renderStats();
        renderMinimap();

        // Render player HP bar at bottom center of screen
        if (player.alive) {
            int barWidth = 200;
            int barHeight = 20;
            int barX = WINDOW_WIDTH / 2 - barWidth / 2;
            int barY = WINDOW_HEIGHT - barHeight - 10;

            // Background of health bar
            SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
            SDL_Rect bgRect = { barX, barY, barWidth, barHeight };
            SDL_RenderFillRect(renderer, &bgRect);

            // Health bar fill
            float hpRatio = static_cast<float>(player.hp) / player.maxHp;
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect hpRect = { barX, barY, static_cast<int>(barWidth * hpRatio), barHeight };
            SDL_RenderFillRect(renderer, &hpRect);

            // HP text
            string hpText = to_string(player.hp) + "/" + to_string(player.maxHp) + " HP";
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* hpSurface = TTF_RenderText_Solid(font, hpText.c_str(), textColor);
            if (hpSurface) {
                SDL_Texture* hpTexture = SDL_CreateTextureFromSurface(renderer, hpSurface);
                SDL_Rect textRect = { barX + barWidth + 10, barY + (barHeight - hpSurface->h) / 2,
                                    hpSurface->w, hpSurface->h };
                SDL_RenderCopy(renderer, hpTexture, nullptr, &textRect);
                SDL_FreeSurface(hpSurface);
                SDL_DestroyTexture(hpTexture);
            }
        }

        renderCooldowns();

        // Render health regeneration info if active
        if (healthRegenInfo.active) {
            renderHealthRegenInfo();
        }
    }

    void updateHealthRegenInfo(float deltaTime) {
        if (healthRegenInfo.active) {
            healthRegenInfo.timeLeft -= deltaTime;

            if (healthRegenInfo.timeLeft <= 0) {
                healthRegenInfo.active = false;
            }
        }
    }

    void renderHealthRegenInfo() {
        if (!healthRegenInfo.active) return;

        // Hiển thị thông báo hồi máu ở giữa màn hình
        string regenText = "HEALING: +" + to_string(healthRegenInfo.amountHealed) + " HP";

        // Hiển thị thời gian còn lại
        string timeText = "Time: " + to_string(static_cast<int>(healthRegenInfo.timeLeft * 10) / 10.0f) + "s";

        // Vẽ nền cho thông báo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect bgRect = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &bgRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // Vẽ viền
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &bgRect);

        // Hiển thị text
        SDL_Color textColor = {0, 255, 0, 255};
        SDL_Surface* regenSurface = TTF_RenderText_Solid(font, regenText.c_str(), textColor);
        if (regenSurface) {
            SDL_Texture* regenTexture = SDL_CreateTextureFromSurface(renderer, regenSurface);
            SDL_Rect regenRect = {
                WINDOW_WIDTH / 2 - regenSurface->w / 2,
                WINDOW_HEIGHT / 2 - 30,
                regenSurface->w,
                regenSurface->h
            };
            SDL_RenderCopy(renderer, regenTexture, nullptr, &regenRect);
            SDL_FreeSurface(regenSurface);
            SDL_DestroyTexture(regenTexture);
        }

        SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor);
        if (timeSurface) {
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            SDL_Rect timeRect = {
                WINDOW_WIDTH / 2 - timeSurface->w / 2,
                WINDOW_HEIGHT / 2 + 10,
                timeSurface->w,
                timeSurface->h
            };
            SDL_RenderCopy(renderer, timeTexture, nullptr, &timeRect);
            SDL_FreeSurface(timeSurface);
            SDL_DestroyTexture(timeTexture);
        }

        // Hiển thị thanh tiến trình
        float progress = healthRegenInfo.timeLeft / HEALTH_REGEN_TIME;
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect progressRect = {
            WINDOW_WIDTH / 2 - 100,
            WINDOW_HEIGHT / 2 + 40,
            static_cast<int>(200 * progress),
            10
        };
        SDL_RenderFillRect(renderer, &progressRect);

        // Vẽ viền cho thanh tiến trình
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect progressBorder = {
            WINDOW_WIDTH / 2 - 100,
            WINDOW_HEIGHT / 2 + 40,
            200,
            10
        };
        SDL_RenderDrawRect(renderer, &progressBorder);
    }


    void renderCooldowns() {
        // Shield cooldown bar
        if (shieldCooldownRemaining > 0) {
            float percentage = 1.0f - (shieldCooldownRemaining / static_cast<float>(SHIELD_COOLDOWN));
            renderCooldownBar(10, 190, 200, 10, percentage, {0, 255, 255, 255});

            string cooldownText = "Shield: " + to_string(shieldCooldownRemaining / 1000) + "s";
            renderText(cooldownText, 10, 205, {255, 255, 255, 255});
        } else if (!player.isShielding) {
            renderText("Shield ready (Press E)", 10, 190, {0, 255, 255, 255});
        } else {
            // Shield active - show duration
            Uint32 currentTime = SDL_GetTicks();
            float percentage = 1.0f - ((currentTime - shieldStartTime) / static_cast<float>(SHIELD_DURATION));
            renderCooldownBar(10, 190, 200, 10, percentage, {0, 255, 255, 255});

            string durationText = "Shield: " + to_string((SHIELD_DURATION - (currentTime - shieldStartTime)) / 1000) + "s";
            renderText(durationText, 10, 205, {255, 255, 255, 255});
        }

        // Rapid fire cooldown bar
        if (rapidFire.cooldownRemaining > 0) {
            float percentage = 1.0f - (rapidFire.cooldownRemaining / static_cast<float>(RAPID_FIRE_COOLDOWN));
            renderCooldownBar(10, 230, 200, 10, percentage, {255, 100, 0, 255});

            string cooldownText = "Rapid Fire: " + to_string(rapidFire.cooldownRemaining / 1000) + "s";
            renderText(cooldownText, 10, 245, {255, 255, 255, 255});
        } else if (!rapidFire.active) {
            renderText("Rapid Fire ready (Press Q)", 10, 230, {255, 100, 0, 255});
        } else {
            // Rapid fire active - show duration
            Uint32 currentTime = SDL_GetTicks();
            float percentage = 1.0f - ((currentTime - rapidFire.startTime) / static_cast<float>(RAPID_FIRE_DURATION));
            renderCooldownBar(10, 230, 200, 10, percentage, {255, 100, 0, 255});

            string durationText = "Rapid Fire: " + to_string((RAPID_FIRE_DURATION - (currentTime - rapidFire.startTime)) / 1000) + "s";
            renderText(durationText, 10, 245, {255, 255, 255, 255});
        }

        // Health pickups
        if (player.healthPickups > 0) {
            string healthText = "Health Packs: " + to_string(player.healthPickups) + " (Press S to use)";
            renderText(healthText, 10, 270, {255, 0, 0, 255});
        }
    }

    void renderCooldownBar(int x, int y, int width, int height, float percentage, SDL_Color color) {
        // Background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect bgRect = {x, y, width, height};
        SDL_RenderFillRect(renderer, &bgRect);

        // Fill
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect fillRect = {x, y, static_cast<int>(width * percentage), height};
        SDL_RenderFillRect(renderer, &fillRect);
    }

    void renderText(const string& text, int x, int y, SDL_Color color) {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!textSurface) return;

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    void renderMenu() {
        // Render background
        if (menuBackgroundTexture) {
            SDL_RenderCopy(renderer, menuBackgroundTexture, nullptr, nullptr);
        } else {
            SDL_SetRenderDrawColor(renderer, 20, 20, 50, 255);
            SDL_RenderClear(renderer);
        }

        // Render game title
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "   ", textColor); //Thêm tên ở giữa màn hình
        if (titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            SDL_Rect titleRect = {
                WINDOW_WIDTH / 2 - titleSurface->w / 2,
                50,
                titleSurface->w,
                titleSurface->h
            };
            SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
            SDL_FreeSurface(titleSurface);
            SDL_DestroyTexture(titleTexture);
        }

        // Update button animations
        updateButtonAnimations();

        // Render buttons with scale animation
        for (const auto& button : menuButtons) {
            SDL_Texture* currentTexture = button.isHovered ? button.hoverTexture : button.texture;
            if (currentTexture) {
                float scale = buttonAnimations[button.type].scale;

                // Calculate scaled dimensions
                int scaledWidth = static_cast<int>(button.rect.w * scale);
                int scaledHeight = static_cast<int>(button.rect.h * scale);

                // Center the scaled button
                int offsetX = (scaledWidth - button.rect.w) / 2;
                int offsetY = (scaledHeight - button.rect.h) / 2;

                SDL_Rect scaledRect = {
                    button.rect.x - offsetX,
                    button.rect.y - offsetY,
                    scaledWidth,
                    scaledHeight
                };

                SDL_RenderCopy(renderer, currentTexture, nullptr, &scaledRect);
            } else {
                // Fallback button rendering if texture not loaded
                SDL_SetRenderDrawColor(renderer, button.isHovered ? 100 : 70, 100, 200, 255);
                SDL_RenderFillRect(renderer, &button.rect);

                // Render button text
                const char* buttonText;
                switch (button.type) {
                    case MenuButton::START: buttonText = "Start Game"; break;
                    case MenuButton::STATS: buttonText = "Statistics"; break;
                    case MenuButton::TUTORIAL: buttonText = "Tutorial"; break;
                    case MenuButton::SETTINGS: buttonText = "Settings"; break;
                    case MenuButton::EXIT: buttonText = "Exit"; break;
                    default: buttonText = ""; break;
                }

                SDL_Surface* textSurface = TTF_RenderText_Solid(font, buttonText, textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    SDL_Rect textRect = {
                        button.rect.x + (button.rect.w - textSurface->w) / 2,
                        button.rect.y + (button.rect.h - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h
                    };
                    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                    SDL_FreeSurface(textSurface);
                    SDL_DestroyTexture(textTexture);
                }
            }
        }
    }

    void renderPauseMenu() {
        // Semi-transparent overlay
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* pauseSurface = TTF_RenderText_Solid(font, "Game Paused", textColor);
        if (!pauseSurface) return;

        SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface);
        SDL_Rect pauseRect = { WINDOW_WIDTH / 2 - pauseSurface->w / 2, WINDOW_HEIGHT / 4, pauseSurface->w, pauseSurface->h };
        SDL_RenderCopy(renderer, pauseTexture, nullptr, &pauseRect);
        SDL_FreeSurface(pauseSurface);
        SDL_DestroyTexture(pauseTexture);

        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_Rect resumeButton = { WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2, WINDOW_HEIGHT / 2 - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT };
        SDL_Rect menuButton = { WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2, WINDOW_HEIGHT / 2 + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT };
        SDL_RenderFillRect(renderer, &resumeButton);
        SDL_RenderFillRect(renderer, &menuButton);

        SDL_Surface* resumeSurface = TTF_RenderText_Solid(font, "Resume", textColor);
        SDL_Surface* menuSurface = TTF_RenderText_Solid(font, "Main Menu", textColor);
        if (!resumeSurface || !menuSurface) return;

        SDL_Texture* resumeTexture = SDL_CreateTextureFromSurface(renderer, resumeSurface);
        SDL_Texture* menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
        SDL_Rect resumeTextRect = { WINDOW_WIDTH / 2 - resumeSurface->w / 2, WINDOW_HEIGHT / 2 - BUTTON_HEIGHT / 2 - resumeSurface->h / 2, resumeSurface->w, resumeSurface->h };
        SDL_Rect menuTextRect = { WINDOW_WIDTH / 2 - menuSurface->w / 2, WINDOW_HEIGHT / 2 + BUTTON_SPACING + BUTTON_HEIGHT / 2 - menuSurface->h / 2, menuSurface->w, menuSurface->h };
        SDL_RenderCopy(renderer, resumeTexture, nullptr, &resumeTextRect);
        SDL_RenderCopy(renderer, menuTexture, nullptr, &menuTextRect);
        SDL_FreeSurface(resumeSurface);
        SDL_FreeSurface(menuSurface);
        SDL_DestroyTexture(resumeTexture);
        SDL_DestroyTexture(menuTexture);
    }

    void renderGameOver() {
        // Semi-transparent overlay
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color textColor = {255, 0, 0, 255};
        SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, "Game Over!", textColor);
        if (!gameOverSurface) return;

        SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
        SDL_Rect gameOverRect = { WINDOW_WIDTH / 2 - gameOverSurface->w / 2,
                                 WINDOW_HEIGHT / 2 - gameOverSurface->h - 20,
                                 gameOverSurface->w,
                                 gameOverSurface->h };
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
        SDL_FreeSurface(gameOverSurface);
        SDL_DestroyTexture(gameOverTexture);

        // Display score
        string scoreText = "Final Score: " + to_string(stats.score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (scoreSurface) {
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = { WINDOW_WIDTH / 2 - scoreSurface->w / 2,
                                  WINDOW_HEIGHT / 2,
                                  scoreSurface->w,
                                  scoreSurface->h };
            SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
            SDL_FreeSurface(scoreSurface);
            SDL_DestroyTexture(scoreTexture);
        }

        // Restart prompt
        SDL_Color whiteColor = {255, 255, 255, 255};
        SDL_Surface* restartSurface = TTF_RenderText_Solid(font, "Press R to Restart", whiteColor);
        if (restartSurface) {
            SDL_Texture* restartTexture = SDL_CreateTextureFromSurface(renderer, restartSurface);
            SDL_Rect restartRect = { WINDOW_WIDTH / 2 - restartSurface->w / 2,
                                    WINDOW_HEIGHT / 2 + 50,
                                    restartSurface->w,
                                    restartSurface->h };
            SDL_RenderCopy(renderer, restartTexture, nullptr, &restartRect);
            SDL_FreeSurface(restartSurface);
            SDL_DestroyTexture(restartTexture);
        }
    }

    void renderStats() {
        SDL_Color textColor = {255, 255, 255, 255};

        // Score
        string scoreText = "Score: " + to_string(stats.score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (scoreSurface) {
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
            SDL_FreeSurface(scoreSurface);
            SDL_DestroyTexture(scoreTexture);
        }

        // Level
        string levelText = "Level: " + to_string(stats.level);
        SDL_Surface* levelSurface = TTF_RenderText_Solid(font, levelText.c_str(), textColor);
        if (levelSurface) {
            SDL_Texture* levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);
            SDL_Rect levelRect = {10, 40, levelSurface->w, levelSurface->h};
            SDL_RenderCopy(renderer, levelTexture, nullptr, &levelRect);
            SDL_FreeSurface(levelSurface);
            SDL_DestroyTexture(levelTexture);
        }

        // Bullets fired
        string bulletsText = "Bullets fired: " + to_string(stats.bulletsFired);
        SDL_Surface* bulletsSurface = TTF_RenderText_Solid(font, bulletsText.c_str(), textColor);
        if (bulletsSurface) {
            SDL_Texture* bulletsTexture = SDL_CreateTextureFromSurface(renderer, bulletsSurface);
            SDL_Rect bulletsRect = {10, 70, bulletsSurface->w, bulletsSurface->h};
            SDL_RenderCopy(renderer, bulletsTexture, nullptr, &bulletsRect);
            SDL_FreeSurface(bulletsSurface);
            SDL_DestroyTexture(bulletsTexture);
        }

        // Tanks destroyed
        string tanksText = "Tanks destroyed: " + to_string(stats.tanksDestroyed);
        SDL_Surface* tanksSurface = TTF_RenderText_Solid(font, tanksText.c_str(), textColor);
        if (tanksSurface) {
            SDL_Texture* tanksTexture = SDL_CreateTextureFromSurface(renderer, tanksSurface);
            SDL_Rect tanksRect = {10, 100, tanksSurface->w, tanksSurface->h};
            SDL_RenderCopy(renderer, tanksTexture, nullptr, &tanksRect);
            SDL_FreeSurface(tanksSurface);
            SDL_DestroyTexture(tanksTexture);
        }

        // Special bullets count
        SDL_Color specialColor = {255, 0, 255, 255};
        string specialText = "Special bullets: " + to_string(player.specialBullets) + "/" + to_string(MAX_SPECIAL_BULLETS);
        SDL_Surface* specialSurface = TTF_RenderText_Solid(font, specialText.c_str(), specialColor);
        if (specialSurface) {
            SDL_Texture* specialTexture = SDL_CreateTextureFromSurface(renderer, specialSurface);
            SDL_Rect specialRect = {10, 130, specialSurface->w, specialSurface->h};
            SDL_RenderCopy(renderer, specialTexture, nullptr, &specialRect);
            SDL_FreeSurface(specialSurface);
            SDL_DestroyTexture(specialTexture);
        }

        // Add special ability instructions if player has special bullets
        if (player.specialBullets > 0) {
            string instructionText = "Hold right-click to charge special shot (A to cancel)";
            SDL_Surface* instructionSurface = TTF_RenderText_Solid(font, instructionText.c_str(), specialColor);
            if (instructionSurface) {
                SDL_Texture* instructionTexture = SDL_CreateTextureFromSurface(renderer, instructionSurface);
                SDL_Rect instructionRect = {10, 160, instructionSurface->w, instructionSurface->h};
                SDL_RenderCopy(renderer, instructionTexture, nullptr, &instructionRect);
                SDL_FreeSurface(instructionSurface);
                SDL_DestroyTexture(instructionTexture);
            }
        }
    }

    void renderMinimap() {
        // Minimap background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect minimapRect = {MINIMAP_X, MINIMAP_Y, MINIMAP_WIDTH, MINIMAP_HEIGHT};
        SDL_RenderFillRect(renderer, &minimapRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // Minimap border
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect borderRect = { MINIMAP_X + static_cast<int>(BORDER_OFFSET * MINIMAP_SCALE),
                                MINIMAP_Y + static_cast<int>(BORDER_OFFSET * MINIMAP_SCALE),
                                static_cast<int>((MAP_WIDTH - 2 * BORDER_OFFSET) * MINIMAP_SCALE),
                                static_cast<int>((MAP_HEIGHT - 2 * BORDER_OFFSET) * MINIMAP_SCALE) };
        SDL_RenderDrawRect(renderer, &borderRect);

        // Player on minimap
        if (player.alive) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            int px = MINIMAP_X + static_cast<int>(player.x * MINIMAP_SCALE);
            int py = MINIMAP_Y + static_cast<int>(player.y * MINIMAP_SCALE);
            SDL_Rect playerDot = {px - 2, py - 2, 4, 4};
            SDL_RenderFillRect(renderer, &playerDot);

            // Player direction
            int dx = px + static_cast<int>(10 * cos(player.angle));
            int dy = py + static_cast<int>(10 * sin(player.angle));
            SDL_RenderDrawLine(renderer, px, py, dx, dy);
        }

        // Enemies on minimap
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (const auto& enemy : enemies) {
            if (enemy.alive) {
                int ex = MINIMAP_X + static_cast<int>(enemy.x * MINIMAP_SCALE);
                int ey = MINIMAP_Y + static_cast<int>(enemy.y * MINIMAP_SCALE);
                SDL_Rect enemyDot = {ex - 2, ey - 2, 4, 4};
                SDL_RenderFillRect(renderer, &enemyDot);

                // Enemy direction
                int dx = ex + static_cast<int>(10 * cos(enemy.angle));
                int dy = ey + static_cast<int>(10 * sin(enemy.angle));
                SDL_RenderDrawLine(renderer, ex, ey, dx, dy);
            }
        }

        // Power-ups on minimap
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (const auto& powerup : powerups) {
            if (powerup.active) {
                int px = MINIMAP_X + static_cast<int>(powerup.x * MINIMAP_SCALE);
                int py = MINIMAP_Y + static_cast<int>(powerup.y * MINIMAP_SCALE);
                SDL_Rect powerupDot = {px - 1, py - 1, 3, 3};
                SDL_RenderFillRect(renderer, &powerupDot);
            }
        }

        // Camera view rectangle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect cameraRect = { MINIMAP_X + static_cast<int>(cameraX * MINIMAP_SCALE),
                                MINIMAP_Y + static_cast<int>(cameraY * MINIMAP_SCALE),
                                static_cast<int>(WINDOW_WIDTH * MINIMAP_SCALE),
                                static_cast<int>(WINDOW_HEIGHT * MINIMAP_SCALE) };
        SDL_RenderDrawRect(renderer, &cameraRect);
    }

    void initializeMenu() {
        // Load menu background
        menuBackgroundTexture = ResourceManager::loadTexture("assets/images/ui/menu_background.png", renderer);

        // Load hover sound
        buttonHoverSound = ResourceManager::loadSound("assets/sounds/button_hover.mp3");

        // Initialize menu buttons
        const int buttonWidth = 330;
        const int buttonHeight = 110;
        const int buttonSpacing = 15;
        const int startY = WINDOW_HEIGHT / 2 - (5 * buttonHeight + 4 * buttonSpacing) / 2;

        vector<string> buttonImages = {
            "assets/images/ui/button_start.png",
            "assets/images/ui/button_stats.png",
            "assets/images/ui/button_tutorial.png",
            "assets/images/ui/button_settings.png",
            "assets/images/ui/button_exit.png"
        };

        vector<string> buttonHoverImages = {
            "assets/images/ui/button_start_hover.png",
            "assets/images/ui/button_stats_hover.png",
            "assets/images/ui/button_tutorial_hover.png",
            "assets/images/ui/button_settings_hover.png",
            "assets/images/ui/button_exit_hover.png"
        };

        for (int i = 0; i < 5; i++) {
            MenuButtonInfo button;
            button.rect.w = buttonWidth;
            button.rect.h = buttonHeight;
            button.rect.x = WINDOW_WIDTH / 2 - buttonWidth / 2 + 450;
            button.rect.y = startY + i * (buttonHeight + buttonSpacing);
            button.texture = ResourceManager::loadTexture(buttonImages[i], renderer);
            button.hoverTexture = ResourceManager::loadTexture(buttonHoverImages[i], renderer);
            button.isHovered = false;
            button.type = static_cast<MenuButton>(i);
            menuButtons.push_back(button);

            // Khởi tạo animation cho nút
            buttonAnimations[button.type] = {1.0f, 1.0f, SDL_GetTicks()};
        }
    }

    void updateButtonAnimations() {
        Uint32 currentTime = SDL_GetTicks();
        float animationSpeed = 0.08f;

        for (auto& [type, anim] : buttonAnimations) {
            float deltaTime = (currentTime - anim.lastUpdateTime) / 1000.0f;

            // Smoothly interpolate current scale to target scale
            if (anim.scale != anim.targetScale) {
                if (anim.scale < anim.targetScale) {
                    anim.scale = min(anim.scale + animationSpeed * deltaTime * 60.0f, anim.targetScale);
                } else {
                    anim.scale = max(anim.scale - animationSpeed * deltaTime * 60.0f, anim.targetScale);
                }
            }

            anim.lastUpdateTime = currentTime;
        }
    }
};

int main(int argc, char* argv[]) {
    // Initialize random number generator
    srand(static_cast<unsigned>(time(nullptr)));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Bùm Bum Blasters!!!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("assets/fonts/VCR_OSD_MONO_1.001.ttf", 24);
    if (!font) {
        cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create game
    Game game(renderer, font);

    // Game loop variables
    bool quit = false;
    SDL_Event e;
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime;
    float deltaTime;

    // Game loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            game.handleEvents(e, quit);
        }

        // Calculate delta time
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Update game
        game.update(deltaTime);

        // Render game
        game.render();

        // Cap frame rate
        SDL_Delay(16);
    }

    // Clean up
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
