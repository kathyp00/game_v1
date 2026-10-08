#include "Level.h"
#include <fstream>
#include <iostream>

void Level::createObject(int x, int y, SDL_Texture* tex, GameInfo& info) {
    GameObject o;
    o.type = ObjectType::level;
    o.position = glm::vec2(x, y);
    o.texture = tex;
    o.collider = { .x = 0, .y = 0, .w = static_cast<float>(info.TILE_SIZE), .h = static_cast<float>(info.TILE_SIZE)};
    info.gs->layers[LAYER_IDX_LEVEL].push_back(o);
}

bool Level::load(const string& filename, GameInfo& info) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    try {
        file >> data;
    }
    catch (const exception& e) {
        return false;
    }


    const auto& level = data["levels"][0];  // first level
    for (const auto& layer : level["layerInstances"]) {
        string type = layer["__type"];

        if (type == "IntGrid") {
            int gridWidth = layer["__cWid"]; // width of the IntGrid in cells

            for (int i = 0; i < layer["intGridCsv"].size(); i++)
            {
                int value = layer["intGridCsv"][i];

                // convert array index into grid coordinates then into world coordinates
                int gridX = i % gridWidth;
                int gridY = i / gridWidth;
                int x = gridX * info.TILE_SIZE;
                int y = gridY * info.TILE_SIZE;

                TileType type = static_cast<TileType>(value);

                switch (type) {
                    case TileType::Computer: {
                        createObject(x, y, info.res->texComputer, info);
                        break;
                    }
                    case TileType::Grass: {
                        createObject(x, y, info.res->texGrass, info);
                        break;
                    }
                    case TileType::Ground: {
                        createObject(x, y, info.res->texGround, info);
                        break;
                    }
                    case TileType::Panel: {
                        createObject(x, y, info.res->texPanel, info);
                        break;
                    }
                    case TileType::Rock: {
                        createObject(x, y, info.res->texRock, info);
                        break;
                    }
                    case TileType::Underground: {
                        createObject(x, y, info.res->texUnderground, info);
                        break;
                    }
                    case TileType::Weed: {
                        createObject(x, y, info.res->texWeed, info);
                        break;
                    }
                    default:
                        break;
                }
            }
        } else if (type == "Entities") {
            for (const auto& entity : layer["entityInstances"])
            {
                int x = entity["px"][0];
                int y = entity["px"][1];
                string identifier = entity["__identifier"];
                if (identifier == "Enemy") {
                    GameObject o;
                    o.type = ObjectType::enemy;
                    o.position = glm::vec2(x, y);
                    o.texture = info.res->texEnemy;
                    o.data.enemy = EnemyData();
                    o.currentAnimation = static_cast<int>(Resources::ENEMY::IDLE);
                    o.animations = info.res->enemyAnims;
                    o.collider = SDL_FRect {
                        .x = 10, .y = 4, .w = 12, .h = 28
                    };
                    o.maxSpeedX = 15;
                    o.dynamic = true;
                    info.gs->layers[LAYER_IDX_CHARACTERS].push_back(o);
                } else if (identifier == "Player") {
                    GameObject o;
                    o.type = ObjectType::player;
                    o.position = glm::vec2(x, y);
                    o.texture = info.res->texIdle;
                    o.data.player = PlayerData();
                    o.animations = info.res->playerAnims;
                    o.currentAnimation = static_cast<int>(Resources::PLAYER::IDLE);
                    o.acceleration = glm::vec2(300, 0);
                    o.maxSpeedX = 100;
                    o.dynamic = true;
                    o.collider = {
                        .x = 11, .y = 6,
                        .w = 10, .h = 26
                    };
                    info.gs->layers[LAYER_IDX_CHARACTERS].push_back(o);
                    info.gs->playerIndex = info.gs->layers[LAYER_IDX_CHARACTERS].size() - 1;
                }
            }
        } else {
            continue;
        }
    }
    return true;
}