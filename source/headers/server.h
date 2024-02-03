#ifndef SERVER_H
#define SERVER_H
#define S_PORT 5657
#define MAX_CLIENTS 10
#define MAX_EVENTS 10

#include "core.h"
#include "client.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/time.h>
#include <fcntl.h>

typedef struct serverOpts {
	SocketOpt socketOpt;
	struct sockaddr* sockaddr;
	socklen_t socklen;
} serverOpts;

typedef struct clientList {
	Client clients[MAX_CLIENTS];
} clientList;

typedef struct tracList {
	tracItem tracs[MAX_CLIENTS];
} tracList;

typedef struct Server {
	int Socket;		/* Socket */
	int kqueueInstance;
	int lkqueueInstance;
	int nConn;
	uint32_t IP;		/* host IP */
	uint32_t destIP;	/* Destination IP */
	size_t size;		/* Server struct Size */
	char serverName[12];	/* Server Name */ 
	serverOpts ServerOpts;	/* Server Options */
	struct kevent Events[MAX_CLIENTS];
	Client client;		/* for client-server hybrid */
	clientList Clientlist;
	tracList Traclist;
	char dir[];	/* Server Dir */
} Server;

#include "serverModules/clientModule.h"
#include "serverModules/parserModules.h"
#include "serverModules/tracModule.h"
#include "serverModules/dataModule.h"
#include "clientModules/parserModules.h"

Server* Init(char* inter, char* ip, char* serverName, char Dir[]);
int checkSockets(Server* serv, int fds[]);
int SocketManager(int fds[], Server* serv);
int ServerListen(Server* serv);

#endif
