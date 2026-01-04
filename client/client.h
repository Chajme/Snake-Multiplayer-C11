
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H
#include <stdbool.h>
#include <stddef.h>

typedef struct Client Client;

Client* client_create(const char* ip, int port);
void client_destroy(Client* c);

// Send a single input (direction)
void client_send_input(Client* c, int input);

// Receive game state from server (blocking)
bool client_receive_state(const Client* c, void* buffer, size_t size);

// Check if client is connected
bool client_is_connected(Client* c);


#endif //GAME_CLIENT_H