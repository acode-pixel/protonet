#include "clientModules/parserModules.h"

int C_tracParser(Packet* buf, Client* client){
    if(client->trac.tracID != 0 && strcmp(buf->Name, client->name) != 0){
	    return 0;
	}
				
    // Create client Parser modules
	struct DATA* trac = malloc(sizeof(struct DATA));
	trac->tracID = ((struct TRAC*)buf->data)->tracID;
    strcpy((char*)trac->data, "OK");

	fillTracItem(&client->trac, trac->tracID, client->name, ((struct TRAC*)buf->data)->hops, ((struct TRAC*)buf->data)->lifetime, NULL, client->fileReq);
	sendPck(client->Socket, client->name, SPTP_TRAC, trac, 0);
	free(trac);
    return 0;
}

int C_brodParser(Packet* buf, Client* client){
    if(strcmp((char*)buf->data, "NO_FILE") == 0){
        char* data = "LEAVE";
        sendPck(client->Socket, client->name, SPTP_BROD, data, 0);
        client->trac.canDelete = 1;
        return 0;
    }

    if(strcmp((char*)buf->data, "LEAVE_OK") == 0 && client->trac.canDelete == 1){
        client->trac.deleted = 1;
        return 0;
    }

    return -1;
}