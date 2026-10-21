#pragma once
#include "GameState.h"
#include "Resources.h"
using namespace std;

struct GameInfo {
    unique_ptr<GameState> gs;
    unique_ptr<Resources> res;
    SDLState state;
    static constexpr int TILE_SIZE = 32;
    GameInfo(int width, int height, int logW, int logH, const char* title)
    : state(width, height, logW, logH, title) {}
};