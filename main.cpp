#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <iostream>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 0.5f;
const float JUMP_FORCE = -12.0f;         // Force added on jump
const float MAX_JUMP_SPEED = -15.0f;    // Max upward speed
const float ROTATION_ANGLE = 15.0f;
const float BOUNCE_DAMPING = -0.7f;     // Damping factor for ceiling bounce
const Uint32 PRINT_INTERVAL = 100;      // Print every 100 ms (0.1 seconds)

struct Character {
    SDL_Rect rect;
    float velocityY;
    float angle;
};

void cleanup(SDL_Window* window, SDL_Renderer* renderer,
             SDL_Texture* bgTexture, SDL_Texture* charTexture) {
    if (charTexture) SDL_DestroyTexture(charTexture);
    if (bgTexture) SDL_DestroyTexture(bgTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        cerr << "Failed to load " << filePath << ": " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        cerr << "Failed to create texture from " << filePath << ": " << SDL_GetError() << endl;
    }
    return texture;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) == 0) {
        cerr << "SDL/IMG Init failed: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Drop Bum Bum!!!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window creation failed: " << SDL_GetError() << endl;
        cleanup(nullptr, nullptr, nullptr, nullptr);
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer creation failed: " << SDL_GetError() << endl;
        cleanup(window, nullptr, nullptr, nullptr);
        return 1;
    }

    SDL_Texture* bgTexture = loadTexture(renderer, "./Images/nen_game.png");
    SDL_Texture* charTexture = loadTexture(renderer, "./Images/ufo.png");
    if (!bgTexture || !charTexture) {
        cleanup(window, renderer, bgTexture, charTexture);
        return 1;
    }

    Character character = {{100, WINDOW_HEIGHT - 100, 50, 50}, 0.0f, 0.0f};
    const int groundY = WINDOW_HEIGHT - character.rect.h;
    Uint32 lastPrintTime = 0; // Track last console update time

    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && event.key.repeat == 0) {
                character.velocityY += JUMP_FORCE;

                // Limit max upward speed
                if (character.velocityY < MAX_JUMP_SPEED)
                    character.velocityY = MAX_JUMP_SPEED;

                character.angle = ROTATION_ANGLE;
            }
        }

        // Update physics
        character.velocityY += GRAVITY;
        character.rect.y += static_cast<int>(character.velocityY);

        // Hit ceiling
        if (character.rect.y <= 0) {
            character.rect.y = 0;
            character.velocityY *= BOUNCE_DAMPING; // Reverse velocity with damping
            character.angle = 0.0f; // Reset angle for visual consistency
        }

        // When falling, gradually reduce angle to 0
        if (character.velocityY > 0.0f && character.angle > 0.0f) {
            character.angle -= 2.0f;
            if (character.angle < 0.0f) character.angle = 0.0f;
        }

        // Hit ground
        if (character.rect.y >= groundY) {
            character.rect.y = groundY;
            character.velocityY = 0.0f;
            character.angle = 0.0f;
        }

        // Print vertical movement state every 0.1 seconds
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
            if (character.velocityY < 0.0f) {
                cout << "len" << endl;
            } else if (character.velocityY > 0.0f) {
                cout << "xuong" << endl;
            }
            // Skip printing when velocityY == 0 (e.g., on ground)
            lastPrintTime = currentTime;
        }

        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr);
        SDL_RenderCopyEx(renderer, charTexture, nullptr, &character.rect,
                         character.angle, nullptr, SDL_FLIP_NONE);
        SDL_RenderPresent(renderer);

        SDL_Delay(32); // ~60 FPS
    }

    cleanup(window, renderer, bgTexture, charTexture);
    return 0;
}
