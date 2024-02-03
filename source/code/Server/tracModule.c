#include "serverModules/tracModule.h"

int addTracItem(tracList* Traclist, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, void* fileOffset, char* fileReq){
	for (int i = 0; i < MAX_CLIENTS; i++){
		if (Traclist->tracs[i].tracID != 0){
			continue;
		}

		fillTracItem(&Traclist->tracs[i], tracID, fileRequester, hops, lifetime, fileOffset, fileReq);
		return 0;
	}

	printf("Maximum transactions reached\n");
	return -1;
}

tracItem* getTracItem(tracList* Traclist, char* name, uint tracID){
	if (name != NULL){
		for(int i = 0; i < MAX_CLIENTS; i++){
			if(strcmp(Traclist->tracs[i].fileRequester, name) == 0){
				return &Traclist->tracs[i];
			}
		}
	} else if (tracID != 0){
		for(int i = 0; i < MAX_CLIENTS; i++){
			if(tracID == Traclist->tracs[i].tracID){
				return &Traclist->tracs[i];
			}
		}
	}

	return (tracItem*)NULL;
}

int tracSpread(clientList* Clientlist, Packet* buf, Server* serv){
	struct TRAC* trac = (struct TRAC*) malloc(sizeof(struct TRAC));
	trac->hops = ((struct BROD*)buf->data)->hops;
	trac->lifetime = 255-trac->hops;
	strcpy(trac->Name, buf->Name);
	srand(time(NULL)+buf->datalen);
	trac->tracID = (buf->Mode == SPTP_BROD) ? rand() : ((struct TRAC*)buf->data)->tracID;


	for (int i = 0; i < MAX_CLIENTS; i++){
		if (Clientlist->clients[i].Socket == 0){
			continue;
		}

		trac->tracID *= Clientlist->clients[i].Socket^2;
		sendPck(Clientlist->clients[i].Socket, serv->serverName, SPTP_TRAC, trac, 0);

	}

	free(trac);
	addTracItem(&serv->Traclist, trac->tracID, trac->Name, trac->hops, trac->lifetime, NULL, ((struct BROD*)buf->data)->fileReq);

	return 0;
}

int IdManager(tracList* traclist){
	printf("tracID|deleted|client|socket|hops|lifetime|fileOffset|confirmed|canDelete|File Request\n");
	printf("========================================================================\n");

	for (int i = 0; i < MAX_CLIENTS; i++){
		if(traclist->tracs[i].tracID != 0){
			printf("%u|", traclist->tracs[i].tracID);
			printf("%x|", traclist->tracs[i].deleted);
			printf("%s|", traclist->tracs[i].fileRequester);
			printf("%i|", traclist->tracs[i].Socket);
			printf("%i|", traclist->tracs[i].hops);
			printf("%i|", traclist->tracs[i].lifetime);
			printf("%p|", traclist->tracs[i].fileOffset);
			printf("%x|", traclist->tracs[i].confirmed);
			printf("%x|", traclist->tracs[i].canDelete);
			printf("%s\n", traclist->tracs[i].fileReq);

			if(traclist->tracs[i].confirmed == 0){
				if(traclist->tracs[i].lifetime == 0){
					fillTracItem(&traclist->tracs[i], 0, "", 0, 0, NULL, "");
				}
				else {traclist->tracs[i].lifetime -= 1;}
			}
		}
	}
	return 0;
}