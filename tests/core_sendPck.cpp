#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "proto/proto.hpp"



uv_thread_t thread1;
uv_async_t async;
int totalRead = 0;

void write_cb(uv_write_t *req, int status){
    if(status < 0){
        printf("write error: %s\n", uv_strerror(status));
        exit(-1);
    }
    free(req);
}

void on_connect(uv_connect_t *req, int status){
    if (status < 0){
        printf("Connection error: %s\n", uv_strerror(status));
        exit(-1);
    }

    sendPck(req->handle, write_cb, "bob", 1, (void *)"packet test", 0);
    free(req);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
    if(nread > 0){
        printf("read bytes: %d\n", nread);
        Packet* pck = (Packet*)buf->base;
        printf("Data size: %d\n", pck->datalen);
        printf("Data: %s\n", pck->data);
        uv_stop(stream->loop);
    } else if(nread < 0) {
        printf("ERROR: %s", uv_strerror(nread));
        exit(-1);
    }
    free(buf->base);
}

void on_connection(uv_stream_t *server, int status){

    if(status < 0){
        printf("Error accepting connection: %s", uv_strerror(status));
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);
    if(uv_accept(server, (uv_stream_t*)client) == 0){
        uv_read_start((uv_stream_t*)client, alloc_cb, read_cb);
    }
}

void connectToServer(uv_async_t* handle){
    uv_loop_t* loop1 = (uv_loop_t*)handle->data;
    uv_tcp_t* tcpHandle1 = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    sockaddr_in addr;
    uv_connect_t* req = (uv_connect_t*)malloc(sizeof(uv_connect_t));

    uv_ip4_addr("127.0.0.1", 5657, &addr);
    uv_tcp_init(loop1, tcpHandle1);
    uv_tcp_connect(req, tcpHandle1, (struct sockaddr*)&addr, on_connect);
}

void client_thread(void* arg){
    uv_loop_t* loop1 = (uv_loop_t*)arg;

    printf("Statrting thread [%s] %lu\n", (char*)(static_cast<uv_loop_t*>(arg)->data), uv_thread_self());
    async.data = loop1;
    uv_async_init(loop1, &async, connectToServer);
    uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
}

void sendSignal(uv_prepare_t* handle){
    uv_async_send(&async);
    uv_prepare_stop(handle);
}

void server_thread(void* arg) {
    sockaddr_in addr;
    printf("Statrting thread [%s] %lu\n", (char*)(static_cast<uv_loop_t*>(arg)->data), uv_thread_self());
    uv_loop_t* loop2 = (uv_loop_t*)arg;
    uv_prepare_t prep;
    uv_tcp_t Server;

    uv_prepare_init(loop2, &prep);
    uv_prepare_start(&prep, sendSignal);
    uv_tcp_init(loop2, &Server);
    uv_ip4_addr("127.0.0.1", 5657, &addr);
    uv_tcp_bind(&Server, (struct sockaddr*)&addr, 0);
    uv_listen((uv_stream_t*)&Server, 10, on_connection);
    uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
}

int main(int argc, char** argv){
    uv_loop_t loop1, loop2;
    uv_loop_init(&loop1);
    uv_loop_init(&loop2);

    uint64_t time = uv_hrtime();

    uv_loop_set_data(&loop1, (void*)"client");
    uv_loop_set_data(&loop2, (void*)"server");

    uv_thread_t thread2;
    uv_thread_create(&thread1, client_thread, &loop1);
    uv_sleep(1000);
    uv_thread_create(&thread2, server_thread, &loop2);
    uv_thread_join(&thread2);
}