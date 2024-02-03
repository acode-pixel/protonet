#ifndef C_PARSER_MODULES_H
#define C_PARSER_MODULES_H

#include "client.h"

int C_tracParser(Packet* buf, Client* client);
int C_brodParser(Packet* buf, Client* client);
#endif