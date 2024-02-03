#ifndef S_TRAC_MODULES_H
#define S_TRAC_MODULES_H
#include "server.h"

int addTracItem(tracList* Traclist, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, void* fileOffset, char* fileReq);
tracItem* getTracItem(tracList* Traclist, char* name, uint tracID);
int tracSpread(clientList* Clientlist, Packet* buf, Server* serv);
int IdManager(tracList* traclist);

#endif