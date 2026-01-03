#include "game_render.h"
#include "../common/game_protocol.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct GameRenderer {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    int window_width;
    int window_height;
    int cell_size;
    bool overlay_active;
};

int renderer_init(GameRenderer** gr, const char* title, int width, int height, int cell_size) {
    *gr = malloc(sizeof(GameRenderer));
    GameRenderer* r = *gr;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;

    if (TTF_Init() != 0) {
        SDL_Quit();
        return -1;
    }

    r->window = SDL_CreateWindow(title,
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 width * cell_size,
                                 height * cell_size,
                                 SDL_WINDOW_SHOWN);
    if (!r->window) return -1;

    r->renderer = SDL_CreateRenderer(r->window, -1, SDL_RENDERER_ACCELERATED);
    if (!r->renderer) return -1;

    r->cell_size = cell_size;
    r->window_width = width;
    r->window_height = height;
    r->overlay_active = false;

    // Load font
    r->font = TTF_OpenFont("../font/PlayfulTime-BLBB8.ttf", 32);
    if (!r->font) {
        SDL_DestroyRenderer(r->renderer);
        SDL_DestroyWindow(r->window);
        TTF_Quit();
        SDL_Quit();
        free(r);
        return -1;
    }

    return 0;
}

void renderer_destroy(GameRenderer* gr) {
    if (!gr) return;

    TTF_CloseFont(gr->font);
    SDL_DestroyRenderer(gr->renderer);
    SDL_DestroyWindow(gr->window);
    TTF_Quit();
    SDL_Quit();
    free(gr);
}

SDL_Color renderer_generate_snake_color(int player_id) {
    // A fixed palette of distinguishable colors
    SDL_Color palette[] = {
        {0, 255, 0, 255},    // green
        {0, 0, 255, 255},    // blue
        {255, 255, 0, 255},  // yellow
        {255, 0, 255, 255},  // magenta
        {0, 255, 255, 255},  // cyan
        {255, 165, 0, 255},  // orange
        {128, 0, 128, 255},  // purple
        {255, 192, 203, 255} // pink
    };

    int palette_size = sizeof(palette) / sizeof(palette[0]);

    // Wrap around if player_id >= palette_size
    return palette[player_id % palette_size];
}

void renderer_draw_text(const GameRenderer* gr, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderText_Blended(gr->font, text, color);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(gr->renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(gr->renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}


void renderer_draw_game_over_overlay(const GameRenderer *gr, int score) {
    SDL_SetRenderDrawBlendMode(gr->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 180);

    SDL_Rect overlay = {
        0, 0,
        gr->window_width * gr->cell_size,
        gr->window_height * gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &overlay);

    // Hardcoded box
    SDL_SetRenderDrawColor(gr->renderer, 255, 255, 255, 255);
    SDL_Rect box = {
        overlay.w / 4,
        overlay.h / 2 - 40,  // roughly vertically centered
        overlay.w / 2,
        125                    // height of the box
    };
    SDL_RenderDrawRect(gr->renderer, &box);

    // Hardcoded text position
    SDL_Color white = {255, 255, 255, 255};
    renderer_draw_text(gr, "GAME OVER", overlay.w / 2 - 75, overlay.h / 2 - 16, white);

    // Draw score below
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %d", score);
    renderer_draw_text(gr, score_text, overlay.w / 2 - 50, overlay.h / 2 + 20, white);
}

void renderer_draw_grid(const GameRenderer* gr, int width, int height) {
    SDL_SetRenderDrawColor(gr->renderer, 50, 50, 50, 255); // dark gray
    for (int x = 0; x <= width; x++) {
        SDL_RenderDrawLine(gr->renderer, x * gr->cell_size, 0, x * gr->cell_size, height * gr->cell_size);
    }
    for (int y = 0; y <= height; y++) {
        SDL_RenderDrawLine(gr->renderer, 0, y * gr->cell_size, width * gr->cell_size, y * gr->cell_size);
    }
}

void renderer_draw_game(GameRenderer* gr, GameState* state) {
    if (!gr || !state) return;

    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gr->renderer);

    // Draw grid
    renderer_draw_grid(gr, gr->window_width, gr->window_height);

    // Draw snakes
    SDL_SetRenderDrawColor(gr->renderer, 0, 255, 0, 255);
    int num = gamestate_get_num_snakes(state);
    for (int i = 0; i < num; i++) {
        int len = gamestate_get_snake_length(state, i);
        for (int j = 0; j < len; j++) {
            SDL_Rect r;
            r.x = gamestate_get_snake_segment_x(state, i, j) * gr->cell_size;
            r.y = gamestate_get_snake_segment_y(state, i, j) * gr->cell_size;
            r.w = r.h = gr->cell_size;
            SDL_RenderFillRect(gr->renderer, &r);
        }
    }

    // Draw fruit
    SDL_SetRenderDrawColor(gr->renderer, 255, 0, 0, 255);
    SDL_Rect fr = {
        gamestate_get_fruit_x(state) * gr->cell_size,
        gamestate_get_fruit_y(state) * gr->cell_size,
        gr->cell_size,
        gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &fr);

    if (gamestate_is_game_over(state)) {
        int player_score = gamestate_get_snake_score(state, 0);
        renderer_draw_game_over_overlay(gr, player_score);
    }

    SDL_RenderPresent(gr->renderer);
}

void renderer_draw_serialized(GameRenderer* gr, const SerializedGameState* s, int snake_id) {
    if (!gr || !s) return;


    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gr->renderer);

    // Draw grid
    renderer_draw_grid(gr, gr->window_width, gr->window_height);

    // Draw snakes
    SDL_SetRenderDrawColor(gr->renderer, 0, 255, 0, 255);
    for (int i = 0; i < s->num_snakes; i++) {
        if (!s->snake_alive[i]) continue;

        SDL_Color color = renderer_generate_snake_color(i);
        SDL_SetRenderDrawColor(gr->renderer, color.r, color.g, color.b, color.a);

        for (int j = 0; j < s->snake_lengths[i]; j++) {
            SDL_Rect r;
            r.x = s->snake_x[i][j] * gr->cell_size;
            r.y = s->snake_y[i][j] * gr->cell_size;
            r.w = r.h = gr->cell_size;
            SDL_RenderFillRect(gr->renderer, &r);
        }
    }

    // Draw fruit
    SDL_SetRenderDrawColor(gr->renderer, 255, 0, 0, 255);
    SDL_Rect fr = {
        s->fruit_x * gr->cell_size,
        s->fruit_y * gr->cell_size,
        gr->cell_size,
        gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &fr);

    // Draw game over overlay
    // if (!s->snake_alive[snake_id]) {
    //     renderer_draw_game_over_overlay(
    //         gr,
    //         s->snake_scores[snake_id]
    //     );
    // }

    if (!s->snake_alive[snake_id] && !gr->overlay_active) {
        gr->overlay_active = true; // activate overlay once
    } else if (s->snake_alive[snake_id]) {
        gr->overlay_active = false; // new snake spawned, hide overlay
    }

    // in render
    if (gr->overlay_active) {
        renderer_draw_game_over_overlay(gr, s->snake_scores[snake_id]);
    }

    SDL_RenderPresent(gr->renderer);
}



