#include "client.hpp"
#include "client-wrapper.h"

void* Client_new(char* inter, char name[], char* IP) {
    return new Client(inter, name, IP);
}

void Client_delete(void* client) {
    if(client == NULL){return;};
    Client* obj;
    obj = static_cast<Client*>(client);
    delete obj;
    obj = nullptr;
}