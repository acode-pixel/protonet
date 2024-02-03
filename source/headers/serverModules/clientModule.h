#ifndef S_CLIENT_MODULES_H
#define S_CLIENT_MODULES_H
#include "server.h"

int addClient(int fd, Server* serv);
Client* getClient(clientList* Clientlist, int fd, char* name);
int delClient(int fd, Server* serv);

#endif