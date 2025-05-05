#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "proto/proto.hpp"

#ifdef __unix__
    #define INTER "lo"
#else
    #define INTER "Loopback Pseudo-Interface 1"
#endif

int main(int argc, char** argv){
    uv_interface_address_t inter = getInterIP(INTER);
    char ip[INET_ADDRSTRLEN];
    uv_ip4_name(&inter.address.address4, ip, INET_ADDRSTRLEN);
    printf("IP: %s", ip);
    if(strcmp(ip, "127.0.0.1") == 0 || strcmp(ip, "0.0.0.0") == 0){
        return 0;
    }
    return -1;
}