#include "Game.h"


Game::Game(const char* title, int width, int height, int logW, int logH) : state(width, height, logW, logH, title) {
}

Game::~Game() {}

void Game::init() {
    if (!state.initialization()) {
        isRunning = false;
    }

    // load game assets and data
    res = make_unique<Resources>();
    res->load(state);
    gs = make_unique<GameState>(state);
    create_tiles();

    isRunning = true;
    prevTime = SDL_GetPerformanceCounter();
    freq = SDL_GetPerformanceFrequency();
}

void Game::handle_events() {
    uint64_t nowTime = SDL_GetPerformanceCounter();
    deltaTime = (float)(nowTime - prevTime) / (float)freq;
    prevTime = nowTime;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT : {
                isRunning = false;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED : {
                state.width = event.window.data1;
                state.height = event.window.data2;
                break;
            }
            case SDL_EVENT_KEY_DOWN : {
                handle_key_input(gs->player(), event.key.scancode, true);
                break;
            }
            case SDL_EVENT_KEY_UP : {
                handle_key_input(gs->player(), event.key.scancode, false);
                if (event.key.scancode == SDL_SCANCODE_F12) {
                    gs->debugMode = !gs->debugMode;
                } else if (event.key.scancode == SDL_SCANCODE_F11) {
                    state.fullscreen = !state.fullscreen;
                    SDL_SetWindowFullscreen(state.window, state.fullscreen);
                }
                break;
            }
        }
    }
}

void Game::game_update() {
    // update all objects
    for (auto& layer : gs->layers) {
        for (GameObject& obj : layer) {
            update(obj);
        }
    }

    // update bullets
    for (GameObject& bullet : gs->bullets) {
        update(bullet);
    }
}

void Game::render() {
    gs->mapViewport.x = (gs->player().position.x + TILE_SIZE / 2) - gs->mapViewport.w / 2;

    // perform drawing, whit bg
    SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
    SDL_RenderClear(state.renderer);

    // draw bg imgs, smaller factor scroll slower
    SDL_RenderTexture(state.renderer, res->texBg1, nullptr, nullptr);
    draw_paralax_background(state.renderer, res->texBg4, gs->player().velocity.x, gs->bg4Scroll, 0.075f);
    draw_paralax_background(state.renderer, res->texBg3, gs->player().velocity.x, gs->bg3Scroll, 0.150f);
    draw_paralax_background(state.renderer, res->texBg2, gs->player().velocity.x, gs->bg2Scroll, 0.3f);

    // draw bg tiles
    for (GameObject& obj : gs->backgroundTiles) {
        SDL_FRect dst {
            .x = obj.position.x - gs->mapViewport.x, 
            .y = obj.position.y,
            .w = static_cast<float>(obj.texture->w),
            .h = static_cast<float>(obj.texture->h)
        };
        SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
    }

    // draw all objects;
    for (auto& layer : gs->layers) {
        for (GameObject& obj : layer) {
            draw_object(obj, TILE_SIZE, TILE_SIZE);
        }
    }

    // draw bullets
    for (GameObject& bullet : gs->bullets) {
        if (bullet.data.bullet.state != BulletState::inactive) {
            draw_object(bullet, bullet.collider.w, bullet.collider.h);
        }
        
    }

    // draw fg tiles
    for (GameObject& obj : gs->foregroundTiles) {
        SDL_FRect dst {
            .x = obj.position.x - gs->mapViewport.x, 
            .y = obj.position.y,
            .w = static_cast<float>(obj.texture->w),
            .h = static_cast<float>(obj.texture->h)
        };
        SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
    }

    // display some debug info
    if (gs->debugMode) {
        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(state.renderer, 5, 5, format("S: {}, B: {}, G: {}", static_cast<int>(gs->player().data.player.state), gs->bullets.size(), gs->player().grounded).c_str());
    }

    // swap buffres and present
    SDL_RenderPresent(state.renderer);
}


void Game::create_tiles() {
    /*
        1 - ground
        2 - panel
        3 - enemy
        4 - player
        5 - grass
        6 - brick
    */
    short map[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 2, 0, 0, 0, 0, 0, 3, 2, 2, 2, 0, 0, 0, 0, 2, 0, 2, 0, 0, 3, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    short foreground[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        5, 0, 0, 5, 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    short background[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    const auto loadMap = [this](short layer[MAP_ROWS][MAP_COLS]) {
        const auto createObject = [this](int r, int c, SDL_Texture* tex, ObjectType type) {
            GameObject o;
            o.type = type;
            o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
            o.texture = tex;
            o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE};
            return o;
        };

        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                switch (layer[r][c]) {
                    case 1 : {
                        GameObject o = createObject(r, c, res->texGround, ObjectType::level);
                        gs->layers[LAYER_IDX_LEVEL].push_back(o);
                        break;
                    }
                    case 2 : {
                        GameObject o = createObject(r, c, res->texPanel, ObjectType::level);
                        gs->layers[LAYER_IDX_LEVEL].push_back(o);
                        break;
                    }
                    case 3 : {
                        GameObject o = createObject(r, c, res->texEnemy, ObjectType::enemy);
                        o.data.enemy = EnemyData();
                        o.currentAnimation = static_cast<int>(Resources::ENEMY::IDLE);
                        o.animations = res->enemyAnims;
                        o.collider = SDL_FRect {
                            .x = 10, .y = 4, .w = 12, .h = 28
                        };
                        o.maxSpeedX = 15;
                        o.dynamic = true;
                        gs->layers[LAYER_IDX_CHARACTERS].push_back(o);
                        break;
                    }
                    case 4 : {
                        GameObject player = createObject(r, c, res->texIdle, ObjectType::player);
                        player.data.player = PlayerData();
                        player.animations = res->playerAnims;
                        player.currentAnimation = static_cast<int>(Resources::PLAYER::IDLE);
                        player.acceleration = glm::vec2(300, 0);
                        player.maxSpeedX = 100;
                        player.dynamic = true;
                        player.collider = {
                            .x = 11, .y = 6,
                            .w = 10, .h = 26
                        };
                        gs->layers[LAYER_IDX_CHARACTERS].push_back(player);
                        gs->playerIndex = gs->layers[LAYER_IDX_CHARACTERS].size() - 1;
                        break;
                    }
                    case 5 : {
                        GameObject o = createObject(r, c, res->texGrass, ObjectType::level);
                        gs->foregroundTiles.push_back(o);
                        break;
                    }
                    case 6 : {
                        GameObject o = createObject(r, c, res->texBrick, ObjectType::level);
                        gs->backgroundTiles.push_back(o);
                        break;
                    }
                }
            }
        }
    };
    loadMap(map);
    loadMap(background);
    loadMap(foreground);
    assert(gs->playerIndex != -1);
}

void Game::clean() {
    state.cleanup();
    res->unload();
    SDL_DestroyProperties(res->options);
}

void Game::update(GameObject& obj) {

    // update animation
    if (obj.currentAnimation != -1) {
        obj.animations[obj.currentAnimation].step(deltaTime);
    }

    if (obj.dynamic && !obj.grounded) {
        // apply some gravity
        obj.velocity += glm::vec2(0, 500) * deltaTime;
    }

    float currentDirection = 0;
    if (obj.type == ObjectType::player) {
        currentDirection = update_player(obj);
    } else if (obj.type == ObjectType::bullet) {
        update_bullet(obj);
    } else if (obj.type == ObjectType::enemy) {
        currentDirection = update_enemy(obj);
    }
    obj.direction = currentDirection;

    // add acceleration to velocity
    obj.velocity += currentDirection * obj.acceleration * deltaTime;
    if (abs(obj.velocity.x) > obj.maxSpeedX) {
        obj.velocity.x = currentDirection * obj.maxSpeedX;
    }

    // add velocity to position
    obj.position += obj.velocity * deltaTime;

    // handle collision detection
    if (obj.type != ObjectType::level) {
        for (auto& layer : gs->layers) {
            for (GameObject& objB : layer) {
                if (&obj == &objB)
                    continue;
                check_collision(obj, objB);
            }
        }
    }
}

float Game::update_player(GameObject& obj) {
    float currentDirection = state.keys[SDL_SCANCODE_A] ? (currentDirection - 1) : ((state.keys[SDL_SCANCODE_D]) ? (currentDirection + 1) : currentDirection);
    Timer& weaponTimer = obj.data.player.weaponTimer;
    weaponTimer.step(deltaTime);

    const auto handleShooting = [&obj, &weaponTimer, this](
        SDL_Texture* tex, SDL_Texture* shootTex, int animIndex, int shootAnimIndex) {
        if (state.keys[SDL_SCANCODE_J]) {
            // set shooting tex/anim
            obj.texture = shootTex;
            obj.currentAnimation = shootAnimIndex;
            if (weaponTimer.isTimeout()) {
                weaponTimer.reset();
                // spawn some bullets
                GameObject bullet;
                bullet.data.bullet = BulletData();
                bullet.type = ObjectType::bullet;
                bullet.direction = gs->player().direction;
                bullet.texture = res->texBullet;
                bullet.currentAnimation = static_cast<int>(Resources::BULLET::MOVING);
                bullet.collider = SDL_FRect {
                    .x = 0, .y = 0,
                    .w = static_cast<float>(res->texBullet->h),
                    .h = static_cast<float>(res->texBullet->h),
                };

                const int yVariation = 40;
                const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
                bullet.velocity = glm::vec2(
                    obj.velocity.x + 600.0f * obj.direction,
                    yVelocity
                );
                bullet.maxSpeedX = 1000.0f;
                bullet.animations = res->bulletAnims;

                // adjust bullet start position
                const float left = 4;
                const float right = 24;
                const float t = (obj.direction + 1) / 2.0f; // result in value of 0..1
                const float xOffset = left + right * t; // LERP btw left and right based on direction 
                bullet.position = glm::vec2(
                    obj.position.x + xOffset,
                    obj.position.y + TILE_SIZE / 2 + 1
                );

                // look for an inactive slot and overwrite the bullet
                bool foundInactive = false;
                for (int i = 0; i < gs->bullets.size() && !foundInactive; i++) {
                    if (gs->bullets[i].data.bullet.state == BulletState::inactive) {
                        foundInactive = true;
                        gs->bullets[i] = bullet;
                    }
                }
                // if not active slot was found
                if (!foundInactive) {
                    gs->bullets.push_back(bullet);
                }
                MIX_PlayAudio(res->mixer, res->soundShoot);
            }
        } else {
            obj.texture = tex;
            obj.currentAnimation = animIndex;
        }
    };

    switch (obj.data.player.state) {
        case PlayerState::idle : {
            if (currentDirection) {
                obj.data.player.state = PlayerState::running;
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
            handleShooting(res->texIdle, res->texShoot, static_cast<int>(Resources::PLAYER::IDLE), static_cast<int>(Resources::PLAYER::SHOOT));
            break;
        }
        case PlayerState::running : {
            if (!currentDirection) {
                obj.data.player.state = PlayerState::idle;
            }

            // moving in opposite direction of velocity, sliding
            // only neg when signs are diff
            if (obj.velocity.x * obj.direction < 0 && obj.grounded) {
                handleShooting(res->texSlide, res->texSlideShoot, static_cast<int>(Resources::PLAYER::SLIDE), static_cast<int>(Resources::PLAYER::SLIDE_SHOOT));
            } else {
                handleShooting(res->texRun, res->texRunShoot, static_cast<int>(Resources::PLAYER::RUN), static_cast<int>(Resources::PLAYER::RUN));
            }
            break;
        }
        case PlayerState::jumping : {
            if (!currentDirection) { // I added this bc it seems to get stuck in this state
                obj.data.player.state = PlayerState::idle;
            }
            handleShooting(res->texRun, res->texRunShoot, static_cast<int>(Resources::PLAYER::RUN), static_cast<int>(Resources::PLAYER::RUN));
            break;
        }
    }
    return currentDirection;
}

void Game::update_bullet(GameObject& obj) {
    switch (obj.data.bullet.state) {
        case BulletState::moving : {
            // bullet passed edge of screen
            if (obj.position.x - gs->mapViewport.x < 0 ||
                obj.position.x - gs->mapViewport.x > state.logW ||
                obj.position.y - gs->mapViewport.y < 0 ||
                obj.position.y - gs->mapViewport.y > state.logH) {
                obj.data.bullet.state = BulletState::inactive;
            }
            break;
        }
        case BulletState::colliding : {
            if (obj.animations[obj.currentAnimation].isDone()) {
                obj.data.bullet.state = BulletState::inactive;
            }
            break;
        }
    }
}

float Game::update_enemy(GameObject& obj) {
    float currentDirection = 0;
    EnemyData& d = obj.data.enemy;
    switch (d.state) {
        case EnemyState::shambling : {
            glm::vec2 playerDir = gs->player().position - obj.position;
            if (glm::length(playerDir) < 100) {
                currentDirection = playerDir.x < 0 ? -1 : 1;
                obj.acceleration = glm::vec2(30,0);
            } else {
                obj.acceleration = glm::vec2(0);
                obj.velocity.x = 0;
            }
            break;
        }
        case EnemyState::damaged : {
            if (d.damagedTimer.step(deltaTime)) {
                d.state = EnemyState::shambling;
                obj.texture = res->texEnemy;
                obj.currentAnimation = static_cast<int>(Resources::ENEMY::IDLE);
            }
            break;
        }
        case EnemyState::dead : {
            obj.velocity.x = 0;
            if (obj.currentAnimation != -1 &&
                obj.animations[obj.currentAnimation].isDone()) {
                // remove animation & set to last frame
                obj.currentAnimation = -1;
                obj.spriteFrame = 18;
            }
            break;
        }
    }
    return currentDirection;
}

void Game::collision_response(const SDL_FRect& rectC, GameObject& objA, GameObject& objB) {
    const auto genericResponse = [&]() {
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
    };

    // obj we're checking
    if (objA.type == ObjectType::player) {
        // obj it's colliding with
        switch(objB.type) {
            case ObjectType::level : {
                genericResponse();
                objA.data.player.state = PlayerState::running;
                break;
            }
            case ObjectType::enemy : {
                if (objB.data.enemy.state != EnemyState::dead) {
                    objA.velocity = glm::vec2(100, 0) * -objA.direction;
                }
                break;
            }
        }
    } else if (objA.type == ObjectType::bullet) {
        bool passthrough = false;
        switch (objA.data.bullet.state) {
            case BulletState::moving : {
                switch(objB.type) {
                    case ObjectType::level : {
                        MIX_PlayAudio(res->mixer, res->soundShootHit);
                        break;
                    }
                    case ObjectType::enemy : {
                        EnemyData& d = objB.data.enemy;
                        if (d.state != EnemyState::dead) {
                            objB.direction = -objA.direction;
                            objB.shouldFlash = true;
                            objB.flashTimer.reset();
                            objB.texture = res->texEnemyHit;
                            objB.currentAnimation = static_cast<int>(Resources::ENEMY::HIT);
                            d.state = EnemyState::damaged;
                            d.healthPoints -= 10;
                            if (d.healthPoints <= 0) {
                                d.state = EnemyState::dead;
                                objB.texture = res->texEnemyDie;
                                objB.currentAnimation = static_cast<int>(Resources::ENEMY::DIE);
                                MIX_PlayAudio(res->mixer, res->soundEnemyHit);
                            }
                        } else {
                            passthrough = true;
                        }
                        break;
                    }
                }
                if (!passthrough) {
                    genericResponse();
                    objA.velocity += 0;
                    objA.data.bullet.state = BulletState::colliding;
                    objA.texture = res->texBulletHit;
                    objA.currentAnimation = static_cast<int>(Resources::BULLET::HIT);
                }
                
                break;
            }
        }
    } else if (objA.type == ObjectType::enemy) {
        genericResponse();
    }
}

void Game::check_collision(GameObject& a, GameObject& b) {
    SDL_FRect rectA {
        .x = a.position.x + a.collider.x, 
        .y= a.position.y + a.collider.y,
        .w = a.collider.w, 
        .h = a.collider.h
    };
    SDL_FRect rectB {
        .x = b.position.x + b.collider.x, 
        .y= b.position.y + b.collider.y,
        .w = b.collider.w, 
        .h = b.collider.h
    };
    SDL_FRect rectC { 0 };
    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC)) {
        // found intersection, respond
        collision_response(rectC, a, b);
    }
}

void Game::handle_key_input(GameObject& obj, SDL_Scancode key, bool keyDown) {

    const float JUMP_FORCE = -200.0f; // upward force
    if (obj.type == ObjectType::player) {
        switch(obj.data.player.state) {
            case PlayerState::idle : {
                if (key == SDL_SCANCODE_K && keyDown && obj.grounded) {
                    obj.data.player.state = PlayerState::jumping;
                    obj.velocity.y += JUMP_FORCE;
                }
                break;
            }
            case PlayerState::running : {
                if (key == SDL_SCANCODE_K && keyDown && obj.grounded) {
                    obj.data.player.state = PlayerState::jumping;
                    obj.velocity.y += JUMP_FORCE;
                }
                break;
            }
        }
    }
}

void Game::draw_paralax_background(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor) {
    scrollPos -= xVelocity * scrollFactor * deltaTime;
    if (scrollPos <= -texture->w) {
        scrollPos = 0;
    }
    SDL_FRect dst {
        .x = scrollPos, .y = 30,
        .w = texture->w * 2.0f,
        .h = static_cast<float>(texture->h)
    };
    SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}

void Game::draw_object(GameObject& obj, float width, float height) {
    float srcX = obj.currentAnimation != -1 
                ? obj.animations[obj.currentAnimation].currentFrame() * width : (obj.spriteFrame - 1) * width;

    SDL_FRect src {
        .x = srcX,
        .y = 0,
        .w = width,
        .h = height
    };

    // where to draw sprite
    SDL_FRect dst {
        .x = obj.position.x - gs->mapViewport.x,
        .y = obj.position.y,
        .w = width,
        .h = height
    };

    SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    if (!obj.shouldFlash) {
        SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
    } else {
        // flash object
        SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
        SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
        SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

        if (obj.flashTimer.step(deltaTime)) {
            obj.shouldFlash = false;
        }
    }

    if (gs->debugMode) {
        SDL_FRect rectA {
            .x = obj.position.x + obj.collider.x - gs->mapViewport.x, 
            .y= obj.position.y + obj.collider.y,
            .w = obj.collider.w, 
            .h = obj.collider.h
        };
        SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
        SDL_RenderFillRect(state.renderer, &rectA);
        SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
    }
}