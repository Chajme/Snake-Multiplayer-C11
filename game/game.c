#include "game.h"

void SetDirection(PlayerState *player, int newDirection) {
    // Block opposite direction changes
    if ((player->direction == LEFT  && newDirection == RIGHT) ||
        (player->direction == RIGHT && newDirection == LEFT)  ||
        (player->direction == UP    && newDirection == DOWN)  ||
        (player->direction == DOWN  && newDirection == UP)) {
        return; // ignore invalid turn
        }

    player->direction = newDirection;
}

int CheckSelfCollision(PlayerState *player) {
    for(int i = 0; i < player->tail_length; i++) {
        if(player->x == player->tailX[i] &&
           player->y == player->tailY[i]) {
            return 1;
           }
    }
    return 0;
}


void ResetGame(PlayerState *player) {
    player->x = GRID_WIDTH/2;
    player->y = GRID_HEIGHT/2;
    player->tail_length = 5;
    player->direction = 2;
}

int UpdateGame(PlayerState *player) {
    int prevX = player->x, prevY = player->y;
    for(int i=player->tail_length-1; i>0; i--){
        player->tailX[i] = player->tailX[i-1];
        player->tailY[i] = player->tailY[i-1];
    }
    if(player->tail_length>0){
        player->tailX[0] = prevX;
        player->tailY[0] = prevY;
    }

    switch (player->direction) {
        case LEFT:  player->x--; break;
        case RIGHT: player->x++; break;
        case UP:    player->y--; break;
        case DOWN:  player->y++; break;
    }

    /* Wrap around */
    if (player->x < 0) player->x = GRID_WIDTH - 1;
    if (player->x >= GRID_WIDTH) player->x = 0;
    if (player->y < 0) player->y = GRID_HEIGHT - 1;
    if (player->y >= GRID_HEIGHT) player->y = 0;


    if(CheckSelfCollision(player)) {
        ResetGame(player);
        return 1;
    }
    return 0;
}
