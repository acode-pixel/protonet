#include "core.h"

FILE* flog;

int Init(void){
	// put logs to file
  	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	char fname[255];
	sprintf(fname, "ProtoNetAPI-0.1_%d:%02d:%02d_%02d:%02d:%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	flog = fopen(fname, "w");
	
	if(flog == NULL){
		perror("Failed creating log file");
		exit(-1);
	}

	log_add_fp(flog, LOG_DEBUG);
	log_info("Started at %d:%02d:%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	return 0;
}

int setSockOpts(int sock, SocketOpt* so, char opts[]){
	so->reuseaddr = opts[0];
	so->keepalive = opts[1];
	so->dontroute = opts[2];

	log_debug("Setting Socket %i options: RA:%i KA:%i DR:%i", sock, opts[0], opts[1], opts[2]);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opts[0], sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opts[1], sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, &opts[2], sizeof(int));
	
	return 0;
}

uint32_t getInterIP(int fd,char inter[]){
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, inter, IFNAMSIZ-1);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == -1){
		perror("Failed tyo get IP of inter");
		return 0;
	} 
	return ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
}

int sendPck(int fd, char* Name, uint8_t Mode, void* data, uint size){
	Packet* pck = NULL;
	uint pckSize =  (size == 0) ? strlen(data) : size;
	pck = (Packet*) malloc(sizeof(Packet) + pckSize+1);
	memset(pck, 0, sizeof(Packet) + pckSize+1);
	memcpy(pck->Proto, "SPTP", 4);
	memcpy(pck->Name, Name, 12);
	pck->Mode = Mode;

	if (pckSize > 1024){
		errno = 84;
		perror("Pck creation error");
		free(pck);
		return -1;
	}
	
	memcpy(pck->data, data, pckSize);
	pck->datalen = pckSize+1;

	if (send(fd, pck, sizeof(*pck) + pckSize+1, 0) == -1){
		perror("Falied to send Pck");
		free(pck);
		return -1;
	}

	free(pck);
	return 0;
}

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
	log_info("Shutting down API due to fatal error");
	fclose(flog);
	exit(-1);
}

void Stop(void){
	log_info("Shutting down API");
	fclose(flog);
}