#ifndef CLIENT_H
#define CLIENT_H

extern "C" {
    #include "./core.h"
    #include "./log.h"
}

#include <string>
#include <vector>
#include "./utils.hpp"

// added due to vscode c Intelisense not detecting __FILE_NAME__
#ifndef __FILE_NAME__
#define __FILE_NAME__ "client.cpp"
#endif

using namespace std;

class Client {
    public:
        uv_thread_t tid;
        uv_async_t cross_write;
        string* name = new string();
        string* Servername = new string();
        uv_stream_t* socket;
        uv_timer_t pollTimeout;
        uv_barrier_t barrier;
        uv_mutex_t cross_write_lock;
	    int socketMode;
        vector<int> dataList;
	    tracItem trac;
	    string* fileReq = new string();
        uv_loop_t* loop;
        string* outDir = new string();
        bool isPartofaServer = false;
        void* server = NULL;
        uint* processing_trac = (uint*)malloc(sizeof(uint));
        vector<cross_write_req*> cross_writes;

        MYLIB_API Client(const char* inter, const char* IP, int serverPort = S_PORT, const char name[] = "", const char outpath[] = "./");
        MYLIB_API ~Client();
        MYLIB_API int connectToNetwork(char* IP, int port);
        MYLIB_API int makeFileReq(char File[]);
	    MYLIB_API int disconnectFromNetwork();
        static void write_to_client_Sok(uv_async_t* handle);
        static void link_write(uv_write_t* req, int status);
    
    private:
        static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
        static void on_connect(uv_connect_t* req, int status);
        static void on_write(uv_write_t* req, int status);
        static void read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
        static void threadStart(void* data);
	    static void on_disconnect(uv_shutdown_t *req, int status);
        static void on_close(uv_handle_t *handle);
        //bool clientCheckSocket(Client* client);

};

#endif
