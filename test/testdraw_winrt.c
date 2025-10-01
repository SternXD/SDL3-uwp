/*
  Copyright (C) 2025 SternXD <stern@sidestore.io>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple program:  draw as many random objects on the screen as possible */

#include <SDL3/SDL.h>
#include <SDL3/SDL_test.h>

#ifdef SDL_PLATFORM_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#define NUM_OBJECTS 100

static SDLTest_CommonState *state;
static int num_objects;
static bool cycle_color;
static bool cycle_alpha;
static int cycle_direction = 1;
static int current_alpha = 255;
static int current_color = 0;
static bool done;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{
    SDLTest_CommonQuit(state);
    exit(rc);
}

void DrawPoints(SDL_Renderer *renderer)
{
    int i;
    float x, y;
    SDL_FRect viewport;

    /* Query the sizes */
    SDL_GetRenderViewport(renderer, &viewport);

    for (i = 0; i < num_objects; ++i) {
        /* Cycle the color and alpha, if desired */
        if (cycle_color) {
            current_color = (current_color + 1) % 255;
        }
        if (cycle_alpha) {
            current_alpha += cycle_direction;
            if (current_alpha < 0) {
                current_alpha = 0;
                cycle_direction = -cycle_direction;
            }
            if (current_alpha > 255) {
                current_alpha = 255;
                cycle_direction = -cycle_direction;
            }
        }

        /* Draw a random point */
        x = (float)(rand() % (int)viewport.w);
        y = (float)(rand() % (int)viewport.h);
        SDL_SetRenderDrawColor(renderer, 255, (Uint8)current_color, (Uint8)current_color, (Uint8)current_alpha);
        SDL_RenderPoint(renderer, x, y);
    }
}

void DrawLines(SDL_Renderer *renderer)
{
    int i;
    float x1, y1, x2, y2;
    SDL_FRect viewport;

    /* Query the sizes */
    SDL_GetRenderViewport(renderer, &viewport);

    for (i = 0; i < num_objects; ++i) {
        /* Cycle the color and alpha, if desired */
        if (cycle_color) {
            current_color = (current_color + 1) % 255;
        }
        if (cycle_alpha) {
            current_alpha += cycle_direction;
            if (current_alpha < 0) {
                current_alpha = 0;
                cycle_direction = -cycle_direction;
            }
            if (current_alpha > 255) {
                current_alpha = 255;
                cycle_direction = -cycle_direction;
            }
        }

        /* Draw a random line */
        x1 = (float)(rand() % (int)viewport.w);
        y1 = (float)(rand() % (int)viewport.h);
        x2 = (float)(rand() % (int)viewport.w);
        y2 = (float)(rand() % (int)viewport.h);
        SDL_SetRenderDrawColor(renderer, 255, (Uint8)current_color, (Uint8)current_color, (Uint8)current_alpha);
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }
}

void DrawRects(SDL_Renderer *renderer)
{
    int i;
    SDL_FRect rect;
    SDL_FRect viewport;

    /* Query the sizes */
    SDL_GetRenderViewport(renderer, &viewport);

    for (i = 0; i < num_objects / 4; ++i) {
        /* Cycle the color and alpha, if desired */
        if (cycle_color) {
            current_color = (current_color + 1) % 255;
        }
        if (cycle_alpha) {
            current_alpha += cycle_direction;
            if (current_alpha < 0) {
                current_alpha = 0;
                cycle_direction = -cycle_direction;
            }
            if (current_alpha > 255) {
                current_alpha = 255;
                cycle_direction = -cycle_direction;
            }
        }

        /* Draw a random rectangle */
        rect.x = (float)(rand() % (int)(viewport.w / 2));
        rect.y = (float)(rand() % (int)(viewport.h / 2));
        rect.w = (float)(rand() % (int)(viewport.h / 2));
        rect.h = (float)(rand() % (int)(viewport.h / 2));
        SDL_SetRenderDrawColor(renderer, 255, (Uint8)current_color, (Uint8)current_color, (Uint8)current_alpha);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void loop()
{
    int i;
    SDL_Event event;
    /* Check for events */
    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &done);
    }
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        if (renderer == NULL) {
            continue;
        }

        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);

        DrawRects(renderer);
        DrawLines(renderer);
        DrawPoints(renderer);

        SDL_RenderPresent(renderer);
    }
#ifdef SDL_PLATFORM_EMSCRIPTEN
    if (done) {
        emscripten_cancel_main_loop();
    }
#endif
}

// Function to be called from C++ main for UWP builds
int testdraw_main(int argc, char *argv[])
{
    int i;
    /* Initialize parameters */
    num_objects = NUM_OBJECTS;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }

    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            consumed = -1;
        }
        if (consumed < 0) {
            SDL_Log("Usage: %s [--num-objects N] [--cycle-color] [--cycle-alpha]\n",
                    argv[0]);
            SDLTest_CommonQuit(state);
            return 1;
        }
        i += consumed;
    }

    if (!SDLTest_CommonInit(state)) {
        SDLTest_CommonQuit(state);
        return 2;
    }

    /* Create the windows and initialize the renderers */
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }

    srand((unsigned int)time(NULL));

    /* Main render loop */
    done = 0;
    while (!done) {
        loop();
    }

    SDLTest_CleanupTextDrawing();
    SDLTest_CommonQuit(state);

    return 0;
}
