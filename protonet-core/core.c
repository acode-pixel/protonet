#include <uv.h>
#include <sodium.h>
#include "./proto/core.h"

FILE* flog;
Protonet* _p;
uv_mutex_t logLock;

void NOP(uv_timer_t *handle){
	return;
}

Protonet* Init(void){
	// put logs to file
  	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	char fname[255];
	uint data;
	uv_random(NULL, NULL, &data, sizeof(int), 0, NULL);
	sprintf(fname, "ProtoNetAPI-1.0-%d.log", data);
	log_info("Log file: %s", fname);
	flog = fopen(fname, "w");
	
	if(flog == NULL){
		perror("Failed creating log file");
		return((Protonet*)-1);
	}

	log_add_fp(flog, LOG_TRACE);
	uv_mutex_init_recursive(&logLock);
	log_set_lock(logLocker, NULL);
	log_add_callback(failCallback, NULL, LOG_FATAL);
	log_info("Started at %d:%02d:%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (sodium_init() < 0) {
       log_fatal("Failed to initialize libsodium. Quiting");
	   return NULL;
    }

	_p = malloc(sizeof(Protonet));
	_p->isUp = true;
	//_p->loop = malloc(sizeof(uv_loop_t));
	//uv_loop_init(_p->loop);
	_p->tid = uv_thread_self();
	return _p;
}

int clientServerHybrid(char* ServerInterface, char* IP){
	// WIP
}

uv_interface_address_t getInterIP(char interface_name[]){
	uv_interface_address_t* interfaces;
    int count;
	uv_interface_address_t inter;
    uv_interface_addresses(&interfaces, &count);

    for(int i = 0; i < count; i++){
        char ip[INET_ADDRSTRLEN];
        memcpy(&inter, &interfaces[i], sizeof(uv_interface_address_t));
		if(strcmp(inter.name, interface_name) == 0){
			uv_free_interface_addresses(interfaces, count);
			return inter;
		}
    } 
	uv_free_interface_addresses(interfaces, count);
	return (uv_interface_address_t){NULL};
}

int sendPck(uv_stream_t* stream_tcp, uv_write_cb write_cb, char* Name, uint8_t Mode, void* data, uint size){
	Packet* pck = NULL;
	uint pckSize =  (size == 0) ? strlen(data) : size;
	pck = (Packet*) malloc(sizeof(Packet) + pckSize);
	memset(pck, 0, sizeof(Packet) + pckSize);
	memcpy(pck->Proto, "SPTP", 4);
	strncpy(pck->Name, Name, MAX_NAMESIZE);
	pck->Mode = Mode;
	
	memcpy(pck->data, data, pckSize);
	pck->datalen = pckSize;

	uv_buf_t pckbuf[1];
	pckbuf[0] = uv_buf_init((char*)pck, sizeof(*pck) + pck->datalen);
	uv_write_t* write = (uv_write_t*)malloc(sizeof(uv_write_t)); // free later
	write->data = pck;
	// need callback from client or server
	uv_write(write, stream_tcp, pckbuf, 1, write_cb);
	return 0;
}

void failCallback(log_Event *ev){
	log_info("Shutting down API due to fatal error: %s", ev->fmt);

	fclose(flog);
	_p->isUp = false;
	//_p = NULL;
	#ifndef DEBUG
	exit(-1);
	#else
	exit(0);
	#endif
	// handle client and server close with uv
}

Protonet* Stop(void){
	// expand this to handle still running instances
	log_info("Shutting down API");
	fclose(flog);
	_p->isUp = false;
	free(_p);
	_p = NULL;
	return NULL;
}

void logLocker(bool lock, void* udata){
	if(lock)
		uv_mutex_lock(&logLock);
	else {
		uv_mutex_unlock(&logLock);
	}
}

#ifdef DEBUG
void failTest(void){
	log_fatal("Fatal test");
	return;
}
#endif
