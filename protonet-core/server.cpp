#include "server.hpp"

Server::Server(char* inter, char* serverName, char Dir[], uv_loop_t* loop){
	if (access(Dir, R_OK) == -1){
		log_error("Directory %s not accessible", Dir);
	}
	
	// alloc server 
	//Server* serv = (Server*) malloc(sizeof(Server) + strlen(Dir)+1);
	//memset(serv, 0, sizeof(Server) + strlen(Dir));

	this->Socket = malloc(sizeof(uv_tcp_t*));

	//log_info("Server Socket: %i", serv->Socket);
	// for sockets
	/*struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(S_PORT);*/

	// set fd as nonblocking
	//fcntl(serv->Socket, F_SETFL, O_NONBLOCK, 1);

	uv_interface_address_t addr = getInterIP(inter);

	this->IP = &addr.address.address4.sin_addr; // src IP
	log_info("Server IP: %s", inet_ntoa(*((struct in_addr*)&serv->IP)));

	strcpy(serv->serverName, serverName);
	log_info("Server name: %s", serv->serverName);
	memcpy(serv->dir, Dir, strlen(Dir));
	log_info("Server dir: %s", serv->dir);
	
	//setSockOpts(serv->Socket, &serv->ServerOpts.socketOpt, "\x01\x01\x00");
    uv_tcp_init( )
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