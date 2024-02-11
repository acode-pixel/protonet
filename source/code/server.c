#include "server.h"

Server* S_Init(char* inter, char* ip, char* serverName, char Dir[]){
	log_add_callback(failCallback, NULL, 5);

	if (access(Dir, R_OK) == -1){
		log_error("Directory %s not accessible", Dir);
		return NULL;
	}
	
	// alloc server 
	Server* serv = (Server*) malloc(sizeof(Server) + strlen(Dir)+1);
	memset(serv, 0, sizeof(Server) + strlen(Dir));

	serv->Socket = socket(AF_INET, SOCK_STREAM, 0);
	log_info("Server Socket: %i", serv->Socket);
	// for sockets
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(S_PORT);

	// set fd as nonblocking
	fcntl(serv->Socket, F_SETFL, O_NONBLOCK, 1);

	sockaddr.sin_addr.s_addr = getInterIP(serv->Socket, inter);

	serv->IP = sockaddr.sin_addr.s_addr; // src IP
	log_info("Server IP: %s", inet_ntoa(*((struct in_addr*)&serv->IP)));

	strcpy(serv->serverName, serverName);
	log_info("Server name: %s", serv->serverName);
	memcpy(serv->dir, Dir, strlen(Dir));
	log_info("Server dir: %s", serv->dir);
	
	setSockOpts(serv->Socket, &serv->ServerOpts.socketOpt, "\x01\x01\x00");

	if (bind(serv->Socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1){
		log_error("Failed binding to Socket: %i", serv->Socket);
		free(serv);
		return NULL;
	}

	serv->size = sizeof(*serv) + strlen(serv->dir);
	serv->ServerOpts.sockaddr = (struct sockaddr*)&sockaddr;
	serv->ServerOpts.socklen = sizeof(sockaddr);

	strcpy(serv->client.name, serverName);

	if (strlen(ip) > 0){
		serv->client.Socket = connectToNetwork(ip, &serv->client);
		serv->destIP = inet_addr(ip);
		log_info("Server's client socket: %i", serv->client.Socket);
	}

	serv->kqueueInstance = kqueue();
	log_debug("Server kqueue: %i", serv->kqueueInstance);
	serv->lkqueueInstance = kqueue();
	log_debug("Server lkqueue: %i", serv->lkqueueInstance);

	struct kevent ev;
	EV_SET(&ev, serv->Socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, "LISTEN");
	kevent(serv->lkqueueInstance, &ev, 1, NULL, 0, NULL);

	if(serv->client.Socket > 0){
		EV_SET(&ev, serv->client.Socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		kevent(serv->kqueueInstance, &ev, 1, NULL, 0, NULL);
	}

	if(serv->Socket != 0 && serv->kqueueInstance != 0 && serv->lkqueueInstance != 0 && serv->IP != 0 && serv->serverName != NULL){
		log_info("Successfully created Server");
		return serv;
	} else {
			free(serv);
			log_error("An error occured while creating Server");
			return NULL;
		}
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
