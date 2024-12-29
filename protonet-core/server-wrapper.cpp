#include "server.hpp"
#include "server-wrapper.h"

void* Server_new(char* inter, char* serverName, char Dir[], char* peerIp) {
    return new Server(inter, serverName, Dir, peerIp);
}

void Server_delete(void* server) {
    if (server == NULL){return;};
    Server* obj;
    obj = static_cast<Server*>(server);
    delete obj;
    obj = nullptr;
}