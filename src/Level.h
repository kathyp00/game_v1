#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "GameObject.h"
#include "SDLState.h"
#include "GameInfo.h"
using namespace std;

enum class TileType {
    Empty = 0,
    Computer = 1,
    Grass = 2,
    Ground = 3,
    Panel = 4,
    Rock = 5,
    Underground = 6,
    Weed = 7
};

class Level {
public:
    const size_t LAYER_IDX_LEVEL = 0;
    const size_t LAYER_IDX_CHARACTERS = 1;
    bool load(const string& filename, GameInfo& info);
    void createObject(int x, int y, SDL_Texture* tex, GameInfo& info);

private:
    nlohmann::json data;
};