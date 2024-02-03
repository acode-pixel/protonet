#include "server.h"

Server* Init(char* inter, char* ip, char* serverName, char Dir[]){
	// Check if Dir is NULL
	assert(Dir != NULL);
	if (access(Dir, R_OK) == -1){
		perror("Direcotry Not Accessible:");
		return NULL;
	}
	
	// alloc server 
	Server* serv = (Server*) malloc(sizeof(Server) + strlen(Dir));
	memset(serv, 0, sizeof(Server) + strlen(Dir));
	serv->ServerOpts.socketOpt.keepalive = 1;
	serv->ServerOpts.socketOpt.reuseaddr = 1;
	serv->Socket = socket(AF_INET, SOCK_STREAM, 0);
	// for sockets
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(S_PORT);

	// set fd as nonblocking
	fcntl(serv->Socket, F_SETFL, O_NONBLOCK, 1);

	sockaddr.sin_addr.s_addr = getInterIP(serv->Socket, inter);

	//serv->Clientlist.nClients = 0;
	serv->IP = sockaddr.sin_addr.s_addr; // src IP

	strcpy(serv->serverName, serverName);
	memcpy(serv->dir, Dir, strlen(Dir));
	
	setSockOpts(serv->Socket, &serv->ServerOpts.socketOpt, "\x01\x01\x00");

	if (bind(serv->Socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1){
		perror("Server::Init::Error BIND");
		return NULL;
	}

	serv->size = sizeof(*serv) + strlen(serv->dir);
	serv->ServerOpts.sockaddr = (struct sockaddr*)&sockaddr;
	serv->ServerOpts.socklen = sizeof(sockaddr);

	strcpy(serv->client.name, serverName);
	if (strlen(ip) > 0){
		serv->client.Socket = connectToNetwork(ip, &serv->client);
		serv->destIP = inet_addr(ip);
	}

	serv->kqueueInstance = kqueue();
	serv->lkqueueInstance = kqueue();

	struct kevent ev;
	EV_SET(&ev, serv->Socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, "LISTEN");
	kevent(serv->lkqueueInstance, &ev, 1, NULL, 0, NULL);
	if(serv->client.Socket > 0){
		EV_SET(&ev, serv->client.Socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		kevent(serv->kqueueInstance, &ev, 1, NULL, 0, NULL);
	}
	return serv;
}

int checkSockets(Server* serv, int fds[]){
	struct timespec ts;
	ts.tv_sec = 1;
       	ts.tv_nsec = 5000000;
	int nevents = kevent(serv->kqueueInstance, NULL, 0, serv->Events, 10, &ts);

	if (nevents == 0){
		return 0;
	}

	int index = 0;

	for (int l = 0; l < MAX_EVENTS; l++){
		if (serv->Events[l].filter == EVFILT_READ && serv->Events[l].flags != EV_EOF){
		fds[index] = serv->Events[l].ident;
		printf("fd: %lu, data: %li", serv->Events[l].ident, serv->Events[l].data);
		index++;		
		}
	}

	return 0;
}

int SocketManager(int fds[], Server* serv){

	Packet* buf = NULL;
	buf = (Packet*) malloc(sizeof(Packet));
	memset(buf, 0, sizeof(Packet));

	for (int i = 0; i <= sizeof(*fds)/sizeof(fds[0]); i++){

		if (fds[i] == 0){
			memset(buf, 0, sizeof(Packet)+buf->datalen);
			free(buf);
			return 0;
		}
		
		if (readPck(fds[i], buf) == 0){

			if(fds[i] == serv->client.Socket){
				if(buf->Mode == SPTP_TRAC){
					tracParser(buf, &serv->client, serv);
				}
			}

			else {
				if (buf->Mode == SPTP_BROD){
					brodParser(buf, getClient(&serv->Clientlist, fds[i], NULL), serv);
				}
				else if (buf->Mode == SPTP_TRAC){
					tracParser(buf, getClient(&serv->Clientlist, fds[i], NULL), serv);
				}
			}

		}

		fds[i] = 0;

	}

	return 0;
}


int ServerListen(Server* serv){
	struct kevent ev;
	struct timespec ts;
	ts.tv_sec = 1;
    ts.tv_nsec = 0;

	int nSockets = kevent(serv->lkqueueInstance, NULL, 0, &ev, 1, &ts);

	if (ev.filter == EVFILT_READ && ev.ident == serv->Socket){
		int fd = accept(ev.ident, serv->ServerOpts.sockaddr, &serv->ServerOpts.socklen);	
		return fd;
	}
	return 0;

}
