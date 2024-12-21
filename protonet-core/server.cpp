#include "server.hpp"

Server::Server(char* inter, char* serverName, char Dir[], char* peerIp, uv_loop_t* loop){
	if (access(Dir, R_OK) == -1){
		log_error("Directory %s not accessible", Dir);
	}
	
	// alloc server 
	uv_tcp_t server;
	uv_tcp_init(loop, &server);

	uv_interface_address_t addr = getInterIP(inter);

	uv_ip4_name(&addr.address.address4, this->IP, INET_ADDRSTRLEN); // src IP
	uv_ip4_addr(this->IP, S_PORT, &addr.address.address4);

	log_info("Server IP: %s", this->IP);
	strcpy(this->serverName, serverName);
	log_info("Server name: %s", this->serverName);
	memcpy(this->dir, Dir, strlen(Dir));
	log_info("Server dir: %s", this->dir);

	int r = uv_tcp_bind(&server, (struct sockaddr*)&addr.address.address4, 0);
    if (r) {
		delete this;
        log_error("Failed binding to Socket due to error: [%s]", uv_err_name(r));
		log_debug("Failed binding to Socket due to error: [%s]", uv_strerror(r));
        return;
    }

	if(strlen(peerIp) > 0){
		Client* client = new Client(inter, serverName, loop);
		client->connectToNetwork(peerIp);
		memcpy(&this->client, client, sizeof(Client));
		strcpy(this->IP, peerIp);
	}

	log_info("Successfully created Server");

	return;
}