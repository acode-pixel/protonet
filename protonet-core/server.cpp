#include "server.hpp"

Server::Server(char* inter, char* serverName, char Dir[], char* peerIp){
	uv_interface_address_t addr = getInterIP(inter);

	uv_loop_t* loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
	uv_loop_init(loop);
	
	this->loop = loop;

	uv_ip4_name(&addr.address.address4, this->IP, INET_ADDRSTRLEN); // src IP
	uv_ip4_addr(this->IP, S_PORT, &addr.address.address4);

	log_info("Server IP: %s", this->IP);
	strcpy(this->serverName, serverName);
	log_info("Server name: %s", this->serverName);
	dir.assign(Dir);
	if (strcmp("/", &dir.back()) != 0)
		dir += "/";
	log_info("Server dir: %s", this->dir.c_str());

	this->Socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, this->Socket);
	uv_tcp_bind(this->Socket, (struct sockaddr*)&addr.address.address4, 0);

	int r = uv_listen((uv_stream_t*)this->Socket, 10, on_connection);
	if(r != 0){
		log_error("Server failed to listen on port %d.[%s]", S_PORT, uv_err_name(r));
		log_debug("Server failed to listen on port %d.[%s]", S_PORT, uv_strerror(r));
		delete this;
		return;
	}

	if(strlen(peerIp) > 0){
		Client* client = new Client(inter, serverName, peerIp, Dir);
		memcpy(&this->client, client, sizeof(Client));
		strcpy(this->IP, peerIp);
	}

	uv_timer_init(this->loop, &this->pollTimeout);
	uv_timer_start(&this->pollTimeout, NOP, 200, 200);

	uv_fs_t req;
	uv_fs_access(this->loop, &req, Dir, UV_FS_O_RDONLY, NULL);
	if (req.result < 0){
		log_error("Directory %s not accessible", Dir);
		delete this;
		return;
	}
	uv_fs_req_cleanup(&req);

	uv_thread_create(&this->tid, Server::threadStart, this);

	uv_check_init(this->loop, &this->tracChecker);
	uv_check_start(&this->tracChecker, Server::tracCheck);

	log_info("Successfully created Server");
	log_info("Started server thread: %lu", this->tid);
	proto_setServer(this);

	return;
}

void Server::threadStart(void* data){
	Server* server = (Server*)data;
	uv_run(server->loop, UV_RUN_DEFAULT);
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
		char str_addr[INET_ADDRSTRLEN];
		uv_tcp_getpeername(client_conn, (struct sockaddr*)&addr, &size);
		uv_ip4_name((struct sockaddr_in*)&addr, str_addr, INET_ADDRSTRLEN);
		new_client->name = new string(str_addr);
		new_client->socket = (uv_stream_t*)client_conn;

		server->Clientlist.push_back(new_client);
        uv_read_start(new_client->socket, Server::alloc_buf, Server::pckParser);
    }
}

void Server::alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void Server::write_cb(uv_write_t *req, int status){
	if(status < 0){
		log_error("Server failed to write due to error: [%s]", uv_err_name(status));
		log_debug("Server failed to write due to error: [%s]", uv_strerror(status));
		free(req->data);
		free(req);
		return;
	}

	Packet* pck = (Packet*)req->data;

	switch (pck->Mode)
	{
	case SPTP_BROD:
		log_debug("Server sent BROD packet");
		break;
	case SPTP_TRAC:
		log_debug("Server sent TRAC packet");
		break;
	case SPTP_DATA:
		log_debug("Server sent DATA packet");
		break;
	}

	free(req->data);
	free(req);

}

void Server::pckParser(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
	if (nread < 0){
		log_error("Server failed to read due to error: [%s]", uv_err_name(nread));
		log_debug("Server failed to read due to error: [%s]", uv_strerror(nread));
		return;
	} else if(nread == 0){
		log_debug("Server read would block");
		free(buf->base);
		return;
	}

	Packet* pck = (Packet*)buf->base;
	Server* server = (Server*)proto_getServer();

	if(pck->Mode == SPTP_BROD){
		log_debug("Server received BROD packet");
		struct BROD* pckData = (struct BROD*)pck->data;
		char filepath[server->dir.size()+strlen(pckData->fileReq)+1];
		memset(filepath, 0, sizeof(filepath));
		strcpy(filepath, server->dir.c_str());
		memcpy(&filepath[server->dir.size()], pckData->fileReq, strlen(pckData->fileReq));
		uv_fs_t req;
		uv_fs_access(server->loop, &req, filepath, UV_FS_O_RDONLY, NULL);

		if(req.result < 0){
			uv_fs_req_cleanup(&req);
			log_error("Server cant access file %s due to error: [%s]", filepath, uv_err_name(req.result));
			log_debug("Server cant access file %s due to error: [%s]", filepath, uv_strerror(req.result));
			free(buf->base);
			return;
		}

		srand(time(0));
		uv_fs_req_cleanup(&req);
		uv_fs_stat(server->loop, &req, filepath, NULL);

		// create trac data
		struct TRAC* data = (TRAC*)malloc(sizeof(struct TRAC));
		memset(data, 0, sizeof(struct TRAC));
		strcpy(data->Name, pck->Name);
		data->tracID = rand();
		data->lifetime = pckData->hops;
		data->hops = pckData->hops;
		data->fileSize = req.statbuf.st_size;

		tracItem* trac = (tracItem*)malloc(sizeof(tracItem));
		memset(trac, 0, sizeof(tracItem));
		trac->tracID = data->tracID;
		trac->lifetime = data->lifetime;
		trac->socketStatus = SPTP_TRAC;
		trac->fileSize = data->fileSize;
		strcpy(trac->fileRequester, data->Name);
		strcpy(trac->fileReq, filepath);
		server->Traclist.push_back(trac);

		// send to client lists

		for(Client* client : server->Clientlist){
			sendPck(client->socket, Server::write_cb, server->serverName, SPTP_TRAC, data, sizeof(struct TRAC));
		}

		free(data);
		uv_fs_req_cleanup(&req);

	} else if(pck->Mode == SPTP_DATA){
		//  make a func that schedule the sending of file data client requested
		struct DATA* data = (struct DATA*)pck->data;
		for (tracItem* trac : server->Traclist){
			if(strcmp(pck->Name, trac->fileRequester) == 0 && data->tracID == trac->tracID){
				if(strcmp((char*)data->data, "OK") == 0){
					trac->confirmed = true;
					trac->Socket = (uv_tcp_t*)stream;
					trac->socketStatus = SPTP_DATA;
					break;
				}
			}
		}
	}
	free(buf->base);

	return;
}

void Server::tracCheck(uv_check_t *handle){
	Server* serv = (Server*)proto_getServer();
	if(serv->Traclist.size() != 0){
		for(tracItem* trac : serv->Traclist){

			if(!trac->confirmed){
				continue;
			} else if(trac->canDelete){
				memset(trac, 0, sizeof(tracItem));
				continue;
			}

			uv_fs_t req;

			if(trac->file == 0){
				uv_fs_open(serv->loop, &req, trac->fileReq, O_RDONLY, 0, NULL);
				if(req.result < 0){
					log_error("Server failed to open requested file %s.[%s]", trac->fileReq, uv_err_name(req.result));
					log_debug("Server failed to open requested file %s.[%s]", trac->fileReq, uv_strerror(req.result));
					uv_fs_req_cleanup(&req);
					continue;
				} 
				
				trac->file = req.result;
				uv_fs_req_cleanup(&req);
			}

			struct DATA* data = (struct DATA*)malloc(sizeof(struct DATA));
			memset(data, 0, sizeof(struct DATA));
			data->tracID = trac->tracID;
			uv_buf_t buff = uv_buf_init((char*)data->data, MAX_FILESIZE);
			uv_fs_read(serv->loop, &req, trac->file, &buff, 1, -1, NULL);

			if(req.result < 0){
				log_error("Server failed to read requested file %s.[%s]", trac->fileReq, uv_err_name(req.result));
				log_debug("Server failed to read requested file %s.[%s]", trac->fileReq, uv_strerror(req.result));
			} else if(req.result == 0){
				// were done reading file
				uv_fs_close(serv->loop, &req, trac->file, NULL);
				trac->canDelete = true;
				strcpy((char*)data->data, "EOF");
				sendPck((uv_stream_t*)trac->Socket, Server::write_cb, serv->serverName, SPTP_DATA, data, 7);
			} else {
				trac->fileOffset += req.result;
				sendPck((uv_stream_t*)trac->Socket, Server::write_cb, serv->serverName, SPTP_DATA, data, sizeof(struct DATA)-(MAX_FILESIZE-req.result));
			} 
			uv_fs_req_cleanup(&req);
			free(data);

		}
	}
}