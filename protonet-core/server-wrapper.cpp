#include <uv.h>
#include "./proto/server.hpp"
#include "./proto/server-wrapper.h"

void* Server_new(char* inter, char Dir[], int port, char* serverName, char* peerIp, int peerPort) {
    return new Server(inter, Dir, port, serverName, peerIp, peerPort);
}

void Server_delete(void* server) {
    if (server == NULL){return;};
    Server* obj;
    obj = static_cast<Server*>(server);
    delete obj;
    obj = nullptr;
}