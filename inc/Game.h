#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <vector>

#include "Structures.h"
#include "Constants.h"
#include "ResourceManager.h"
#include "Tank.h"
#include "Bullet.h"
#include "Explosion.h"
#include "PowerUp.h"
#include "ParticleSystem.h"
#include "KillNotification.h"

using namespace std;

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

    // Menu properties
    SDL_Texture* menuBackgroundTexture;
    vector<MenuButtonInfo> menuButtons;
    MenuButton currentHoveredButton;
    Stats stats;
    RapidFire rapidFire;
    ScreenShake screenShake;
    HealthRegenInfo healthRegenInfo;

    unordered_map<MenuButton, ButtonAnimation> buttonAnimations;

public:
    Game();
    ~Game();

    int run();

private:
    void init(SDL_Renderer* rend, TTF_Font* f);
    void handleEvents(SDL_Event& e, bool& quit);
    void update(float deltaTime);
    void render();

    void handleSpecialAbility(float deltaTime);
    void renderSpecialTargetingLine();
    void handleMenuEvents(SDL_Event& e, bool& quit);
    void handlePauseEvents(SDL_Event& e);
    void handleGameEvents(SDL_Event& e);
    void useHealthPickup();
    void activateShield();
    void activateScreenShake(float intensity, Uint32 duration);
    bool isMouseInsideBorder(int mouseX, int mouseY);
    void updateCamera();
    void updateDifficulty();
    void handleWallBounce(Tank& tank);
    void shoot();
    void handleRapidFire(float deltaTime);
    void spawnEnemy();
    void spawnPowerUp();
    void spawnHealthPickup();
    void updateEnemyBehavior(Tank& enemy, float deltaTime);
    void enemyShoot(Tank& enemy);
    void handleCollisions();
    void applyPowerUp(const PowerUp& powerup);
    void cleanup();
    void reset();
    void renderGame();
    void updateHealthRegenInfo(float deltaTime);
    void renderHealthRegenInfo();
    void renderCooldowns();
    void renderCooldownBar(int x, int y, int width, int height, float percentage, SDL_Color color);
    void renderText(const string& text, int x, int y, SDL_Color color);
    void renderMenu();
    void renderPauseMenu();
    void renderGameOver();
    void renderStats();
    void renderMinimap();
    void initializeMenu();
    void updateButtonAnimations();
};

#endif // !GAME_H
