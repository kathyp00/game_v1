#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_mixer/SDL_mixer.h"
#include <vector>
using namespace std;

struct Resources {
    enum class PLAYER {
        IDLE,
        RUN,
        SLIDE,
        SHOOT,
        SLIDE_SHOOT,
        COUNT
    };

    enum class BULLET {
        MOVING,
        HIT
    };

    enum class ENEMY {
        IDLE,
        HIT,
        DIE
    };

    vector<Animation> playerAnims;
    vector<Animation> bulletAnims;
    vector<Animation> enemyAnims;

    vector<SDL_Texture*> textures;
    SDL_Texture *texIdle, *texRun, *texBrick, *texGrass, *texGround, *texPanel, *texSlide, *texBg1, *texBg2, *texBg3, *texBg4, *texBullet, *texBulletHit, *texShoot, *texRunShoot, *texSlideShoot, *texEnemy, *texEnemyHit, *texEnemyDie;

    MIX_Mixer* mixer;
    vector<MIX_Audio*> sounds;
    MIX_Audio *soundShoot, *soundShootHit, *soundEnemyHit, *musicMain;
    SDL_PropertiesID options;

    Resources() {
        playerAnims = { Animation(8, 1.6f), // 8 frames & run 1.6 sec
                        Animation(4, 0.5f),
                        Animation(1, 1.0f),
                        Animation(4, 0.5f),
                        Animation(4, 0.5f)};

        bulletAnims = { Animation(4, 0.05f),
                        Animation(4, 0.15f)};

        enemyAnims = { Animation(8, 1.0f),
                       Animation(8, 1.0f),
                       Animation(18, 2.0f)};
    }

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& filepath) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    MIX_Audio* loadSound(const string& filepath) {
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
        MIX_Audio* sound = MIX_LoadAudio(mixer, filepath.c_str(), true);
        MIX_Track* sfxTrack = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(sfxTrack, sound);
        MIX_SetTrackGain(sfxTrack, 0.5f); // 50 % volume
        sounds.push_back(sound);
        return sound;
    }

    void load(SDLState& state) {
        texIdle = loadTexture(state.renderer, "data/idle.png");
        texRun = loadTexture(state.renderer, "data/run.png");
        texSlide = loadTexture(state.renderer, "data/slide.png");
        texBrick = loadTexture(state.renderer, "data/tiles/brick.png");
        texGrass = loadTexture(state.renderer, "data/tiles/grass.png");
        texGround = loadTexture(state.renderer, "data/tiles/ground.png");
        texPanel = loadTexture(state.renderer, "data/tiles/panel.png");
        texBg1 = loadTexture(state.renderer, "data/bg/bg_layer1.png");
        texBg2 = loadTexture(state.renderer, "data/bg/bg_layer2.png");
        texBg3 = loadTexture(state.renderer, "data/bg/bg_layer3.png");
        texBg4 = loadTexture(state.renderer, "data/bg/bg_layer4.png");
        texBullet = loadTexture(state.renderer, "data/bullet.png");
        texBulletHit = loadTexture(state.renderer, "data/bullet_hit.png");
        texShoot = loadTexture(state.renderer, "data/shoot.png");
        texRunShoot = loadTexture(state.renderer, "data/shoot_run.png");
        texSlideShoot = loadTexture(state.renderer, "data/slide_shoot.png");
        texEnemy = loadTexture(state.renderer, "data/enemy.png");
        texEnemyHit = loadTexture(state.renderer, "data/enemy_hit.png");
        texEnemyDie = loadTexture(state.renderer, "data/enemy_die.png");

        soundShoot = loadSound("data/audio/shoot.wav");
        soundShootHit = loadSound("data/audio/wall_hit.wav");
        soundEnemyHit = loadSound("data/audio/enemy_hit.wav");
        musicMain = MIX_LoadAudio(mixer, "data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3", false);

        MIX_Track* musicTrack = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(musicTrack, musicMain);
        MIX_SetTrackGain(musicTrack, 0.3f);
        options = SDL_CreateProperties();
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        MIX_PlayTrack(musicTrack, options);
    }

    void unload() {
        for (SDL_Texture* tex : textures) {
            SDL_DestroyTexture(tex);
        }
        for (MIX_Audio* sound : sounds) {
            MIX_DestroyAudio(sound);
        }
        MIX_DestroyAudio(musicMain);
    }
};