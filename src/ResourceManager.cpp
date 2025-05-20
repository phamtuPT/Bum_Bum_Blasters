#include "ResourceManager.h"

unordered_map<string, SDL_Texture*> ResourceManager::textures;
unordered_map<string, Mix_Chunk*> ResourceManager::sounds;
unordered_map<string, Mix_Music*> ResourceManager::music;

void ResourceManager::init(SDL_Renderer* renderer) {
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
    loadSound("assets/sounds/shield_activate.mp3"); // New sound for shield activation
    loadSound("assets/sounds/shield_deactivate.mp3"); // New sound for shield deactivation
    loadSound("assets/sounds/heal.mp3"); // New sound for healing

    // Pre-load music
    loadMusic("assets/sounds/background_music.mp3");
}

void ResourceManager::cleanup() {
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

SDL_Texture* ResourceManager::loadTexture(const string& path, SDL_Renderer* renderer) {
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
SDL_Texture* ResourceManager::createRecoloredTexture(const string& sourcePath, const string& newPath, SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b) {
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

SDL_Texture* ResourceManager::getTexture(const string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }
    return nullptr;
}

Mix_Chunk* ResourceManager::loadSound(const string& path) {
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

Mix_Chunk* ResourceManager::getSound(const string& path) {
    auto it = sounds.find(path);
    if (it != sounds.end()) {
        return it->second;
    }
    return nullptr;
}

Mix_Music* ResourceManager::loadMusic(const string& path) {
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

Mix_Music* ResourceManager::getMusic(const string& path) {
    auto it = music.find(path);
    if (it != music.end()) {
        return it->second;
    }
    return nullptr;
}
