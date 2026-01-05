#include <SDL_events.h>
#include <SDL_timer.h>
#include <stdbool.h>

#include "client.h"

int main(int argc, char* argv[]) {
    // if (argc < 3) {
    //     fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
    //     return 1;
    // }
    //
    // const char* ip = argv[1];
    // int port = atoi(argv[2]);

    // Client* client = client_create(ip, port, 60, 45, 20);
    Client* client = client_create("127.0.0.1", 1338, 60, 45, 20);
    if (!client) return -1;

    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            client_handle_input(client, &event);
        }

        if (!client_receive_state(client)) quit = true;

        client_render(client);

        SDL_Delay(16); // ~60 FPS
    }

    client_destroy(client);
    return 0;
}
