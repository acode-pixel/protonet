#include "serverModules/parserModules.h"

int brodParser(Packet* buf, Client* client, Server* serv){

    struct stat st;
    char* data;

    if (strcmp((char*)buf->data, "LEAVE") == 0){
        data = "LEAVE_OK";
        sendPck(client->Socket, serv->serverName, SPTP_BROD, data, 0);
        close(client->Socket);
		delClient(client->Socket, serv);
        return 0;
    }

	char* fileReq = ((struct BROD*)buf->data)->fileReq;
	char filepath[strlen(serv->dir)+strlen(fileReq)];
	strcpy(filepath, serv->dir);
	strcat(filepath, fileReq);
    stat(filepath, &st);

	if(access(filepath, R_OK) == -1 || st.st_size > 1024000000){
		if(serv->client.Socket != 0){
			((struct BROD*)buf->data)->hops += 1;
			sendPck(serv->client.Socket, buf->Name, SPTP_BROD, buf->data, 0);
		}
	} else {
		client->socketMode = SPTP_BROD;
		tracSpread(&serv->Clientlist, buf, serv);
		getTracItem(&serv->Traclist, buf->Name, 0)->fileSize = st.st_size;
	}
	return 0;
}

int tracParser(Packet* buf, Client* client, Server* serv){
	if (serv->client.Socket == client->Socket){
		if (strcmp(client->name, ((struct TRAC*)buf->data)->Name) != 0){
			
			addTracItem(&serv->Traclist, ((struct TRAC*)buf->data)->tracID, "", ((struct TRAC*)buf->data)->hops, ((struct TRAC*)buf->data)->lifetime, NULL, "");
			((struct TRAC*)buf->data)->lifetime -= 1;

			for (int i = 0; i < MAX_CLIENTS; i++){
				if (serv->Clientlist.clients[i].Socket == 0){
					continue;
				}

				sendPck(serv->Clientlist.clients[i].Socket, buf->Name, SPTP_TRAC, buf->data, 0);
			}
		} 

		else {
			struct DATA* trac = malloc(sizeof(struct DATA));
			trac->tracID = ((struct TRAC*)buf->data)->tracID;
    		strcpy((char*)trac->data, "OK");

			fillTracItem(&client->trac, trac->tracID, client->name, ((struct TRAC*)buf->data)->hops, ((struct TRAC*)buf->data)->lifetime, NULL, client->fileReq);
			sendPck(client->Socket, client->name, SPTP_TRAC, trac, 0);
			free(trac);
		}
	}

	else if(strcmp((char*)((struct DATA*)buf->data)->data, "OK") == 0){
		if(strcmp(getTracItem(&serv->Traclist, NULL, ((struct DATA*)buf->data)->tracID)->fileRequester, buf->Name) == 0){
			tracItem* trac = getTracItem(&serv->Traclist, NULL, ((struct DATA*)buf->data)->tracID);
			trac->confirmed = 1;
			trac->Socket = client->Socket;
		} else {
			tracItem* trac = getTracItem(&serv->Traclist, NULL, ((struct DATA*)buf->data)->tracID);
			trac->confirmed = 1;
			trac->Socket = client->Socket;
			sendPck(serv->client.Socket, buf->Name, SPTP_TRAC, buf->data, 0);
		}
	} 
	return 0;
}