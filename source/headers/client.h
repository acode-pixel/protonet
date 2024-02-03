#ifndef CLIENT_H
#define CLIENT_H

#include "core.h"
#define C_PORT 5657

typedef struct Client {
	char name[12];
	int Socket;
	int socketMode;
	int kqueueInstance;
	tracItem trac;
	char fileReq[255];
} Client;

#include "clientModules/parserModules.h"

Client* Cl_Init(char* inter, char name[]);

int connectToNetwork(char* IP, Client* cli);
int makeFileReq(Client* client, char File[]);
bool clientCheckSocket(Client* client);
#endif
