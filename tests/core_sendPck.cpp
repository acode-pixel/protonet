#include <stdlib.h>
#include <string.h>
#include "client.hpp"
#include <uv.h>

extern "C" {
    #include "core.h"
}

uv_tcp_t Server;

void on_connect(uv_connect_t *req, int status){
    sendPck(req->handle, NULL, "bob", 1, (void*)"packet test", 0);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(1024);
  buf->len = 1024;
}

void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
    if(nread > 0){
        exit(0);
    } else {
        exit(-1);
    }
}

void on_connection(uv_stream_t *server, int status){
    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);
    if(uv_accept(server, (uv_stream_t*)client) == 0){
        uv_read_start((uv_stream_t*)client, alloc_cb, read_cb);
    }
}

void loop_thread(void* arg) {
    sockaddr_in addr;
    if(strcmp((char*)(static_cast<uv_loop_t*>(arg)->data), "client") == 0){
        uv_loop_t* loop1 = (uv_loop_t*)arg;
        uv_connect_t req;
        uv_tcp_t tcpHandle1;

        uv_ip4_addr("0.0.0.0", 5657, &addr);
        uv_tcp_init(loop1, &tcpHandle1);
        uv_tcp_connect(&req, &tcpHandle1, (struct sockaddr*)&addr, on_connect);
    } else if(strcmp((char*)(static_cast<uv_loop_t*>(arg)->data), "server") == 0){
        uv_loop_t* loop2 = (uv_loop_t*)arg;

        uv_tcp_init(loop2, &Server);
        uv_ip4_addr("0.0.0.0", 5657, &addr);
        uv_tcp_bind(&Server, (struct sockaddr*)&addr, NULL);
        uv_listen((uv_stream_t*)&Server, 10, on_connection);
    }

    uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
}

int main(int argc, char** argv){
    Protonet* loop1 = Init();
    Protonet* loop2 = Init();

    uint64_t time = uv_hrtime();

    uv_loop_set_data(loop1->loop, (void*)"client");
    uv_loop_set_data(loop2->loop, (void*)"server");

    uv_thread_t thread1, thread2;
    uv_thread_create(&thread2, loop_thread, loop2->loop);
    uv_thread_create(&thread1, loop_thread, loop1->loop);

    uv_thread_join(&thread1);
    uv_thread_join(&thread2);
}