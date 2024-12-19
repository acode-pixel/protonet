#include "core.h"

FILE* flog;
Protonet* _p;

Protonet* Init(void){
	// put logs to file
  	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	char fname[255];
	sprintf(fname, "ProtoNetAPI-0.1_%d-%02d-%02d_%02d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	log_info("Log file: %s", fname);
	flog = fopen(fname, "w");
	
	if(flog == NULL){
		perror("Failed creating log file");
		return((Protonet*)-1);
	}

	log_add_fp(flog, LOG_INFO);
	log_add_callback(failCallback, NULL, LOG_FATAL);
	log_info("Started at %d:%02d:%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	_p = malloc(sizeof(Protonet));
	_p->isUp = true;
	_p->loop = malloc(sizeof(uv_loop_t));
	_p->Client = NULL;
	_p->Server = NULL;
	uv_loop_init(_p->loop);
	return _p;
}

uv_interface_address_t getInterIP(char interface_name[]){
	/*struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, inter, IFNAMSIZ-1);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == -1){
		perror("Failed tyo get IP of inter");
		return 0;
	} 
	return ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;*/
	uv_interface_address_t* interfaces;
    int count;
    uv_interface_addresses(&interfaces, &count);

    for(int i = 0; i < count; i++){
        char ip[INET_ADDRSTRLEN];
        uv_interface_address_t inter = interfaces[i];

        if(inter.is_internal){
            continue;
        }

		uv_inet_ntop(AF_INET, &inter.address.address4.sin_addr, ip, INET_ADDRSTRLEN);
		if(strcmp(inter.name, interface_name) == 0 && strcmp(ip, "0.0.0.0") != 0){
			return inter;
		}
    }
	return (uv_interface_address_t){NULL};
}

int sendPck(/*int fd*/ uv_stream_t* stream_tcp, uv_write_cb write_cb, char* Name, uint8_t Mode, void* data, uint size){
	Packet* pck = NULL;
	uint pckSize =  (size == 0) ? strlen(data) : size;
	pck = (Packet*) malloc(sizeof(Packet) + pckSize+1);
	memset(pck, 0, sizeof(Packet) + pckSize+1);
	memcpy(pck->Proto, "SPTP", 4);
	memcpy(pck->Name, Name, 12);
	pck->Mode = Mode;

	if (pckSize > 1024){
		errno = 84;
		log_fatal("Failed creating packet: %s", strerror(errno));
		free(pck);
		return -1;
	}
	
	memcpy(pck->data, data, pckSize);
	pck->datalen = pckSize+1;

	/*if (send(fd, pck, sizeof(*pck) + pckSize+1, 0) == -1){
		perror("Falied to send Pck");
		free(pck);
		return -1;
	}*/

	uv_buf_t pckbuf[1];
	pckbuf[0] = uv_buf_init((char*)pck, sizeof(*pck) + pck->datalen);
	uv_write_t write;
	// need callback from client or server
	uv_write(&write, stream_tcp, pckbuf, 1, write_cb);
	uv_run(_p->loop, UV_RUN_ONCE);
	/*free(pck);*/
	return 0;
}

// REMOVE THIS, client and server should have their own response from uv_read
/*
int readPck(int fd, Packet* buf){

	if (recv(fd, buf, sizeof(*buf), 0) == -1){
		perror("read Failed:");
		return errno;
	}

	if (strlen(buf->Proto) == 0){
		return -1;
	}

	buf = realloc(buf, sizeof(Packet)+buf->datalen);
	read(fd, buf->data, buf->datalen);

	return 0;
} 
*/
int fillTracItem(tracItem* trac, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, void* fileOffset, char* fileReq){
	trac->tracID = tracID;
	trac->hops = hops;
	trac->lifetime = lifetime;
	trac->fileOffset = fileOffset;
	strcpy(trac->fileReq, fileReq);
	strcpy(trac->fileRequester, fileRequester);
	return 0;
}

void failCallback(log_Event *ev){
	log_info("Shutting down API due to fatal error: %s", ev->fmt);

	fclose(flog);
	_p->isUp = false;
	Client_delete(_p->Client);
	Server_delete(_p->Server);
	//_p = NULL;
	#ifndef DEBUG
	exit(-1);
	#else
	exit(0);
	#endif
	// handle client and server close with uv
}

Protonet* Stop(void){
	log_info("Shutting down API");
	fclose(flog);
	_p->isUp = false;
	free(_p);
	_p = NULL;
	return NULL;
}

#ifdef DEBUG
void failTest(void){
	log_fatal("Fatal test");
	return;
}
#endif