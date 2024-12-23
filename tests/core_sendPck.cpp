#include <stdlib.h>
#include <string.h>
#include "client.hpp"
#include <uv.h>

extern "C" {
    #include "core.h"
}

uv_tcp_t Server;
Protonet* loop1;
uv_thread_t thread1;
uv_cond_t cond1, cond2;
uv_mutex_t mutex;

void on_connect(uv_connect_t *req, int status){
    sendPck(req->handle, NULL, "bob", 1, (void*)"packet test", 0);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(1024);
  buf->len = 1024;
}

void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
    uv_mutex_lock(&mutex);
    if(nread > 0){
        uv_cond_signal(&cond2);
    } else if(nread < 0) {
        printf("ERROR: %s", uv_strerror(nread));
        exit(-1);
    }
    free(buf->base);
    uv_mutex_unlock(&mutex);
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
    printf("Statrting thread [%s] %lu\n", (char*)(static_cast<uv_loop_t*>(arg)->data), uv_thread_self());
    if(strcmp((char*)(static_cast<uv_loop_t*>(arg)->data), "client") == 0){
        uv_loop_t* loop1 = (uv_loop_t*)arg;
        uv_connect_t req;
        uv_tcp_t tcpHandle1;

        uv_mutex_lock(&mutex);
        uv_ip4_addr("127.0.0.1", 5657, &addr);
        uv_tcp_init(loop1, &tcpHandle1);
        uv_tcp_connect(&req, &tcpHandle1, (struct sockaddr*)&addr, on_connect);
        uv_mutex_unlock(&mutex);
        uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
    } else if(strcmp((char*)(static_cast<uv_loop_t*>(arg)->data), "server") == 0){
        uv_loop_t* loop2 = (uv_loop_t*)arg;

        uv_mutex_lock(&mutex);
        uv_tcp_init(loop2, &Server);
        uv_ip4_addr("127.0.0.1", 5657, &addr);
        uv_tcp_bind(&Server, (struct sockaddr*)&addr, 0);
        uv_listen((uv_stream_t*)&Server, 10, on_connection);
        uv_cond_signal(&cond1);
        uv_mutex_unlock(&mutex);
        uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
    }
}

int main(int argc, char** argv){
    loop1 = Init();
    Protonet* loop2 = Init();

    uint64_t time = uv_hrtime();
    uv_cond_init(&cond1);
    uv_cond_init(&cond2);

    uv_loop_set_data(loop1->loop, (void*)"client");
    uv_loop_set_data(loop2->loop, (void*)"server");

    uv_thread_t thread2;
    uv_mutex_lock(&mutex);
    uv_thread_create(&thread2, loop_thread, loop2->loop);
    uv_sleep(250);
    uv_cond_wait(&cond1, &mutex);
    uv_thread_create(&thread1, loop_thread, loop1->loop);
    uv_sleep(250);
    uv_cond_wait(&cond2, &mutex);
    exit(0);
}