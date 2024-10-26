#include <stdlib.h>
#include <string.h>
#include "client.hpp"

extern "C" {
    #include "core.h"
}

int main(int argc, char** argv){
    Protonet* protonet2 = Init();
    Client* client;
    #ifdef _WIN32
    client = new Client("WiFi", "test", protonet2->loop);
    #else
    client = new Client("lo0", "test", protonet2->loop);
    #endif
    protonet2->Client = client;
    failTest();
    return 0;
}