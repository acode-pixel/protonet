#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

int main(int argc, char** argv){
    Protonet* p = Init();
    Client* client;
    Server* server;
    #ifdef _WIN32
    server = new Server("Loopback Pseudo-Interface 1", "test", ".", "");
    uv_sleep(1000);
    client = new Client("Loopback Pseudo-Interface 1", "test", "127.0.0.1", ".");
    #else
    server = new Server("lo", "test", ".", "");
    uv_sleep(1000);
    client = new Client("lo", "test", "127.0.0.1", ".");
    #endif

    if(client->socket != NULL){
        return 0;
    }
    return -1;
    
}