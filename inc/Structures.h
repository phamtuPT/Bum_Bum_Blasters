#ifndef STRUCTURES_H
#define STRUCTURES_H

// Game states
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    TUTORIAL_SCREEN,
    SETTINGS_SCREEN,
    STATS_SCREEN
};

// Enemy types
enum class EnemyType {
    BASIC,
    FAST,
    HEAVY
};

// Power-up types
enum class PowerUpType {
    HEALTH,
    SPEED,
    DAMAGE,
    SHIELD,
    RAPID_FIRE,
    HEALTH_PICKUP
};

// Menu button types
enum class MenuButton {
    START,
    STATS,
    TUTORIAL,
    SETTINGS,
    EXIT
};

// Add new struct for button animation
struct ButtonAnimation {
    float scale;
    float targetScale;
    Uint32 lastUpdateTime;
};

// Menu button structure
struct MenuButtonInfo {
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* hoverTexture;
    bool isHovered;
    MenuButton type;
};

struct Stats {
    int bulletsFired;
    int tanksDestroyed;
    int score;
    int level;
};

struct RapidFire {
    bool active;
    Uint32 startTime;
    Uint32 lastShotTime;
    Uint32 lastActivationTime;
    int cooldownRemaining;
};

struct ScreenShake {
    bool active;
    float intensity;
    Uint32 startTime;
    Uint32 duration;
};

// Thêm cấu trúc cho thông báo hồi máu vào class Game
struct HealthRegenInfo {
    bool active;
    float timeLeft;
    int amountHealed;
};

struct Particle {
    float x, y;
    float vx, vy;
    int life;
    int maxLife;
    SDL_Color color;
    bool active;
};

#endif // !STRUCTURES_H
