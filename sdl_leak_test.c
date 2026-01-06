#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include "client/gui/game_render.h"
#include "server/game/game.h"

int main(void) {
    // // 1. Create game and snake
    // Game *g = game_create(60, 45);
    // Snake *s = game_add_snake(g, 30, 20);
    //
    // // 2. Define game window
    // GameRenderer *gr = NULL;
    // if (renderer_init(&gr, "Snake", game_get_width(g), game_get_heigth(g), 20) != 0) {
    //     fprintf(stderr, "SDL initialization error\n");
    //     game_destroy(g);
    //     return -1;
    // }
    //
    // int running = 1;
    // SDL_Event e;
    //
    // while (running) {
    //     // 3a. Input
    //     while (SDL_PollEvent(&e)) {
    //         if (e.type == SDL_QUIT) running = 0;
    //         if (e.type == SDL_KEYDOWN) {
    //             switch (e.key.keysym.sym) {
    //                 case SDLK_UP:    snake_set_direction(s, 0); break;
    //                 case SDLK_RIGHT: snake_set_direction(s, 1); break;
    //                 case SDLK_DOWN:  snake_set_direction(s, 2); break;
    //                 case SDLK_LEFT:  snake_set_direction(s, 3); break;
    //             }
    //         }
    //     }
    //
    //     // 3a Check whether the game hasn't ended
    //     if (!game_is_over(g)) {
    //         // game_update(g);
    //     }
    //
    //     // 3b. Get game state and render
    //     GameState* state = game_get_state(g);
    //     renderer_draw_game(gr, state);
    //     game_free_state(state);
    //
    //     // 3c. Delay
    //     SDL_Delay(100);
    // }
    //
    //
    // // 4. Free resources
    // renderer_destroy(gr);
    // game_destroy(g);
    //
    // return 0;
    // Initialize SDL and TTF
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("SDL Leak Test",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
    }

    // Create font
    TTF_Font *font = TTF_OpenFont("../client/gui/font/PlayfulTime-BLBB8.ttf", 24);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    // Rendering
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Destroy
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Quit SDL and TTF
    TTF_Quit();
    SDL_Quit();

    return 0;
}
