#ifndef KILLNOTIFICATION_H
#define KILLNOTIFICATION_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

#include "ResourceManager.h"

using namespace std;

class KillNotification {
public:
    string text;
    Uint32 startTime;
    bool active;

    KillNotification(const string& text_);

    bool update();
    void render(SDL_Renderer* renderer, TTF_Font* font);
};

#endif // !KILLNOTIFICATION_H
