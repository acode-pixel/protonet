#include "server.hpp"
#include "server-wrapper.h"

void* Server_new(char* inter, char* serverName, char Dir[], char* peerIp, uv_loop_t* loop) {
    return new Server(inter, serverName, Dir, peerIp, loop);
}

void Server_delete(void* server) {
    Server* obj;
    obj = static_cast<Server*>(server);
    delete obj;
    obj = nullptr;
}