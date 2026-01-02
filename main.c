#include <stdio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include "game/game.h"
#include "gui/game_render.h"

int main(void) {
    // 1. Create game and snake
    Game *g = game_create(60, 45);
    Snake *s = game_add_snake(g, 30, 20);

    // 2. Define game window
    GameRenderer *gr = NULL;
    if (renderer_init(&gr, "Snake", game_get_width(g), game_get_heigth(g), 20) != 0) {
        fprintf(stderr, "SDL initialization error\n");
        game_destroy(g);
        return -1;
    }

    int running = 1;
    SDL_Event e;

    while (running) {
        // 3a. Input
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:    snake_set_direction(s, 0); break;
                    case SDLK_RIGHT: snake_set_direction(s, 1); break;
                    case SDLK_DOWN:  snake_set_direction(s, 2); break;
                    case SDLK_LEFT:  snake_set_direction(s, 3); break;
                }
            }
        }

        // 3a Check whether the game hasn't ended
        if (!game_is_over(g)) {
            game_update(g);
        }

        // 3b. Get game state and render
        GameState* state = game_get_state(g);
        renderer_draw_game(gr, state);
        game_free_state(state);

        // 3c. Delay
        SDL_Delay(100);
    }


    // 4. Free resources
    renderer_destroy(gr);
    game_destroy(g);

    return 0;
}
