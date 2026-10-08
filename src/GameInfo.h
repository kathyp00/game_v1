#pragma once
#include "GameState.h"
#include "Resources.h"
using namespace std;

struct GameInfo {
    unique_ptr<GameState> gs;
    unique_ptr<Resources> res;
    static constexpr int TILE_SIZE = 32;
};