#include <stdlib.h>
#include <string.h>
#include <uv.h>
extern "C" {
    #include "core.h"
}

#ifdef __unix__
    #define INTER "lo0"
#else
    #define INTER "WiFi"
#endif

int main(int argc, char** argv){
    uv_interface_address_t inter = getInterIP(INTER);
    char ip[INET_ADDRSTRLEN];
    int r = uv_ip4_name(&inter.address.address4, ip, INET_ADDRSTRLEN);
    if(r == 0){
        return 0;
    }
    return -1;
}