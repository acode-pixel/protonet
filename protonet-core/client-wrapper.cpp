#include "client.hpp"
#include "client-wrapper.h"

void* Client_new(char* inter, char name[], uv_loop_t* loop) {
    return new Client(inter, name, loop);
}

void Client_delete(void* client) {
    Client* obj;
    obj = static_cast<Client*>(client);
    delete obj;
    obj = nullptr;
}