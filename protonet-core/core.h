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

typedef unsigned int uint;

struct BROD {
	uint8_t hops;
	char fileReq[];
};

struct DATA {
	uint tracID;
	uint8_t data[1020];
};

struct TRAC {
	uint tracID;
	uint8_t hops;
	uint8_t lifetime;
	char Name[12];
};

typedef struct tracItem {
	uint tracID; 		// transaction ID
	uint8_t deleted;	// transaction is deleted
	char fileRequester[12]; // Name of file requester
	uv_tcp_t* Socket; // Socket to file requester
	uint socketStatus; // status of socket 
	uv_file* file; // fd to requested file (if trac is confirmed)
	uint fileSize; // size of file requested
	uint8_t hops; 		// hops between client and server from initial BROD packet
	uint8_t lifetime; 	// calculated lifetime of packet from hops
	void* fileOffset; 	// current file offset
	uint8_t confirmed; 	// if transaction id is confirmed
	uint8_t canDelete;	// if transaction can be deleted
	char fileReq[255]; 	// file requested

} tracItem;

typedef struct Packet {
	char Proto[4];
	char Name[12];
	uint8_t Mode;
	uint32_t datalen;

	uint8_t data[]; /* MAX 1024 */
} Packet;

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
MYLIB_API int fillTracItem(tracItem* trac, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, void* fileOffset, char* fileReq);
MYLIB_API void failCallback(log_Event *ev);
MYLIB_API Protonet* Init(void);
MYLIB_API Protonet* Stop(void);
MYLIB_API void failTest(void);
#else
uv_interface_address_t getInterIP(char interface_name[]);
int sendPck(/*int fd*/ uv_stream_t* stream_tcp, uv_write_cb write_cb, char* Name, uint8_t Mode, void* data, uint size);
/*int readPck(int fd, Packet* buf);*/
int fillTracItem(tracItem* trac, uint tracID, char* fileRequester, uint8_t hops, uint8_t lifetime, void* fileOffset, char* fileReq);
void failCallback(log_Event *ev);
MYLIB_API Protonet* Init(void);
MYLIB_API Protonet* Stop(void);
#endif

#ifdef __cplusplus
}
#endif

#endif