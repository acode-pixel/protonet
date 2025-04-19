#ifndef CLIENT_H
#define CLIENT_H

extern "C" {
    #include "./core.h"
    #include "./log.h"
}

#include <string>

using namespace std;

class Client {
    public:
        uv_thread_t tid;
        char name[INET_ADDRSTRLEN];
        uv_stream_t* socket;
        uv_timer_t pollTimeout;
	    int socketMode;
	    tracItem trac;
	    char fileReq[255];
        uv_loop_t* loop;
        string outDir;

        MYLIB_API Client(char* inter, char name[], char* IP, char outpath[]);
        MYLIB_API ~Client();
        int connectToNetwork(char* IP);
        MYLIB_API int makeFileReq(char File[]);
        int C_tracParser(Packet* buf);
        int C_brodParser(Packet* buf);
    
    private:
        static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
        static void on_connect(uv_connect_t* req, int status);
        static void on_write(uv_write_t* req, int status);
        static void read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
        static void threadStart(void* data);
        //bool clientCheckSocket(Client* client);

};

#endif