#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;

class ResourceManager {
private:
    static unordered_map<string, SDL_Texture*> textures;
    static unordered_map<string, Mix_Chunk*> sounds;
    static unordered_map<string, Mix_Music*> music;

public:
    static void init(SDL_Renderer* renderer);
    static void cleanup();
    static SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer);
    // New function to create a recolored texture
    static SDL_Texture* createRecoloredTexture(const string& sourcePath, const string& newPath, SDL_Renderer* renderer,
                                               Uint8 r, Uint8 g, Uint8 b);
    static SDL_Texture* getTexture(const string& path);
    static Mix_Chunk* loadSound(const string& path);
    static Mix_Chunk* getSound(const string& path);
    static Mix_Music* loadMusic(const string& path);
    static Mix_Music* getMusic(const string& path);
};

#endif // !RESOURCEMANAGER_H
