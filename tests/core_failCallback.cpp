#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "proto/proto.hpp"


int main(int argc, char** argv){
    Protonet* protonet2 = Init();
    Client* client;
    #ifdef _WIN32
    client = new Client("WiFi", "test", "0.0.0.0", ".");
    #else
    client = new Client("lo", "test", "0.0.0.0", ".");
    #endif
    protonet2->Client = client;
    failTest();
    return 0;
}