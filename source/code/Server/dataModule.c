#include "serverModules/dataModule.h"

int DataManager(tracList* Traclist, Server* serv){
	char filepath[strlen(serv->dir)+255];
	struct DATA* data = malloc(sizeof(struct DATA));
	int sizeRead;
	strcpy(filepath, serv->dir);
	
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(Traclist->tracs[i].tracID == 0 || Traclist->tracs[i].confirmed == 0){
			continue;
		}

		if(Traclist->tracs[i].fd == 0){
			strcat(filepath, Traclist->tracs[i].fileReq);
			Traclist->tracs[i].fd = open(filepath, O_RDONLY);
			memset(&filepath[strlen(serv->dir)+1], 0, strlen(Traclist->tracs[i].fileReq));
		}

		if((int)Traclist->tracs[i].fileOffset != Traclist->tracs[i].fileSize && Traclist->tracs[i].socketStatus == 0){
			data->tracID = Traclist->tracs[i].tracID;
			sizeRead = read(Traclist->tracs[i].fd, data->data, 1020);
			sendPck(Traclist->tracs[i].Socket, serv->serverName, SPTP_DATA, data, sizeRead);
			Traclist->tracs[i].socketStatus = 1;
		}


	}

	return 0;
}