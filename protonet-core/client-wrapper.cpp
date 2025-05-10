#include <uv.h>
#include "./proto/client.hpp"
#include "./proto/client-wrapper.h"

void* Client_new(char* inter, char* IP, int serverPort, char name[], char outpath[]) {
    return new Client(inter, IP, serverPort, name, outpath);
}

void Client_delete(void* client) {
    if(client == NULL){return;};
    Client* obj;
    obj = static_cast<Client*>(client);
    delete obj;
    obj = nullptr;
}