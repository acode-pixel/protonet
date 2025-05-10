#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

int main(int argc, char** argv){
    Protonet* p = Init();
    Client* client;
    Server* server;
    #ifdef _WIN32
    // char* inter, char Dir[], int port, char* serverName, char* peerIp
    server = new Server("Loopback Pseudo-Interface 1", ".", S_PORT);
    uv_sleep(1000);
    client = new Client("Loopback Pseudo-Interface 1", "127.0.0.1");
    #else
    server = new Server("lo", ".", S_PORT);
    uv_sleep(1000);
    client = new Client("lo", "127.0.0.1");
    #endif

    if(client->socket != NULL){
        return 0;
    }
    return -1;
    
}