#include "server.hpp"

Server::Server(char* inter, char* serverName, char Dir[], char* peerIp, uv_loop_t* loop){
	if (access(Dir, R_OK) == -1){
		log_error("Directory %s not accessible", Dir);
	}
	
	// alloc server 
	uv_tcp_init(loop, (uv_tcp_t*)&this->Socket);
	this->loop = loop;

	uv_interface_address_t addr = getInterIP(inter);

	uv_ip4_name(&addr.address.address4, this->IP, INET_ADDRSTRLEN); // src IP
	uv_ip4_addr(this->IP, S_PORT, &addr.address.address4);

	log_info("Server IP: %s", this->IP);
	strcpy(this->serverName, serverName);
	log_info("Server name: %s", this->serverName);
	memcpy(this->dir, Dir, strlen(Dir));
	log_info("Server dir: %s", this->dir);

	int r = uv_tcp_bind((uv_tcp_t*)&this->Socket, (struct sockaddr*)&addr.address.address4, 0);
    if (r) {
		delete this;
        log_error("Failed binding to Socket due to error: [%s]", uv_err_name(r));
		log_debug("Failed binding to Socket due to error: [%s]", uv_strerror(r));
        return;
    }

	uv_listen(&this->Socket, 10, on_connection);

	if(strlen(peerIp) > 0){
		Client* client = new Client(inter, serverName, loop);
		client->connectToNetwork(peerIp);
		memcpy(&this->client, client, sizeof(Client));
		strcpy(this->IP, peerIp);
	}

	log_info("Successfully created Server");
	proto_setServer(this);

	return;
}

void Server::on_connection(uv_stream_t *stream, int status){
	if (status < 0){
		log_error("Incoming connection error: [%s]", uv_err_name(status));
		log_debug("Incoming connection error: [%s]", uv_strerror(status));
		return;
	}

	uv_tcp_t* client_conn = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	Server* server = (Server*)proto_getServer();
	uv_tcp_init(server->loop, client_conn);
	if (uv_accept(stream, (uv_stream_t*) client_conn) == 0) {
		Client* new_client = (Client*)malloc(sizeof(Client));
		struct sockaddr_storage addr;
		int size = INET_ADDRSTRLEN;
		uv_tcp_getpeername(client_conn, (struct sockaddr*)&addr, &size);
		uv_ip4_name((struct sockaddr_in*)&addr, new_client->name, INET_ADDRSTRLEN);
		new_client->socket = (uv_stream_t*)client_conn;

		server->Clientlist.push_back(new_client);
        uv_read_start(new_client->socket, Server::alloc_buf, Server::pckParser);
    }
}

void Server::alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void Server::pckParser(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
	return;
}