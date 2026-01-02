
#ifndef GAME_SERVER_H
#define GAME_SERVER_H
#include <stdbool.h>
#include "../common/game_protocol.h"
#include <stddef.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 256

typedef struct Server Server;

// Creates and initializes the server
Server* server_create(int port);

// Starts listening for clients and handling them
void server_start(Server* srv);

// Stops the server and cleans up resources
void server_destroy(Server* srv);

// Sends a message to all clients
void server_broadcast_state(Server* srv, SerializedGameState* s);

// Pushes client input to server queue
void server_push_input(Server* srv, int client_idx, int input);

// Gets next input from queue (for game logic)
bool server_get_next_input(Server* srv, int* client_idx, int* input);

#endif //GAME_SERVER_H