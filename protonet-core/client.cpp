#include "client.hpp"

Client::Client(char* inter, char name[], uv_loop_t* loop){
	log_add_callback(failCallback, NULL, 5);

	uv_interface_address_t addr;
	addr = getInterIP(inter);
	log_info("Client IP: %s", inet_ntoa(addr.address.address4.sin_addr));

	if (name == NULL){
		strcpy(this->name, (char*)(inet_ntoa(addr.address.address4.sin_addr)));
	} 
	else {strcpy(this->name, name);}
	log_info("Client name: %s", this->name);

	if(this != NULL && this->name != NULL){
		log_info("Successfully created Client");
		this->loop = loop;
		return;
	} else {
		delete this;
		log_error("An error occured while creating Client.[%s]", strerror(errno));
		return;
	}
}

Client::~Client(){
	this->loop = nullptr;
	if(uv_is_active((uv_handle_t*)&this->socket) != 0){
		uv_close((uv_handle_t*)&this->socket, NULL);
	}
	return;
}

void Client::connectToNetwork(char* IP){
    uv_connect_t connect_req;
	uv_tcp_t tcpSocket; 

	uv_tcp_init(this->loop, &tcpSocket);

	struct sockaddr_in dest;

    uv_ip4_addr(IP, C_PORT, &dest);

    tcpSocket.data = this;

    uv_tcp_connect(&connect_req, &tcpSocket, (sockaddr*)&dest, Client::on_connect);
	uv_run(this->loop, UV_RUN_ONCE);
}

void Client::on_connect(uv_connect_t *req, int status){
    Client* client = (Client*)(req->handle->data);
    if (status == 0) {
        // Handle successful connection
        log_info("Connected!");
		memcpy(&client->socket, req->handle, sizeof(uv_tcp_t));
    } else {
        log_error("Connection failed.[%s]", strerror(errno));
    }
}

int Client::makeFileReq(char File[]){
	if(strlen(File) > 255){
		log_error("File name too long");
		return -1;
	}
	struct BROD* br = (struct BROD*)malloc(sizeof(struct BROD) + strlen(File)+1);
	br->hops = 0x01;
	strcpy(br->fileReq, File);
	strcpy(this->fileReq, File);
	this->socketMode = 1;
	sendPck(&this->socket, Client::on_write, this->name, 1, br, 0);
	fillTracItem(&this->trac, 0, this->name, 0, 0, NULL, this->name);
	free(br);
	return 0;

}

void Client::on_write(uv_write_t* req, int status){
	Client* client = (Client*)(req->handle->data);
    if (status == 0) {
        log_info("Request sent!");
		uv_read_start(req->handle, Client::alloc_buf, Client::read);
    } else {
        log_error("Request failed.[%s]", strerror(errno));
     }
}

void Client::alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void Client::read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
	
}
/*
bool clientCheckSocket(Client* client){
	struct timespec ts = {1, 0};

	int nSockets = kevent(client->kqueueInstance, NULL, 0, NULL, 1, &ts);

	if (nSockets == 0){
		return false;
	} else {return true;}
}
*/