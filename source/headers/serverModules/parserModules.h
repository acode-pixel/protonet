#ifndef S_PARSER_MODULES_H
#define S_PARSER_MODULES_H
#include "server.h"
#include <sys/stat.h>

int brodParser(Packet* buf, Client* client, Server* serv);
int tracParser(Packet* buf, Client* client, Server* serv);

#endif