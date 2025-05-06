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
		uv_timer_t pollTimeout;
		uv_check_t tracChecker;
	    int nConn;
	    char IP[INET_ADDRSTRLEN];		/* host IP */
	    char destIP[INET_ADDRSTRLEN];	/* Destination IP */
	    char serverName[12];	/* Server Name */ 
	    Client* client;		/* for client-server hybrid */
	    vector<Client*> Clientlist;
	    vector<tracItem*> Traclist;
	    string dir;	/* Server Dir */

		MYLIB_API Server(char* inter, char* serverName, char Dir[], char* peerIp);

		private:
			static void on_connection(uv_stream_t *server, int status);
			static void on_disconnection(uv_shutdown_t *req, int status);
			static void pckParser(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
			static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
			static void write_cb(uv_write_t *req, int status);
			static void tracCheck(uv_check_t *handle);
			static void threadStart(void* data);
			static void on_close(uv_handle_t *handle);

};
#endif