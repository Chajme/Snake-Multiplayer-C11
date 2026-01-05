
#ifndef SNAKEGAME_CLIENT_H
#define SNAKEGAME_CLIENT_H
#include <SDL_events.h>
#include <stdbool.h>

typedef struct Client Client;

Client *client_create(const char *ip, int port, int width, int height, int cell_size);
void client_destroy(Client *c);

void client_sent_input(Client *c, int direction);
bool client_receive_state(Client *c);

void client_handle_input(Client *c, const SDL_Event *event);
void client_render(Client *c);


#endif