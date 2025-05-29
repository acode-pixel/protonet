#include <uv.h>
#include <sodium.h>
#include "./proto/server.hpp"

Server::Server(const char* inter, const char Dir[], int port, const char* serverName, const char* peerIp, int peerPort){
	uv_interface_address_t addr = getInterIP((char*)inter);

	uv_loop_t* loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
	memset(loop, 0, sizeof(uv_loop_t));
	uv_loop_init(loop);
	
	this->loop = loop;
	this->loop->data = this;

	uv_ip4_name(&addr.address.address4, this->IP, INET_ADDRSTRLEN); // src IP
	uv_ip4_addr(this->IP, port, &addr.address.address4);

	log_info("Server IP: %s", this->IP);

	if (serverName == NULL || strlen(serverName) > MAX_NAMESIZE || strlen(serverName) < MIN_NAMESIZE){
		if(serverName != NULL)
			(strlen(serverName) > MAX_NAMESIZE || strlen(serverName) < MIN_NAMESIZE) ? log_info("Note: Name should be between %d and %d long.", MIN_NAMESIZE, MAX_NAMESIZE) : (void)NULL;
		char name[MAX_NAMESIZE];
		//uv_ip4_name(&addr.address.address4, str_addr, INET_ADDRSTRLEN);
		char data[8];
		uv_random(NULL, NULL, data, 8, 0, NULL);
		getHex((uint8_t*)data, 8, name);
		this->serverName.assign(name);
	} 
	else{this->serverName.assign(serverName);}
	log_info("Server name: %s", this->serverName.c_str());
	dir.resize(strlen(Dir)+1);
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

	this->client = nullptr;
	if(strlen(peerIp) > 0){
		Client* client = new Client(inter, peerIp, peerPort, serverName, Dir);
		client->isPartofaServer = true;
		client->server = this;
		this->client = client;
		uv_async_init(this->loop, &this->cross_write, Server::write_to_Serv_Sok);
		uv_async_init(client->loop, &client->cross_write, Client::write_to_client_Sok);
		uv_mutex_init(&client->cross_write_lock);
		uv_mutex_init(&this->cross_write_lock);
		strcpy(this->IP, peerIp);
	}

	memset(&this->pollTimeout, 0, sizeof(uv_timer_t));
	uv_timer_init(this->loop, &this->pollTimeout);
	uv_timer_start(&this->pollTimeout, NOP, 200, 200);

	uv_fs_t req;
	memset(&req, 0, sizeof(uv_fs_t));
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
	//proto_setServer(this);

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
	//Server* server = (Server*)proto_getServer();
	Server* server = (Server*)stream->loop->data;
	uv_tcp_init(server->loop, client_conn);
	//uv_tcp_nodelay(client_conn, 1);
	if (uv_accept(stream, (uv_stream_t*) client_conn) == 0) {
		Client* new_client = (Client*)malloc(sizeof(Client));
		struct sockaddr_storage addr;
		int size = INET_ADDRSTRLEN;
		char str_addr[INET_ADDRSTRLEN];
		uv_tcp_getpeername(client_conn, (struct sockaddr*)&addr, &size);
		uv_ip4_name((struct sockaddr_in*)&addr, str_addr, INET_ADDRSTRLEN);
		new_client->name = new string(str_addr);
		new_client->socket = (uv_stream_t*)client_conn;
		new_client->socket->data = new_client;

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
	Server* serv = (Server*)req->handle->loop->data;

	switch (pck->Mode)
	{
	case SPTP_BROD:
		log_debug("Server[%s] sent BROD packet", serv->serverName.c_str());
		break;
	case SPTP_TRAC:
		log_debug("Server[%s] sent TRAC packet", serv->serverName.c_str());
		break;
	case SPTP_DATA:
		log_debug("Server[%s] sent DATA packet", serv->serverName.c_str());
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
	//Server* server = (Server*)proto_getServer();
	Server* server = (Server*)stream->loop->data;

	if(pck->Mode == SPTP_BROD){
		log_debug("Server[%s] received BROD packet", server->serverName.c_str());
		struct BROD* pckData = (struct BROD*)pck->data;
		char filepath[server->dir.size()+(pck->datalen-1)+1];
		memset(filepath, 0, sizeof(filepath));
		strcpy(filepath, server->dir.c_str());
		memcpy(&filepath[server->dir.size()], pckData->fileReq, pck->datalen-1);
		uv_fs_t req;
		uv_fs_access(server->loop, &req, filepath, UV_FS_O_RDONLY, NULL);

		if(req.result < 0){
			uv_fs_req_cleanup(&req);
			log_error("Server cant access file %s due to error: [%s]", filepath, uv_err_name(req.result));
			log_debug("Server cant access file %s due to error: [%s]", filepath, uv_strerror(req.result));
			if(server->client != NULL){
				log_info("Server[%s] re-broadcasting to other servers", server->serverName.c_str());
				for(Client* client : server->Clientlist){
					if(client->socket == stream && pckData->hops == 1)
						client->name->assign(pck->Name);
				}
				pckData->hops += 1;
				//uv_write_t* wreq = (uv_write_t*)malloc(sizeof(uv_write_t));
				cross_write_req* wreq = (cross_write_req*)malloc(sizeof(cross_write_req));
				char* data = (char*)malloc(sizeof(Packet)+pck->datalen);
				memcpy(data, buf->base, sizeof(Packet)+pck->datalen);
				uv_buf_t buff = uv_buf_init(data, sizeof(Packet)+pck->datalen);
				//wreq->cb = Client::link_write;
				//wreq->handle = (uv_stream_t*)server->client->socket;
				//wreq->data = data;
				//memcpy(&wreq->write_buffer, &buff, sizeof(uv_buf_t));
				wreq->buf = buff;
				wreq->handle = (uv_stream_t*)server->client->socket;
				uv_mutex_lock(&server->client->cross_write_lock);
				server->client->cross_writes.push_back(wreq);
				uv_mutex_unlock(&server->client->cross_write_lock);
				uv_async_send(&server->client->cross_write);
				//sendPck(server->client->socket, NULL, pck->Name, pck->Mode, pck->data, pck->datalen);
			}
			if(nread > sizeof(Packet)+pck->datalen){
				char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
				memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
				uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
				Server::pckParser(stream, buff2.len, &buff2);
			}

			free(buf->base);
			return;
		}

		uv_fs_req_cleanup(&req);
		uv_fs_stat(server->loop, &req, filepath, NULL);

		// check if server recognizes client
		for(tracItem* trac : server->Traclist){
			if(strncmp(trac->fileRequester, pck->Name, MAX_NAMESIZE) == 0){
				trac->confirmed = true;
				trac->complete = false;
				strcpy(trac->fileReq, filepath);
				trac->fileOffset = 0;
				trac->fileSize = req.statbuf.st_size;
				trac->file = 0;

				struct TRAC* data = (TRAC*)malloc(sizeof(struct TRAC));
				memset(data, 0, sizeof(struct TRAC));
				strncpy(data->Name, pck->Name, MAX_NAMESIZE);
				data->tracID = trac->tracID;
				data->lifetime = trac->hops;
				data->hops = trac->hops;
				data->fileSize = trac->fileSize;
				sendPck((uv_stream_t*)trac->Socket, Server::write_cb, (char*)server->serverName.c_str(), SPTP_TRAC, data, sizeof(struct TRAC));
				uv_fs_req_cleanup(&req);
				free(data);

				if(nread > sizeof(Packet)+pck->datalen){
					char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
					memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
					uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
					Server::pckParser(stream, buff2.len, &buff2);
				}

				free(buf->base);
				return;
			}
		}
		// create trac data
		struct TRAC* data = (TRAC*)malloc(sizeof(struct TRAC));
		memset(data, 0, sizeof(struct TRAC));
		strncpy(data->Name, pck->Name, MAX_NAMESIZE);
		randombytes_buf(&data->tracID, sizeof(data->tracID));
		data->lifetime = pckData->hops*2;
		data->hops = pckData->hops;
		data->fileSize = req.statbuf.st_size;

		tracItem* trac = (tracItem*)malloc(sizeof(tracItem));
		memset(trac, 0, sizeof(tracItem));
		trac->tracID = data->tracID;
		trac->lifetime = data->lifetime*2;
		trac->socketStatus = SPTP_TRAC;
		trac->Socket = (uv_tcp_t*)stream;
		trac->fileSize = data->fileSize;
		strncpy(trac->fileRequester, data->Name, MAX_NAMESIZE);
		strcpy(trac->fileReq, filepath);
		server->Traclist.push_back(trac);

		// send to client lists

		for(Client* client : server->Clientlist){
			if(client->socket == stream && pckData->hops == 1)
				client->name->assign(pck->Name);
			sendPck(client->socket, Server::write_cb, (char*)server->serverName.c_str(), SPTP_TRAC, data, sizeof(struct TRAC));
		}

		free(data);
		uv_fs_req_cleanup(&req);

		if(nread > sizeof(Packet)+pck->datalen){
			char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
			memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
			uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
			Server::pckParser(stream, buff2.len, &buff2);
		}

	} else if(pck->Mode == SPTP_DATA){
		//  handle disconnect when were client/server hybrid
		struct DATA* data = (struct DATA*)pck->data;

		if(strncmp(data->data, "DISCONNECT", 10) == 0){
			struct sockaddr_storage addr;
			int size = sizeof(struct sockaddr_storage);
			char str_addr[INET_ADDRSTRLEN];
			uv_tcp_getpeername((uv_tcp_t*)stream, (struct sockaddr*)(&addr), &size);
			uv_ip4_name((struct sockaddr_in*)(&addr), str_addr, INET_ADDRSTRLEN);

			for (Client* client : server->Clientlist){
				
				if(strncmp(client->name->c_str(), pck->Name, MAX_NAMESIZE) == 0 || strcmp(client->name->c_str(), str_addr) == 0){
					char msg[] = "DISCONNECT OK";
					struct DATA* buff = (struct DATA*)malloc(sizeof(struct DATA));
					strcpy(buff->data, msg);
					buff->tracID = data->tracID;
					sendPck(stream, Server::write_cb, (char*)server->serverName.c_str(), SPTP_DATA, buff, sizeof(struct DATA)-(MAX_DATASIZE-13));
					free(buff);

					uv_shutdown_t* shreq = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
					shreq->data = client;
					uv_read_stop(stream);
					uv_shutdown(shreq, stream, Server::on_disconnection);
					//uv_close((uv_handle_t*)stream, NULL);

					for (auto it = server->Traclist.begin(); it != server->Traclist.end(); ++it) {
						if(((tracItem*)(*it))->tracID == data->tracID){
							free(*it);
							server->Traclist.erase(it);
							break;
						}
					}
				}
			}
		} else if(strncmp(data->data, "VERIFY", 6) == 0) {
			for (tracItem* trac : server->Traclist){
				if(strncmp(pck->Name, trac->fileRequester, MAX_NAMESIZE) == 0 && data->tracID == trac->tracID && trac->complete){
					if(server->client != NULL && trac->isLink){
						cross_write_req* wreq = (cross_write_req*)malloc(sizeof(cross_write_req));
						char* data = (char*)malloc(sizeof(Packet)+pck->datalen);
						memcpy(data, buf->base, sizeof(Packet)+pck->datalen);
						uv_buf_t buff = uv_buf_init(data, sizeof(Packet)+pck->datalen);
						wreq->handle = (uv_stream_t*)server->client->socket;
						wreq->buf = buff;
						//wreq->cb = Client::link_write;
						//wreq->handle = (uv_stream_t*)server->client->socket;
						//wreq->data = data;
						//memcpy(&wreq->write_buffer, &buff, sizeof(uv_buf_t));
						uv_mutex_lock(&server->client->cross_write_lock);
						server->client->cross_writes.push_back(wreq);
						uv_mutex_unlock(&server->client->cross_write_lock);
						uv_async_send(&server->client->cross_write);
						//sendPck(server->client->socket, NULL, pck->Name, pck->Mode, pck->data, pck->datalen);
						if(nread > sizeof(Packet)+pck->datalen){
							char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
							memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
							uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
							Server::pckParser(stream, buff2.len, &buff2);
						}
						free(buf->base);
						return;
					}
					getFileHashSHA256(trac->fileReq, server->loop, trac->hash);
					struct DATA* data2 = (struct DATA*)malloc(sizeof(struct DATA));
					if(memcmp(trac->hash, data->data+7, 32) == 0){
						data2->tracID = trac->tracID;
						strcpy(data2->data, "VERIFIED");
						sendPck((uv_stream_t*)trac->Socket, Server::write_cb, (char*)server->serverName.c_str(), SPTP_DATA, data2, sizeof(struct DATA)-(MAX_DATASIZE - 8));
					} else {
						data2->tracID = trac->tracID;
						strcpy(data2->data, "NOT VERIFIED");
						sendPck((uv_stream_t*)trac->Socket, Server::write_cb, (char*)server->serverName.c_str(), SPTP_DATA, data2, sizeof(struct DATA)-(MAX_DATASIZE - 12));
						log_debug("Failed hash for %s", trac->fileReq);
					}
					free(data2);
					if(nread > sizeof(Packet)+pck->datalen){
						char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
						memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
						uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
						Server::pckParser(stream, buff2.len, &buff2);
					}
				}
			}
		} else {
			for (tracItem* trac : server->Traclist){
				if(strncmp(pck->Name, trac->fileRequester, MAX_NAMESIZE) == 0 && data->tracID == trac->tracID && trac->Socket == (uv_tcp_t*)stream){
					if(strncmp((char*)data->data, "OK", 2) == 0){
						trac->confirmed = true;
						trac->socketStatus = SPTP_DATA;
						if(trac->isLink){
							cross_write_req* wreq = (cross_write_req*)malloc(sizeof(cross_write_req));
							char* data = (char*)malloc(sizeof(Packet)+pck->datalen);
							memcpy(data, buf->base, sizeof(Packet)+pck->datalen);
							uv_buf_t buff = uv_buf_init(data, sizeof(Packet)+pck->datalen);
							//wreq->cb = Client::link_write;
							//wreq->handle = (uv_stream_t*)server->client->socket;
							//wreq->data = data;
							//memcpy(&wreq->write_buffer, &buff, sizeof(uv_buf_t));
							wreq->handle = (uv_stream_t*)server->client->socket;
							wreq->buf = buff;
							uv_mutex_lock(&server->client->cross_write_lock);
							server->client->cross_writes.push_back(wreq);
							uv_mutex_unlock(&server->client->cross_write_lock);
							uv_async_send(&server->client->cross_write);
							//sendPck(server->client->socket, NULL, pck->Name, pck->Mode, pck->data, pck->datalen);
						}
						if(int a = nread - (sizeof(Packet)+pck->datalen); a > 0){
							char* data = (char*)malloc(nread - (sizeof(Packet)+pck->datalen));
							memcpy(data, buf->base+(sizeof(Packet)+pck->datalen), nread - (sizeof(Packet)+pck->datalen));
							uv_buf_t buff2 = uv_buf_init(data, nread - (sizeof(Packet)+pck->datalen));
							Server::pckParser(stream, buff2.len, &buff2);
						}
						break;
					}
				}
			}
		}
	}
	free(buf->base);

	return;
}

void Server::tracCheck(uv_check_t *handle){
	//Server* serv = (Server*)proto_getServer();
	Server* serv = (Server*)handle->loop->data;
	if(serv->Traclist.size() != 0){
		for(int a = 0; a < serv->Traclist.size(); a++){
			tracItem* trac = serv->Traclist[a];

			if(trac->lifetime < 0){
				auto it = serv->Traclist.begin();
				serv->Traclist.erase(it+a);
				free(trac);
				continue;
			}

			if(!trac->confirmed){
				trac->lifetime -= 1;
				continue;
			} else if(trac->complete){
				// prepare trac for other requests
				//memset(trac, 0, sizeof(tracItem));
				trac->socketStatus = 0;
				continue;
			}

			if(trac->isLink)
				continue;

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
			uv_buf_t buff = uv_buf_init((char*)data->data, MAX_DATASIZE);
			uv_fs_read(serv->loop, &req, trac->file, &buff, 1, trac->fileOffset, NULL);


			if(req.result < 0){
				log_error("Server failed to read requested file %s.[%s]", trac->fileReq, uv_err_name(req.result));
				log_debug("Server failed to read requested file %s.[%s]", trac->fileReq, uv_strerror(req.result));
			} else if(req.result == 0){
				// were done reading file
				uv_fs_close(serv->loop, &req, trac->file, NULL);
				trac->complete = true;
				strcpy((char*)data->data, "EOF");
				sendPck((uv_stream_t*)trac->Socket, Server::write_cb, (char*)serv->serverName.c_str(), SPTP_DATA, data, sizeof(struct DATA)-(MAX_DATASIZE-3));
				log_info("Total tx: %d", trac->total_transmitted);
			} else {
				trac->fileOffset += req.result;
				trac->total_transmitted += 1;
				data->id = trac->total_transmitted;
				sendPck((uv_stream_t*)trac->Socket, Server::write_cb, (char*)serv->serverName.c_str(), SPTP_DATA, data, sizeof(struct DATA)-(MAX_DATASIZE-req.result));
			} 
			uv_fs_req_cleanup(&req);
			free(data);

		}
	}
}

void Server::on_disconnection(uv_shutdown_t *req, int status){
	Client* client = (Client*)req->data;
	log_info("Client %s has disconnected from server", client->name->c_str());
	uv_close((uv_handle_t*)client->socket, Server::on_close);
	//log_info("Disconnected from network");
	free(req);
	return;
}

void Server::on_close(uv_handle_t *handle){
	Client* client = (Client*)handle->data;
	//Server* server = (Server*)proto_getServer();
	Server* server = (Server*)handle->loop->data;
	free(handle);

	for (auto it = server->Clientlist.begin(); it != server->Clientlist.end(); ++it) {
		if(((Client*)(*it))->name->compare(*client->name) == 0){
			server->Clientlist.erase(it);
			break;
		}
	}

	if(client->name != nullptr){
		delete client->name;
		client->name == nullptr;
	}
	free(client);

}

void Server::write_to_Serv_Sok(uv_async_t* handle){
	Server* serv = (Server*)handle->loop->data;
	uv_mutex_lock(&serv->cross_write_lock);
	auto it = serv->cross_writes.begin();
	int nReqs = serv->cross_writes.size();

	for(int a = 0; a < nReqs; a++){
		cross_write_req* wreq = serv->cross_writes[a]; 
		uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t)); 
		req->data = wreq->buf.base;
		uv_write(req, wreq->handle, &wreq->buf, 1, Server::link_write);
		//serv->cross_writes.erase(it+a);
		free(wreq);
	}
	serv->cross_writes.resize(0);
	uv_mutex_unlock(&serv->cross_write_lock);
}

void Server::link_write(uv_write_t* req, int status){
	Server*  serv = (Server*)req->handle->loop->data;
	if(status < 0){
		log_error("Server[%s] failed writing to other clients: [%s]", serv->serverName.c_str(), uv_err_name(status));
		log_debug("Server[%s] failed writing to other clients: [%s]", serv->serverName.c_str(), uv_strerror(status));
	}
	free(req->data);
	free(req);
}