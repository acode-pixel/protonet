#include <uv.h>
#include "./proto/client.hpp"
// make client name random letters not ip

Client:: Client(const char* inter, const char* IP, int serverPort, const char name[], const char outpath[]){
	log_add_callback(failCallback, NULL, 5);

	uv_interface_address_t addr;
	addr = getInterIP((char*)inter);
	log_info("Client IP: %s", inet_ntoa(addr.address.address4.sin_addr));

	if (name == NULL || strlen(name) > MAX_NAMESIZE || strlen(name) < MIN_NAMESIZE){
		if(name != NULL)
			(strlen(name) > MAX_NAMESIZE || strlen(name) < MIN_NAMESIZE) ? log_info("Note: Name should be between %d and %d long.", MIN_NAMESIZE, MAX_NAMESIZE) : (void)NULL;
		char name[MAX_NAMESIZE];
		//uv_ip4_name(&addr.address.address4, str_addr, INET_ADDRSTRLEN);
		char data[8];
		uv_random(NULL, NULL, data, 8, 0, NULL);
		getHex((uint8_t*)data, 8, name);
		this->name->assign(name);
	} 

	else {this->name->assign(name);}
	log_info("Client name: %s", this->name->c_str());
	this->tid = uv_thread_self();

	if(!this->name->empty()){
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

		this->outDir->assign(outpath);
		log_info("Client output Dir: %s", this->outDir->c_str());

		proto_setClient(this);
		int r = Client::connectToNetwork((char*)IP, serverPort);

		if(r < 0){
			log_error("An error occured while creating Client.");
		} else {
			uv_timer_init(this->loop, &this->pollTimeout);
			uv_timer_start(&this->pollTimeout, NOP, 200, 200);
			uv_barrier_init(&this->barrier, 2);
			log_info("Successfully created Client");
			memset(&this->trac, 0, sizeof(tracItem));
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
	uv_loop_t* loop = client->loop;
	uv_run(client->loop, UV_RUN_DEFAULT);
	free(loop);
	return;
}


Client::~Client(){
	if(uv_is_active((uv_handle_t*)&this->socket) != 0){
		uv_close((uv_handle_t*)&this->socket, NULL);
	}
	if(uv_loop_alive(this->loop) != 0)
		uv_stop(this->loop);

	if(this->fileReq != nullptr)
		delete this->fileReq;
		this->fileReq = nullptr;
	if(this->outDir != nullptr)
		delete this->outDir;
		this->outDir = nullptr;
	if(this->name != nullptr)
		delete this->name;
		this->name = nullptr;
	if(this->Servername != nullptr)
		delete this->Servername;
		this->Servername = nullptr;
	free(this->loop);
}

int Client::connectToNetwork(char* IP, int port){
    uv_connect_t connect_req;
	uv_tcp_t* tcpSocket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t)); 
	//uv_tcp_t tcpSocket;

	uv_tcp_init(this->loop, tcpSocket);

	struct sockaddr_in dest;

    uv_ip4_addr(IP, port, &dest);

    tcpSocket->data = this;

    uv_tcp_connect(&connect_req, tcpSocket, (sockaddr*)&dest, Client::on_connect);
	uv_run(this->loop, UV_RUN_ONCE);

	if(this->socket != NULL){
		log_info("Connected to %s", IP);
		return 0;
	} 
	return -1; 
}

int Client::disconnectFromNetwork(){
	//uv_fs_t req;
	//this->socket->data = this;
	uv_shutdown_t* shreq = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
	shreq->data = this;
	// note: somehow handle disconnect if it is server hybrid
	if(this->trac.tracID != 0){
		char data[] = "DISCONNECT";
		struct DATA* buff = (struct DATA*)malloc(sizeof(struct DATA));
		strcpy(buff->data, data);
		buff->tracID = this->trac.tracID;
        sendPck((uv_stream_t*)this->trac.Socket, Client::on_write, (char*)this->name->c_str(), SPTP_DATA, buff, sizeof(struct DATA)-(MAX_DATASIZE-10));
		uv_shutdown(shreq, this->socket, Client::on_disconnect);
		free(buff);
	} else {
		//uv_close((uv_handle_t*)this->trac.Socket, Client::on_disconnect);
		uv_shutdown(shreq, this->socket, Client::on_disconnect);
		uv_read_stop(this->socket);
	}
	return 0;
}

void Client::on_disconnect(uv_shutdown_t *req, int status){
	Client* client = (Client*)req->data;
	struct sockaddr_storage addr;
	int size = sizeof(struct sockaddr_storage);
	char str_addr[INET_ADDRSTRLEN];
	uv_tcp_getpeername((uv_tcp_t*)client->socket, (struct sockaddr*)(&addr), &size);
	uv_ip4_name((struct sockaddr_in*)(&addr), str_addr, INET_ADDRSTRLEN);
	log_info("Disconnected from %s", (client->Servername->empty()) ? str_addr : client->Servername->c_str());
	uv_close((uv_handle_t*)client->socket, Client::on_close);
	//free((uv_handle_t*)client->socket);
	//log_info("Disconnected from network");
	free(req);
	return;
	//delete client;
	//uv_thread_detach(uv_thread_self());
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
	uv_timespec64_t past, future;
    uv_clock_gettime(UV_CLOCK_MONOTONIC, &past);
	struct BROD* br = (struct BROD*)malloc(sizeof(struct BROD) + strlen(File)+1);
	memset(br, 0, sizeof(struct BROD) + strlen(File)+1);
	br->hops = 0x01;
	strcpy(br->fileReq, File);
	this->fileReq->assign(File);
	//this->socketMode = SPTP_BROD;
	sendPck(this->socket, Client::on_write, (char*)this->name->c_str(), 1, br, sizeof(struct BROD) + strlen(File));
	strncpy(this->trac.fileRequester, this->name->c_str(), MAX_NAMESIZE);
	strcpy(this->trac.fileReq, this->fileReq->c_str());
	uv_barrier_wait(&this->barrier);
	uv_clock_gettime(UV_CLOCK_MONOTONIC, &future);
	log_info("Completed request in %ld.%us", (future.tv_sec - past.tv_sec), (future.tv_nsec - past.tv_nsec));
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

	if (pck->Mode == SPTP_TRAC && !client->trac.readAgain){
		struct TRAC* pckdata = (struct TRAC*)pck->data;

		if(strncmp(pckdata->Name, client->name->c_str(), MAX_NAMESIZE) != 0){
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
		client->trac.tracID = pckdata->tracID;
		client->Servername->assign(pck->Name);
		client->socketMode = SPTP_TRAC;
		strcpy((char*)data->data, "OK");
		sendPck(client->socket, Client::on_write, (char*)client->name->c_str(), SPTP_DATA, data, sizeof(struct DATA)-(MAX_DATASIZE-2));
		free(data);
		log_info("Downloading %s (size=%d bytes)", client->trac.fileReq, client->trac.fileSize);
		
	} else if(pck->Mode == SPTP_DATA || client->trac.readAgain){
		// data go brrrrrrrrrrrrrrrr
		uv_fs_t req;
		struct DATA* pckdata = (struct DATA*)pck->data;
		string filepath;

		if(strncmp((char*)pckdata->data, "EOF", 3) == 0){
			// close open file
			uv_fs_ftruncate(client->loop, &req, client->trac.file, client->trac.fileSize, NULL);
			uv_fs_req_cleanup(&req);

			uv_fs_close(client->loop, &req, client->trac.file, NULL);
			client->trac.complete = true;
			uv_fs_req_cleanup(&req);
			filepath.assign(*client->outDir).append(client->trac.fileReq);

			getFileHashSHA256((char*)filepath.c_str(), client->loop, client->trac.hash); // rename later

			char encoded[sizeof(client->trac.hash)*2];
			getHex(client->trac.hash, sizeof(client->trac.hash), encoded);

			log_info("Downloaded File hash: %s", encoded);
			log_info("Total rx: %d", client->trac.total_received);

			struct DATA* data2 = (struct DATA*)malloc(sizeof(struct DATA));
			memset(data2, 0, sizeof(struct DATA));
			data2->tracID = client->trac.tracID;
			memcpy(data2->data, "VERIFY", 6);
			memcpy(data2->data+7, client->trac.hash, 32);
			sendPck(stream, Client::on_write, (char*)client->name->c_str(), SPTP_DATA, data2, sizeof(struct DATA) - (MAX_DATASIZE - 39));
			free(data2);

		} else if(strncmp((char*)pckdata->data, "DISCONNECT OK", 13) == 0){
			uv_shutdown_t shreq;
			shreq.data = client;
			//uv_shutdown(&shreq, client->socket, Client::on_disconnect);
			uv_read_stop(client->socket);
		} else if(strncmp((char*)pckdata->data, "VERIFIED", 8) == 0){
			log_info("File %s is verified", client->trac.fileReq);
			uv_barrier_wait(&client->barrier);
		} else if(strncmp((char*)pckdata->data, "NOT VERIFIED", 12) == 0){
			filepath.assign(*client->outDir).append(client->trac.fileReq);
			log_info("File %s is not verified, deleting file", client->trac.fileReq);
			uv_fs_unlink(client->loop, &req, filepath.c_str(), NULL);
			uv_barrier_wait(&client->barrier);

		}else {
			if(client->trac.file == 0){
				filepath.assign(*client->outDir).append(client->trac.fileReq);
				uv_fs_open(client->loop, &req, filepath.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
				client->trac.file = req.result;
				uv_fs_req_cleanup(&req);
			}

			if(client->trac.readAgain){
				uv_buf_t buff = uv_buf_init(buf->base, nread);
				uv_fs_write(client->loop, &req, client->trac.file, &buff, 1, client->trac.fileOffset, NULL);
				uv_fs_req_cleanup(&req);
				client->trac.readExtra -= nread;
				client->trac.fileOffset += nread;
				if(client->trac.readExtra <= 0){
					client->trac.readAgain = false;
					client->trac.readExtra = 0;
				}
			} else if(pck->datalen-8 > nread){
				uv_buf_t buff = uv_buf_init((char*)pckdata->data, nread-(sizeof(Packet)+8));
				uv_fs_write(client->loop, &req, client->trac.file, &buff, 1, client->trac.fileOffset, NULL);
				uv_fs_req_cleanup(&req);
				client->trac.fileOffset += nread-(sizeof(Packet)+8);
				client->trac.readAgain = true;
				client->trac.readExtra = (pck->datalen-8) - (nread-(sizeof(Packet)+8));
			} else {
				uv_buf_t buff = uv_buf_init((char*)pckdata->data, pck->datalen-8);
				uv_fs_write(client->loop, &req, client->trac.file, &buff, 1, client->trac.fileOffset, NULL);
				uv_fs_req_cleanup(&req);
				client->trac.fileOffset += pck->datalen-8;
			}
			client->trac.total_received += 1;
		}
	}

	free(buf->base);


}

void Client::on_close(uv_handle_t *handle){
	Client* client = (Client*)handle->data;
	free(client->socket);
	client->socket = nullptr;
	return;
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
