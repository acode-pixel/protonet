#ifdef _WIN32
    #ifdef MYLIB_EXPORTS
        #define MYLIB_API __declspec(dllexport)
    #else
        #define MYLIB_API __declspec(dllimport)
    #endif
#else
    #define MYLIB_API
#endif

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif
// make long-live tracItems for use
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "./log.h"
#include "./client-wrapper.h"
#include "./server-wrapper.h"
#include <uv.h>

#define SPTP_BROD 1
#define SPTP_TRAC 2
#define SPTP_DATA 3

#define C_PORT 5657
#define S_PORT 5657
#define MAX_NAMESIZE 16
typedef struct Packet {
	char Proto[4];
	char Name[MAX_NAMESIZE];
	uint8_t Mode;
	uint32_t datalen;

	uint8_t data[];
} Packet;

#define MAX_DATASIZE 60000
#define MAX_FILESIZE sizeof(uint)*MAX_DATASIZE

typedef unsigned int uint;

struct BROD {
	uint8_t hops;
	char fileReq[];
};

struct DATA {
	uint tracID;
	uint id;
	char data[MAX_DATASIZE];
};

struct TRAC {
	uint tracID;
	uint8_t hops;
	uint8_t lifetime;
	size_t fileSize;
	char Name[MAX_NAMESIZE]; // Name of the file requester
};

typedef struct tracItem {
	uint tracID; 		// transaction ID
	uint8_t deleted;	// transaction is deleted
	char fileRequester[MAX_NAMESIZE]; // Name of file requester
	uv_tcp_t* Socket; // Socket to file requester
	uint socketStatus; // status of socket 
	uv_file file; // fd to requested file (if trac is confirmed)
	uint fileSize; // size of file requested
	uint8_t hops; 		// hops between client and server from initial BROD packet
	uint8_t lifetime; 	// calculated lifetime of packet from hops
	uint fileOffset; 	// current file offset
	bool confirmed; 	// if transaction id is confirmed
	bool complete;		// if transaction is complete
	bool canDelete;	// if transaction can be deleted
	char fileReq[255]; 	// file requested
	uint8_t hash[32];
	bool readAgain;
	uint readExtra; // if packet is split
	uint total_transmitted; 
	uint total_received;

} tracItem;

typedef struct Protonet {
	bool isUp;
	bool clientServerHybrid;
	//uv_loop_t* loop;
	uv_thread_t tid;
	void* Client; // Client var
	void* Server; // Server var
} Protonet;

void proto_setClient(void* Client);
void* proto_getClient();
void proto_setServer(void* Server);
void* proto_getServer();
void NOP(uv_timer_t *handle);

#ifdef DEBUG
MYLIB_API uv_interface_address_t getInterIP(char interface_name[]);
MYLIB_API int sendPck(/*int fd*/ uv_stream_t* stream_tcp, uv_write_cb write_cb, char* Name, uint8_t Mode, void* data, uint size);
/*int readPck(int fd, Packet* buf);*/
//MYLIB_API int fillTracItem(tracItem* trac, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, int fileOffset, char* fileReq);
MYLIB_API void failCallback(log_Event *ev);
MYLIB_API void logLocker(bool lock, void* udata);
MYLIB_API Protonet* Init(void);
MYLIB_API Protonet* Stop(void);
MYLIB_API void failTest(void);
#else
uv_interface_address_t getInterIP(char interface_name[]);
int sendPck(/*int fd*/ uv_stream_t* stream_tcp, uv_write_cb write_cb, char* Name, uint8_t Mode, void* data, uint size);
/*int readPck(int fd, Packet* buf);*/
//int fillTracItem(tracItem* trac, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, int fileOffset, char* fileReq);
void failCallback(log_Event *ev);
void logLocker(bool lock, void* udata);
MYLIB_API Protonet* Init(void);
MYLIB_API Protonet* Stop(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
