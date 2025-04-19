#include "client.hpp"
#include "client-wrapper.h"

void* Client_new(char* inter, char name[], char* IP, char outpath[]) {
    return new Client(inter, name, IP, outpath);
}

void Client_delete(void* client) {
    if(client == NULL){return;};
    Client* obj;
    obj = static_cast<Client*>(client);
    delete obj;
    obj = nullptr;
}