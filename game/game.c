#include "game.h"

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

    switch(player->direction){
        case 1: player->x--; break; // left
        case 2: player->x++; break; // right
        case 3: player->y--; break; // up
        case 4: player->y++; break; // down
    }

    if(CheckSelfCollision(player)) {
        ResetGame(player);
        return 1;
    }
    return 0;
}
