#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "proto/proto.hpp"


int main(int argc, char** argv){
    Protonet* protonet2 = Init();
    Client* client;
    #ifdef _WIN32
    client = new Client("Loopback Pseudo-Interface 1", "127.0.0.1");
    #else
    client = new Client("lo", "127.0.0.1");
    #endif
    protonet2->Client = client;
    failTest();
    return 0;
}