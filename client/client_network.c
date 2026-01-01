// client_network.c
#include "../include/client_network.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static size_t recv_all(int sock, void *buf, size_t len){
    size_t total = 0;
    char *p = buf;
    while(total < len){
        ssize_t n = recv(sock, p + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

int ConnectToServer(ClientNet *net, const char *ip, int port){
    net->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(net->sockfd < 0) {
        return -1;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    if(connect(net->sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;

    if(recv(net->sockfd, &net->playerId, sizeof(int), 0) <= 0) return -1;
    return 0;
}

int SendDirection(ClientNet *net, uint8_t dir){
    return send(net->sockfd, &dir, sizeof(dir), 0) == sizeof(dir) ? 0 : -1;
}

int ReceiveGameState(ClientNet *net, GameState *state){
    ServerMessage msg;
    int bytes = recv_all(net->sockfd, &msg, sizeof(msg));
    if(bytes <= 0) return -1;
    if(msg.type == MSG_STATE){
        *state = msg.state;
        return 1;
    }
    if (msg.type == MSG_GAME_OVER) {
        return 0;
    }
    return 0;
}
