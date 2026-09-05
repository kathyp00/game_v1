#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string>
#include <array>
#include "game_object.h"

using namespace std;

struct SDLState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width, height, logW, logH;
    const bool* keys;

    SDLState() : keys(SDL_GetKeyboardState(nullptr)) {}
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState {
    array<vector<GameObject>, 2> layers;
    int playerIndex;
    GameState() {
        playerIndex = 0; 
    }
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    vector<Animation> playerAnims;
    vector<SDL_Texture*> textures;
    SDL_Texture* texIdle, *texRun, *texBrick, *texGrass, *texGround, *texPanel;


    SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& filepath) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    void load(SDLState& state) {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f); // 8 frames & run 1.6 sec
        playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
        texIdle = loadTexture(state.renderer, "data/idle.png");
        texRun = loadTexture(state.renderer, "data/run.png");
        texBrick = loadTexture(state.renderer, "data/tiles/brick.png");
        texGrass = loadTexture(state.renderer, "data/tiles/grass.png");
        texGround = loadTexture(state.renderer, "data/tiles/ground.png");
        texPanel = loadTexture(state.renderer, "data/tiles/panel.png");
    }

    void unload() {
        for (SDL_Texture* tex : textures) {
            SDL_DestroyTexture(tex);
        }
    }


};

bool initialization(SDLState& state);
void cleanup(SDLState&  state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
void createTiles(const SDLState& state, GameState& gs,  const Resources& res);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);

int main(int argc, char* argv[]) {
    SDLState state;
    state.width = 1600;
    state.height = 900;
    state.logW = 640;
    state.logH = 320;

    if (!initialization(state)) {
        return 1;
    }

    // load game assets 
    Resources res;
    res.load(state);

    // setup game data
    GameState gs;
    createTiles(state, gs, res);

    uint64_t prevTime = SDL_GetTicks();

    // game loop
    bool running = true;
    while (running) {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f; // converts to seconds
        SDL_Event event {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT :
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED :
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    break;
            }
        }

        // update all objects
        for (auto& layer : gs.layers) {
            for (GameObject& obj : layer) {
                update(state, gs, res, obj, deltaTime);

                // update animation
                if (obj.currentAnimation != -1) {
                    obj.animations[obj.currentAnimation].step(deltaTime);
                }
            }
        }

        // perform drawing, whit bg
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        // draw all objects;
        for (auto& layer : gs.layers) {
            for (GameObject& obj : layer) {
                drawObject(state, gs, obj, deltaTime);
            }
        }

        // swap buffres and present
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }

    res.unload();
    cleanup(state);
    return 0;
}

bool initialization(SDLState& state) {
    bool initSuccess = true;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error init sdl3", nullptr);
        initSuccess = false;
    }

    // create window
    state.window = SDL_CreateWindow("sdl3 game", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if (!state.window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error creating window", nullptr);
        cleanup(state);
        initSuccess = false;
    }

    // create the renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error creating renderer", state.window);
        cleanup(state);
        initSuccess = false;
    }

    // config presentation, keep figure scale regardless of window size
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return initSuccess;
}

void cleanup(SDLState&  state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime) {
    const float spriteSize = 32;
    float srcX = obj.currentAnimation != -1 
                ? obj.animations[obj.currentAnimation].currentFrame() * spriteSize : 0.0f;

    SDL_FRect src {
        .x = srcX,
        .y = 0,
        .w = spriteSize,
        .h = spriteSize
    };

    // where to draw sprite
    SDL_FRect dst {
        .x = obj.position.x,
        .y = obj.position.y,
        .w = spriteSize,
        .h = spriteSize
    };

    SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
}

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {

    if (obj.dynamic) {
        // apply some gravity
        obj.velocity += glm::vec2(0, 500) * deltaTime;
    }

    if (obj.type == ObjectType::player) {
        float currentDirection = 0;
        if (state.keys[SDL_SCANCODE_A]) {
            currentDirection += -1;
        }
        if (state.keys[SDL_SCANCODE_D]) {
            currentDirection += 1;
        }
        if (currentDirection) {
            obj.direction = currentDirection;
        }

        switch (obj.data.player.state) {
            case PlayerState::idle : {
                if (currentDirection) {
                    obj.data.player.state = PlayerState::running;
                    obj.texture = res.texRun;
                    obj.currentAnimation = res.ANIM_PLAYER_RUN;
                } else {
                    // deaccelerate
                    if (obj.velocity.x) {
                        const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
                        float amount = factor * obj.acceleration.x * deltaTime;
                        if (abs(obj.velocity.x) < abs(amount)) {
                            obj.velocity.x = 0;
                        } else {
                            obj.velocity.x += amount;
                        }
                    }
                }
                break;
            }
            case PlayerState::running : {
                if (!currentDirection) {
                    obj.data.player.state = PlayerState::idle;
                    obj.texture = res.texIdle;
                    obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                }
                break;
            }
            case PlayerState::jumping : {
                break;
            }
        }

        // add acceleration to velocity
        obj.velocity += currentDirection * obj.acceleration * deltaTime;
        if (abs(obj.velocity.x) > obj.maxSpeedX) {
            obj.velocity.x = currentDirection * obj.maxSpeedX;
        }
    }
    // add velocity to position
    obj.position += obj.velocity * deltaTime;

    // handle collision detection
    for (auto& layer : gs.layers) {
        for (GameObject& objB : layer) {
            if (&obj != &objB) {
                checkCollision(state, gs, res, obj, objB, deltaTime);
            }
        }
    }
}

void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject& objA, GameObject& objB, float deltaTime) {
    // obj we're checking
    if (objA.type == ObjectType::player) {
        // obj it's colliding with
        switch(objB.type) {
            case ObjectType::level : {
                if (rectC.w < rectC.h) {
                    // horizontal collision
                    if (objA.velocity.x > 0) { // right
                        objA.position.x -= rectC.w;
                    } else if (objA.velocity.x < 0) { // left
                        objA.position.x += rectC.w;
                    }
                    objA.velocity.x = 0;

                } else {
                    // vertical collision
                    if (objA.velocity.y > 0) { // down
                        objA.position.y -= rectC.h;
                    } else if (objA.velocity.y < 0){ // up
                        objA.position.y += rectC.h;
                    }
                    objA.velocity.y = 0;
                }
                break;
            }
        }
    }
}

void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime) {
    SDL_FRect rectA {
        .x = a.position.x, .y= a.position.y,
        .w = TILE_SIZE, .h = TILE_SIZE
    };
    SDL_FRect rectB {
        .x = b.position.x, .y= b.position.y,
        .w = TILE_SIZE, .h = TILE_SIZE
    };
    SDL_FRect rectC { 0 };
    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC)) {
        // found intersection, respond
        collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime);

    }

}

void createTiles(const SDLState& state, GameState& gs,  const Resources& res) {
    /*
        1 - ground
        2 - panel
        3 - enemy
        4 - player
        5 - grass
        6 - brick
    */
    short map[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    const auto createObject = [&state](int r, int c, SDL_Texture* tex, ObjectType type) {
        GameObject o;
        o.type = type;
        o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
        o.texture = tex;
        return o;
    };

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            switch (map[r][c]) {
                case 1 : {
                    GameObject o = createObject(r, c, res.texGround, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 2 : {
                    GameObject o = createObject(r, c, res.texPanel, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 4 : {
                    GameObject player = createObject(r, c, res.texIdle, ObjectType::player);
                    player.data.player = PlayerData();
                    player.animations = res.playerAnims;
                    player.currentAnimation = res.ANIM_PLAYER_IDLE;
                    player.acceleration = glm::vec2(300, 0);
                    player.maxSpeedX = 100;
                    player.dynamic = true;
                    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);   
                    break;
                }
                default :
                    break;
            }
        }
    }
}