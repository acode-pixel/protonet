#include <stdlib.h>
#include <string.h>
#include <uv.h>
extern "C"{
    #include "core.h"
}

int main(int argc, char** argv){
    Protonet* protonet2 = Init();
    if(protonet2 != NULL){
        protonet2 = Stop();
        return 0;
    }
    return -1;
}