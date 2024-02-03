#include "serverModules/clientModule.h"

int delClient(int fd, Server* serv){
	for (int i = 0; i < MAX_CLIENTS; i++){
		if (serv->Clientlist.clients[i].Socket == fd){
			serv->Clientlist.clients[i].Socket = 0;
			memset(serv->Clientlist.clients[i].name, 0, strlen(serv->Clientlist.clients[i].name));
			serv->Clientlist.clients[i].socketMode = 0;
			serv->nConn -= 1;
			return 0;
		}
	} 
	return -1;
}

int addClient(int fd, Server* serv){
	struct kevent ev;

	if (serv->nConn >= MAX_CLIENTS){
		close(fd);
		printf("Client MAX Reached\n");
		return 0;
	}


	for(int i = 0; i <= MAX_CLIENTS; i++){
		if (i > MAX_CLIENTS-1){
			return -1;
		}

		else if (serv->Clientlist.clients[i].Socket == 0){
			serv->Clientlist.clients[i].Socket = fd;
			serv->nConn += 1;
			serv->Clientlist.clients[i].socketMode = 0;
			EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
			kevent(serv->kqueueInstance, &ev, 1, NULL, 0, NULL);
			return 0;
		}
	}

	return -1;
}

Client* getClient(clientList* Clientlist, int fd, char* name){
	if (fd != 0){
		for (int i = 0; i < MAX_CLIENTS; i++){
			if (Clientlist->clients[i].Socket == fd){
				return &Clientlist->clients[i];
			}
		}
	} else if(name != NULL){
		for (int i = 0; i < MAX_CLIENTS; i++){
			if (strcmp(Clientlist->clients[i].name, name) == 0){
				return &Clientlist->clients[i];
			}
		}
	}

	return (Client*)NULL;
}
