#include "game_render.h"
#include "../../common/game_protocol.h"

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

static int renderer_total_width(const GameRenderer* gr) {
    return UI_PANEL_WIDTH + gr->window_width * gr->cell_size;
}

static void renderer_draw_scoreboard(const GameRenderer* gr, const SerializedGameState* s) {
    SDL_SetRenderDrawColor(gr->renderer, 30, 30, 30, 255);
    SDL_Rect panel = {
        0,
        0,
        UI_PANEL_WIDTH,
        gr->window_height * gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &panel);

    const int padding = 10;
    const int line_height = 24;

    int x = padding;
    int y = padding;

    SDL_Color white = {255, 255, 255, 255};

    renderer_draw_text(gr, "SCOREBOARD", x, y, white);
    y += line_height + 10;

    for (int i = 0; i < s->num_snakes; i++) {
        SDL_Color color = renderer_generate_snake_color(i);

        char line[64];
        snprintf(
            line,
            sizeof(line),
            "P%d: %d %s",
            i,
            s->snake_scores[i],
            s->snake_alive[i] ? "" : "(DEAD)"
        );

        renderer_draw_text(gr, line, x, y, color);
        y += line_height;
    }
}

static void renderer_draw_snakes(const GameRenderer *gr, const SerializedGameState *s) {
    SDL_SetRenderDrawColor(gr->renderer, 0, 255, 0, 255);
    for (int i = 0; i < s->num_snakes; i++) {
        if (!s->snake_alive[i]) continue;

        const SDL_Color color = renderer_generate_snake_color(i);
        SDL_SetRenderDrawColor(gr->renderer, color.r, color.g, color.b, color.a);

        for (int j = 0; j < s->snake_lengths[i]; j++) {
            SDL_Rect r;
            r.x = UI_PANEL_WIDTH + s->snake_x[i][j] * gr->cell_size;
            r.y = s->snake_y[i][j] * gr->cell_size;
            r.w = r.h = gr->cell_size;
            SDL_RenderFillRect(gr->renderer, &r);
        }
    }
}

static void renderer_draw_player_score(const GameRenderer* gr, const int score) {
    char text[32];
    snprintf(text, sizeof text, "Score: %d", score);

    const SDL_Color white = {255, 255, 255, 255};

    const int padding = 10;
    const int bottom_margin = 40;

    renderer_draw_text(
        gr,
        text,
        padding,
        gr->window_height * gr->cell_size - bottom_margin,
        white
    );
}

GameRenderer* renderer_create(const char* title, int width, int height, int cell_size) {
    GameRenderer* gr = calloc(1, sizeof(GameRenderer));
    if (!gr) return NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        renderer_destroy(gr);
        return NULL;
    }

    if (TTF_Init() != 0) {
        renderer_destroy(gr);
        return NULL;
    }

    gr->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        UI_PANEL_WIDTH + width * cell_size,
        height * cell_size,
        SDL_WINDOW_SHOWN
    );
    if (!gr->window) {
        renderer_destroy(gr);
        return NULL;
    }

    gr->renderer = SDL_CreateRenderer(gr->window, -1, SDL_RENDERER_ACCELERATED);
    if (!gr->renderer) {
        renderer_destroy(gr);
        return NULL;
    }

    gr->font = TTF_OpenFont(
        "../client/gui/font/PlayfulTime-BLBB8.ttf",
        32
    );

    if (!gr->font) {
        renderer_destroy(gr);
        return NULL;
    }

    gr->cell_size = cell_size;
    gr->window_width = width;
    gr->window_height = height;
    gr->overlay_active = false;

    return gr;
}

void renderer_destroy(GameRenderer* gr) {
    if (!gr) return;

    if (gr->font) TTF_CloseFont(gr->font);
    if (gr->renderer) SDL_DestroyRenderer(gr->renderer);
    if (gr->window) SDL_DestroyWindow(gr->window);

    TTF_Quit();
    SDL_Quit();
    free(gr);
}

SDL_Color renderer_generate_snake_color(const int player_id) {
    // Palette
    const SDL_Color palette[] = {
        {0, 255, 0, 255},    // green
        {0, 0, 255, 255},    // blue
        {255, 255, 0, 255},  // yellow
        {255, 0, 255, 255},  // magenta
        {0, 255, 255, 255},  // cyan
        {255, 165, 0, 255},  // orange
        {128, 0, 128, 255},  // purple
        {255, 192, 203, 255} // pink
    };

    const int palette_size = sizeof(palette) / sizeof(palette[0]);

    return palette[player_id % palette_size];
}

void renderer_draw_text(const GameRenderer* gr, const char* text, const int x, const int y, const SDL_Color color) {
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


void renderer_draw_game_over_overlay(const GameRenderer *gr, const int score) {
    SDL_SetRenderDrawBlendMode(gr->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 180);

    const SDL_Rect overlay = {
        0, 0,
        renderer_total_width(gr),
        gr->window_height * gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &overlay);

    // Box
    SDL_SetRenderDrawColor(gr->renderer, 255, 255, 255, 255);
    const SDL_Rect box = {
        overlay.w / 4,
        overlay.h / 2 - 40,
        overlay.w / 2,
        125
    };
    SDL_RenderDrawRect(gr->renderer, &box);

    // Text position
    const SDL_Color white = {255, 255, 255, 255};
    renderer_draw_text(gr, "GAME OVER", overlay.w / 2 - 75, overlay.h / 2 - 16, white);

    // Draw score
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %d", score);
    renderer_draw_text(gr, score_text, overlay.w / 2 - 50, overlay.h / 2 + 20, white);
}

void renderer_draw_grid(const GameRenderer* gr, const int width, const int height) {
    SDL_SetRenderDrawColor(gr->renderer, 50, 50, 50, 255); // dark gray
    for (int x = 0; x <= width; x++) {
        SDL_RenderDrawLine(gr->renderer,UI_PANEL_WIDTH + x * gr->cell_size, 0, UI_PANEL_WIDTH + x * gr->cell_size, height * gr->cell_size);
    }
    for (int y = 0; y <= height; y++) {
        SDL_RenderDrawLine(gr->renderer, UI_PANEL_WIDTH, y * gr->cell_size, UI_PANEL_WIDTH + width * gr->cell_size, y * gr->cell_size);
    }
}

void renderer_draw_serialized(GameRenderer* gr, const SerializedGameState* s, const int snake_id, const bool client_connected) {
    if (!gr || !s) return;

    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gr->renderer);

    // Draw grid
    renderer_draw_grid(gr, gr->window_width, gr->window_height);

    // Draw snakes
    renderer_draw_snakes(gr, s);

    // Draw fruit
    SDL_SetRenderDrawColor(gr->renderer, 255, 0, 0, 255);
    const SDL_Rect fr = {
        UI_PANEL_WIDTH + s->fruit_x * gr->cell_size,
        s->fruit_y * gr->cell_size,
        gr->cell_size,
        gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &fr);

    // Scoreboard
    renderer_draw_scoreboard(gr, s);

    const int player_score = s->snake_scores[snake_id];
    renderer_draw_player_score(gr, player_score);

    if (!s->snake_alive[snake_id] && !gr->overlay_active) {
        gr->overlay_active = true;
    } else if (s->snake_alive[snake_id]) {
        gr->overlay_active = false;
    }

    // in render
    if (gr->overlay_active) {
        renderer_draw_game_over_overlay(gr, s->snake_scores[snake_id]);
    }

    if (!client_connected) {
        renderer_draw_server_shutdown_message(gr);
    }

    SDL_RenderPresent(gr->renderer);
}

void renderer_draw_server_shutdown_message(const GameRenderer* gr) {
    SDL_SetRenderDrawBlendMode(gr->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gr->renderer, 0, 0, 0, 180);

    const SDL_Rect overlay = {
        0, 0,
        renderer_total_width(gr),
        gr->window_height * gr->cell_size
    };
    SDL_RenderFillRect(gr->renderer, &overlay);

    // Box
    SDL_SetRenderDrawColor(gr->renderer, 255, 255, 255, 255);
    const SDL_Rect box = {
        overlay.w / 4,
        overlay.h / 2 - 40,
        overlay.w / 2,
        125
    };
    SDL_RenderDrawRect(gr->renderer, &box);

    // "SERVER SHUTDOWN" text
    const SDL_Color white = {255, 255, 255, 255};
    renderer_draw_text(gr, "SERVER SHUTDOWN", overlay.w / 2 - 100, overlay.h / 2 - 16, white);

    // Score
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %d", 0);
    renderer_draw_text(gr, score_text, overlay.w / 2 - 50, overlay.h / 2 + 20, white);
}