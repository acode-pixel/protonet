#include "client.hpp"

Client:: Client(char* inter, char name[], char* IP, char outpath[]){
	log_add_callback(failCallback, NULL, 5);

	uv_interface_address_t addr;
	addr = getInterIP(inter);
	log_info("Client IP: %s", inet_ntoa(addr.address.address4.sin_addr));

	if (name == NULL){
		uv_ip4_name(&addr.address.address4, this->name, INET_ADDRSTRLEN);
	} 

	else {strcpy(this->name, name);}
	log_info("Client name: %s", this->name);
	this->tid = uv_thread_self();

	if(this->name != NULL){
		uv_loop_t* loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
		uv_loop_init(loop);
		this->loop = loop;

		uv_fs_t req;
		uv_fs_access(this->loop, &req, outpath, R_OK | W_OK, NULL);

		if(req.result < 0){
			log_error("Client outDir %s cant be accessed.[%s]", outpath, uv_err_name(req.result));
			log_debug("Client outDir %s cant be accessed.[%s]", outpath, uv_strerror(req.result));
			delete this;
			uv_fs_req_cleanup(&req);
			return;
		}

		uv_fs_req_cleanup(&req);

		this->outDir.assign(outpath);
		log_info("Client output Dir: %s", this->outDir.c_str());

		proto_setClient(this);
		int r = Client::connectToNetwork(IP);

		if(r < 0){
			log_error("An error occured while creating Client.");
		} else {
			uv_timer_init(this->loop, &this->pollTimeout);
			uv_timer_start(&this->pollTimeout, NOP, 200, 200);
			log_info("Successfully created Client");
			uv_thread_create(&this->tid, Client::threadStart, this);
			log_info("Started client thread: %lu", this->tid);
		}
		return;

	} else {
		delete this;
		log_error("An error occured while creating Client.[%s]", strerror(errno));
		return;
	}
}


void Client::threadStart(void* data){
	Client* client = (Client*)data;
	uv_run(client->loop, UV_RUN_DEFAULT);
}


Client::~Client(){
	if(uv_is_active((uv_handle_t*)&this->socket) != 0){
		uv_close((uv_handle_t*)&this->socket, NULL);
	}
	uv_stop(this->loop);
	free(this->loop);
	return;
}

int Client::connectToNetwork(char* IP){
    uv_connect_t connect_req;
	uv_tcp_t* tcpSocket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t)); 
	//uv_tcp_t tcpSocket;

	uv_tcp_init(this->loop, tcpSocket);

	struct sockaddr_in dest;

    uv_ip4_addr(IP, C_PORT, &dest);

    tcpSocket->data = this;

    uv_tcp_connect(&connect_req, tcpSocket, (sockaddr*)&dest, Client::on_connect);
	uv_run(this->loop, UV_RUN_ONCE);

	if(this->socket != NULL){
		log_info("Connected to %s", IP);
		return 0;
	}
	return -1; 
}

void Client::on_connect(uv_connect_t *req, int status){
    Client* client = (Client*)(req->handle->data);
    if (status == 0) {
        // Handle successful connection
		//memcpy(&client->socket, req->handle, sizeof(uv_tcp_t));
		client->socket = req->handle;
		uv_read_start(req->handle, Client::alloc_buf, Client::read);
    } else {
        log_error("Connection failed.[%s]", uv_err_name(status));
		log_debug("Connection failed.[%s]", uv_strerror(status));
		client->socket = NULL;
		free(req->handle);
    }
}

int Client::makeFileReq(char File[]){
	if(strlen(File) > 255){
		log_error("[File name too long]");
		return -1;
	}
	struct BROD* br = (struct BROD*)malloc(sizeof(struct BROD) + strlen(File)+1);
	memset(br, 0, sizeof(struct BROD) + strlen(File)+1);
	br->hops = 0x01;
	strcpy(br->fileReq, File);
	strcpy(this->fileReq, File);
	//this->socketMode = SPTP_BROD;
	sendPck(this->socket, Client::on_write, this->name, 1, br, 0);
	strcpy(this->trac.fileRequester, this->name);
	strcpy(this->trac.fileReq, this->fileReq);

	//fillTracItem(&this->trac, 0, this->name, 0, 0, NULL, this->name);
	free(br);
	return 0;

}

void Client::on_write(uv_write_t* req, int status){
	Client* client = (Client*)(req->handle->data);

	if(status < 0){
		log_error("Sending packet failed.[%s]", uv_strerror(status));
		free(req->data);
	 	free(req);
		return;
	}

	Packet* pck = (Packet*)req->data;

	switch (pck->Mode)
	{
	case SPTP_BROD:
		log_info("Request sent!");
		client->socketMode = SPTP_BROD;
		break;
	
	case SPTP_DATA:
		struct DATA* data = (struct DATA*)pck->data;
		log_info("Sending data with tracID: %d", data->tracID);
		client->socketMode = SPTP_DATA;
		break;
	}

	 free(req->data);
	 free(req);
}

void Client::alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void Client::read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
	if (nread < 0){
		log_error("Client failed to read due to error: [%s]", uv_err_name(nread));
		log_debug("Client failed to read due to error: [%s]", uv_strerror(nread));
		return;
	} else if(nread == 0){
		log_debug("Client read would block");
		free(buf->base);
		return;
	}

	Packet* pck = (Packet*)buf->base;
	Client* client = (Client*)proto_getClient();

	if (pck->Mode == SPTP_TRAC){
		struct TRAC* pckdata = (struct TRAC*)pck->data;

		if(strcmp(pckdata->Name, client->name) != 0){
			if(proto_getServer() != NULL){
				// WIP (when it isnt for us but we can send it to someone else)
			}
			free(buf->base);
			return;
		}

		// when packet is intended for us
		struct DATA* data = (struct DATA*)malloc(sizeof(struct DATA));
		memset(data, 0, sizeof(struct DATA));
		data->tracID = pckdata->tracID;
		client->trac.confirmed = true;
		client->trac.hops = pckdata->hops;
		client->trac.Socket = (uv_tcp_t*)stream;
		client->trac.fileSize = pckdata->fileSize;
		strcpy((char*)data->data, "OK");
		sendPck(client->socket, Client::on_write, client->name, SPTP_DATA, data, sizeof(data));
		free(data);
		
	} else if(pck->Mode == SPTP_DATA){
		// data go brrrrrrrrrrrrrrrr
		uv_fs_t req;
		struct DATA* pckdata = (struct DATA*)pck->data;
		if(strcmp((char*)pckdata->data, "EOF") == 0){
			// close open file
			uv_fs_close(client->loop, &req, client->trac.file, NULL);
			client->trac.canDelete = true;
		} else {
			if(client->trac.file == 0){
				string filepath;
				filepath.assign(client->outDir).append(client->trac.fileReq);
				uv_fs_open(client->loop, &req, filepath.c_str(), O_CREAT | O_RDWR, 0, NULL);
				client->trac.file = req.result;
				uv_fs_req_cleanup(&req);
			}

			uv_buf_t buff = uv_buf_init((char*)pckdata->data, pck->datalen-4);
			uv_fs_write(client->loop, &req, client->trac.file, &buff, 1, client->trac.fileOffset, NULL);
		}

		uv_fs_req_cleanup(&req);


	}

	free(buf->base);


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