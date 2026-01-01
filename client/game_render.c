#include  "game_render.h"

void DrawGame(SDL_Renderer *r, int playerId, GameState *state) {
    // Clear screen first
    SDL_SetRenderDrawColor(r, 20, 20, 20, 255);
    SDL_RenderClear(r);

    SDL_Rect rect;

    // Draw grid lines
    SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    for(int i = 0; i <= GRID_WIDTH; i++)
        SDL_RenderDrawLine(r, i*CELL_SIZE, 0, i*CELL_SIZE, GRID_HEIGHT*CELL_SIZE);
    for(int j = 0; j <= GRID_HEIGHT; j++)
        SDL_RenderDrawLine(r, 0, j*CELL_SIZE, GRID_WIDTH*CELL_SIZE, j*CELL_SIZE);

    // Draw fruit
    SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
    rect = (SDL_Rect){state->fruitX*CELL_SIZE, state->fruitY*CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(r, &rect);

    // Draw all players
    for(int i = 0; i < state->numPlayers; i++) {
        if (!state->players[i].alive) continue;
        SDL_SetRenderDrawColor(
            r,
            state->players[i].r,
            state->players[i].g,
            state->players[i].b,
            255
        );

        rect.x = state->players[i].x * CELL_SIZE;
        rect.y = state->players[i].y * CELL_SIZE;
        rect.w = rect.h = CELL_SIZE;
        SDL_RenderFillRect(r, &rect);

        // Draw tail
        for(int t = 0; t < state->players[i].tail_length; t++){
            rect.x = state->players[i].tailX[t] * CELL_SIZE;
            rect.y = state->players[i].tailY[t] * CELL_SIZE;
            SDL_RenderFillRect(r, &rect);
        }
    }

    SDL_RenderPresent(r);
}