#include <stdlib.h>
#include "core.h"
#include "client.hpp"

int main(int argc, char** argv){
    Protonet* p = Init();
    #ifdef _WIN32
    client = new Client("Loopback Pseudo-Interface 1", "test", protonet2->loop);
    #else
    client = new Client("lo", "test", protonet2->loop);
    #endif
    
}