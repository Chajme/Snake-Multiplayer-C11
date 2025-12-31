#ifndef UDSP_SEMESTRALKA_3_PROTOCOL_H
#define UDSP_SEMESTRALKA_3_PROTOCOL_H

#include <stdint.h>

#define PORT 13272
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define GRID_WIDTH 40
#define GRID_HEIGHT 20
#define CELL_SIZE 20
#define MAX_TAIL 100

typedef struct {
    int x, y;
    int tailX[MAX_TAIL], tailY[MAX_TAIL];
    int tail_length;
    int direction;
    int id;
    uint8_t r, g, b;
} PlayerState;

typedef struct {
    PlayerState players[MAX_CLIENTS];
    int numPlayers;
    int fruitX, fruitY;
} GameState;


// Network messages
typedef enum {
    MSG_STATE = 1,
    MSG_GAME_OVER = 2
} MessageType;

typedef struct {
    MessageType type;
    GameState state;
} ServerMessage;

#endif //UDSP_SEMESTRALKA_3_PROTOCOL_H