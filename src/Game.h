#pragma once
#include "SDLState.h"
#include "Timer.h"
#include "Animation.h"
#include "GameInfo.h"
using namespace std;

class Game {

public :
    SDLState state;
    bool isRunning = true;
    uint64_t prevTime;
    uint64_t freq;
    SDL_Event event;
    float deltaTime;
    uint64_t nowTime;
    GameInfo info;

    Game(const char* title, int width, int height, int logW, int logH);
    ~Game();

    void init();
    void handle_events();
    void game_update();
    void render();
    void create_tiles();
    void clean();
    void update(GameObject& obj);
    bool running() { return isRunning; }
    void update_player(GameObject& obj, float& currentDirection);
    void update_bullet(GameObject& obj);
    void update_enemy(GameObject& obj, float& currentDirection);
    void collision_response(const SDL_FRect& rectC, GameObject& objA, GameObject& objB);
    void check_collision(GameObject& a, GameObject& b);
    void handle_key_input(GameObject& obj, SDL_Scancode key, bool keyDown);
    void draw_paralax_background(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor);
    void draw_object(GameObject& obj, float width, float height);
};