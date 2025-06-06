#ifndef SERVER_H
#define SERVER_H
#define MAX_CLIENTS 10
#define DEFAULT_TRAC_LIFETIME 10

extern "C" {
	#include "./core.h"
	#include "./log.h"
}

// added due to vscode c Intelisense not detecting __FILE_NAME__
#ifndef __FILE_NAME__
#define __FILE_NAME__ "server.cpp"
#endif

#include "./client.hpp"
#include "./utils.hpp"
#include <vector>
#include <string>

using namespace std; 

class Server {
    public:
	    uv_tcp_t* Socket;		/* Socket */
		uv_loop_t* loop;
		uv_thread_t tid;
		uv_timer_t tracChecker;
		uv_async_t cross_write;
		uv_mutex_t cross_write_lock;
	    int nConn;
	    char IP[INET_ADDRSTRLEN];		/* host IP */
	    char destIP[INET_ADDRSTRLEN];	/* Destination IP */
	    string serverName;	/* Server Name */ 
	    Client* client;		/* for client-server hybrid */
	    vector<Client*> Clientlist;
	    vector<tracItem*> Traclist;
		vector<cross_write_req*> cross_writes;
	    string dir;	/* Server Dir */

		MYLIB_API Server(const char* inter, const char Dir[] = "./", int port = S_PORT, const char* serverName = "", const char* peerIp = "", int peerPort = S_PORT);
		static void write_to_Serv_Sok(uv_async_t* handle);
		static void link_write(uv_write_t* req, int status);

		private:
			static void on_connection(uv_stream_t *server, int status);
			static void on_disconnection(uv_shutdown_t *req, int status);
			static void pckParser(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
			static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
			static void write_cb(uv_write_t *req, int status);
			static void tracCheck(uv_timer_t *handle);
			static void threadStart(void* data);
			static void on_close(uv_handle_t *handle);

};
#endif