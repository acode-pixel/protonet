#ifndef SERVER_H
#define SERVER_H
#define S_PORT 5657
#define MAX_CLIENTS 10
#define MAX_EVENTS 10

#include "core.h"
#include "client.hpp"
/*
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
*/
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

class Server {
    public:
	    uv_stream_t Socket;		/* Socket */
		uv_loop_t* loop;
	    int nConn;
	    uint32_t IP;		/* host IP */
	    uint32_t destIP;	/* Destination IP */
	    //size_t size;		/* Server struct Size */
	    char serverName[12];	/* Server Name */ 
	    //serverOpts ServerOpts;	/* Server Options */
	    //struct kevent Events[MAX_CLIENTS];
	    Client client;		/* for client-server hybrid */
	    clientList Clientlist;
	    tracList Traclist;
	    char dir[];	/* Server Dir */

		MYLIB_API Server(char* inter, char* serverName, char Dir[], uv_loop_t* loop);


};
/*
#include "serverModules/clientModule.h"
#include "serverModules/parserModules.h"
#include "serverModules/tracModule.h"
#include "serverModules/dataModule.h"
#include "clientModules/parserModules.h"
*/
#endif