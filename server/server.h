
#ifndef SNAKEGAME_SERVER_H
#define SNAKEGAME_SERVER_H
#include <stdbool.h>
#include "../common/game_protocol.h"

#define MAX_CLIENTS 5
#define BUFFER_SIZE 256

typedef struct Server Server;

/* Lifecycle */
Server* server_create(int port);
void server_start(Server* srv);
void server_start_async(Server* srv);
void server_stop(Server* srv);
void server_destroy(Server* srv);

/* Networking */
void server_broadcast_state(Server* srv, const SerializedGameState* state);

/* Input handling (event-based) */
bool server_poll_input(Server* srv, int* player_id, int* direction);

/* Status */
bool server_is_running(const Server* srv);
bool server_is_client_connected(const Server *srv, int player_id);

#endif