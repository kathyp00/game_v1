#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_mixer/SDL_mixer.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <array>
#include <format>
using namespace std;

struct SDLState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width, height, logW, logH;
    const bool* keys;
    bool fullscreen;
    const char* title;

    SDLState(int w, int h, int lgW, int lgH, const char* t) {
        keys = SDL_GetKeyboardState(nullptr);
        width = w;
        height = h;
        logW = lgW;
        logH = lgH;
        fullscreen = false;
        title = t;
    }

    bool initialization() {
        bool initSuccess = true;
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error init sdl3", nullptr);
            initSuccess = false;
        }

        // create window
        window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
        if (!window) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error creating window", nullptr);
            cleanup();
            initSuccess = false;
        }

        // create the renderer
        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error creating renderer", window);
            cleanup();
            initSuccess = false;
        }
        SDL_SetRenderVSync(renderer, 1);

        // config presentation, keep figure scale regardless of window size
        SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

        // init sdl mixer
        if (!SDL_Init(SDL_INIT_AUDIO)) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error init audio", nullptr);
            cleanup();
            initSuccess = false;
        }
        if (!MIX_Init()) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", "error init mixer", nullptr);
            cleanup();
            initSuccess = false;
        }

        return initSuccess;
    }

    void cleanup() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};