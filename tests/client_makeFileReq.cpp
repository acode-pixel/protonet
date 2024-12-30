#include <stdlib.h>
#include "core.h"
#include "client.hpp"
#include "server.hpp"
#include <uv.h>

int main(int argc, char** argv){
    Protonet* p = Init();
    Server* server;
    Client* client;

    #ifdef _WIN32
        server = new Server("Loopback Pseudo-Interface 1", "test", ".", "");
        uv_sleep(1000);
        client = new Client("Loopback Pseudo-Interface 1", "test", "127.0.0.1");
    #else
        server = new Server("lo", "test", ".", "");
        uv_sleep(1000);
        client = new Client("lo", "test", "127.0.0.1");
    #endif

    client->makeFileReq("test.txt");
    uv_sleep(2000);
    if(client->socketMode != SPTP_BROD)
        return -1;
    
    return 0;
}