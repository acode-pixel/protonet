#ifndef SERVER_H
#define SERVER_H
#define MAX_CLIENTS 10
#define MAX_EVENTS 10

extern "C" {
	#include "core.h"
	#include "log.h"
}
#include "client.hpp"

typedef struct clientList {
	Client* clients[MAX_CLIENTS];
} clientList;

typedef struct tracList {
	tracItem tracs[MAX_CLIENTS];
} tracList;

class Server {
    public:
	    uv_stream_t Socket;		/* Socket */
		uv_loop_t* loop;
	    int nConn;
	    char IP[INET_ADDRSTRLEN];		/* host IP */
	    char destIP[INET_ADDRSTRLEN];	/* Destination IP */
	    //size_t size;		/* Server struct Size */
	    char serverName[12];	/* Server Name */ 
	    //serverOpts ServerOpts;	/* Server Options */
	    //struct kevent Events[MAX_CLIENTS];
	    Client* client;		/* for client-server hybrid */
	    clientList Clientlist;
	    tracList Traclist;
	    char dir[];	/* Server Dir */

		MYLIB_API Server(char* inter, char* serverName, char Dir[], char* peerIp, uv_loop_t* loop);


};
/*
#include "serverModules/clientModule.h"
#include "serverModules/parserModules.h"
#include "serverModules/tracModule.h"
#include "serverModules/dataModule.h"
#include "clientModules/parserModules.h"
*/
#endif