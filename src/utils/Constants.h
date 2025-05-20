#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL.h>

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

#endif // !CONSTANTS_H
