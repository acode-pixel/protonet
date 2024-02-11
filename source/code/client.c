#include "client.h"

Client* Cl_Init(char* inter, char name[]){
	log_add_callback(failCallback, NULL, 5);

	Client* cli = (Client*) malloc(sizeof(Client));
	cli->Socket = socket(AF_INET, SOCK_STREAM, 0);
	log_info("Client Socket: %i", cli->Socket);

	cli->kqueueInstance = kqueue();
	log_debug("Client kqueue: %i", cli->kqueueInstance);

	struct in_addr addr;
	addr.s_addr = getInterIP(cli->Socket, inter);
	log_info("Client IP: %s", inet_ntoa(addr));

	if (name == NULL){
		strcpy(cli->name, (char*)(inet_ntoa(addr)));
	} 
	else {strcpy(cli->name, name);}
	log_info("Client name: %s", cli->name);

	if(cli != NULL && cli->Socket != 0 && cli->kqueueInstance != 0 && cli->name != NULL){
		log_info("Successfully created Client");
		return cli;
	} else {
		free(cli);
		log_error("An error occured while creating Client");
		return NULL;
	}
}

int connectToNetwork(char* IP, Client* cli){
	int tcpSocket = (cli->Socket == 0) ? socket(AF_INET, SOCK_STREAM, 0) : cli->Socket;

	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(C_PORT);
	inet_aton(IP, &sockaddr.sin_addr);
	
	if (connect(tcpSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1){
		return -1;
	}
	log_info("Connected to %s", IP);

	fcntl(tcpSocket, F_SETFL, O_NONBLOCK, 1);

	struct kevent ev;
	EV_SET(&ev, cli->Socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)"CLIENT");
	kevent(cli->kqueueInstance, &ev, 1, NULL, 0, NULL);
	return tcpSocket;
}

int makeFileReq(Client* client, char File[]){
	if(strlen(File) > 255){
		printf("File name too long");
		return -1;
	}
	struct BROD* br = (struct BROD*)malloc(sizeof(struct BROD) + strlen(File)+1);
	br->hops = 0x01;
	strcpy(br->fileReq, File);
	strcpy(client->fileReq, File);
	assert(sendPck(client->Socket, client->name, 1, br, 0) == 0);
	fillTracItem(&client->trac, 0, client->name, 0, 0, NULL, client->name);
	free(br);
	return 0;

}

bool clientCheckSocket(Client* client){
	struct timespec ts = {1, 0};

	int nSockets = kevent(client->kqueueInstance, NULL, 0, NULL, 1, &ts);

	if (nSockets == 0){
		return false;
	} else {return true;}
}
