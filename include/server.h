#ifndef UDSP_SEMESTRALKA_3_SERVER_H
#define UDSP_SEMESTRALKA_3_SERVER_H

#include "protocol.h"
#include <signal.h>
#include <pthread.h>

extern volatile sig_atomic_t server_running;
extern GameState gameState;
extern pthread_mutex_t gameMutex;
extern int clientSockets[MAX_CLIENTS];
extern int server_fd;

void handle_sigint(int sig);
void *accept_thread(void *arg);
static int find_free_player_slot(void);

#endif