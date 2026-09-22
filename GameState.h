#pragma once
#include "GameObject.h"
struct GameState {
    const size_t LAYER_IDX_LEVEL = 0;
    const size_t LAYER_IDX_CHARACTERS = 1;
    array<vector<GameObject>, 2> layers;
    vector<GameObject> backgroundTiles;
    vector<GameObject> foregroundTiles;
    vector<GameObject> bullets;

    int playerIndex;
    SDL_FRect mapViewport;
    float bg2Scroll, bg3Scroll, bg4Scroll;
    bool debugMode;

    GameState() {}
    GameState(SDLState& state) {
        playerIndex = -1;
        mapViewport = SDL_FRect {
            .x = 0, .y = 0, 
            .w = static_cast<float>(state.logW), 
            .h = static_cast<float>(state.logH)
        };
        bg2Scroll = bg3Scroll = bg4Scroll = 0;
        debugMode = false;
    }
    GameObject& player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }
};