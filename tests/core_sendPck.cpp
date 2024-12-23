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

void write_cb(uv_write_t *req, int status){
    if(status < 0){
        printf("write error: %s\n", uv_strerror(status));
    }
}

void on_connect(uv_connect_t *req, int status){
    if (status < 0){
        printf("Connection error: %s\n", uv_strerror(status));
    }
    sendPck(req->handle, write_cb, "bob", 1, (void*)"packet test", 0);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(1024);
  buf->len = 1024;
}

void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
    if(nread > 0){
        uv_cond_signal(&cond2);
        printf("Signal cond2 sent from server\n");
    } else if(nread < 0) {
        printf("ERROR: %s", uv_strerror(nread));
        exit(-1);
    }
    free(buf->base);
}

void on_connection(uv_stream_t *server, int status){
    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);
    if(uv_accept(server, (uv_stream_t*)client) == 0){
        uv_read_start((uv_stream_t*)client, alloc_cb, read_cb);
    }
}

void client_thread(void* arg){
    sockaddr_in addr;
    uv_loop_t* loop1 = (uv_loop_t*)arg;
    uv_connect_t req;
    uv_tcp_t tcpHandle1;

    printf("Statrting thread [%s] %lu\n", (char*)(static_cast<uv_loop_t*>(arg)->data), uv_thread_self());
    printf("client thread wants lock\n");
    uv_mutex_lock(&mutex);
    printf("client thread has lock\n");
    uv_ip4_addr("127.0.0.1", 5657, &addr);
    uv_tcp_init(loop1, &tcpHandle1);
    uv_tcp_connect(&req, &tcpHandle1, (struct sockaddr*)&addr, on_connect);
    uv_mutex_unlock(&mutex);
    printf("client thread releases lock\n");
    uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
}

void server_thread(void* arg) {
    sockaddr_in addr;
    printf("Statrting thread [%s] %lu\n", (char*)(static_cast<uv_loop_t*>(arg)->data), uv_thread_self());
    uv_loop_t* loop2 = (uv_loop_t*)arg;

    printf("server thread wants lock\n");
    uv_mutex_lock(&mutex);
    printf("server thread has lock\n");
    uv_tcp_init(loop2, &Server);
    uv_ip4_addr("127.0.0.1", 5657, &addr);
    uv_tcp_bind(&Server, (struct sockaddr*)&addr, 0);
    uv_listen((uv_stream_t*)&Server, 10, on_connection);
    uv_cond_signal(&cond1);
    printf("Signal cond1 sent from server\n");
    uv_mutex_unlock(&mutex);
    printf("server thread releases lock\n");
    uv_run(static_cast<uv_loop_t*>(arg), UV_RUN_DEFAULT);
}

int main(int argc, char** argv){
    loop1 = Init();
    Protonet* loop2 = Init();

    uint64_t time = uv_hrtime();
    uv_cond_init(&cond1);
    uv_cond_init(&cond2);
    uv_mutex_init(&mutex);

    uv_loop_set_data(loop1->loop, (void*)"client");
    uv_loop_set_data(loop2->loop, (void*)"server");

    uv_thread_t thread2;
    printf("Main thread wanting lock\n");
    uv_mutex_lock(&mutex);
    printf("Main thread has lock\n");
    uv_thread_create(&thread2, server_thread, loop2->loop);
    //uv_sleep(250);
    printf("Main thread releases lock\n");
    uv_cond_wait(&cond1, &mutex);
    printf("Main thread has lock\n");
    uv_thread_create(&thread1, client_thread, loop1->loop);
    //uv_sleep(250);
    printf("Main thread releases lock\n");
    uv_cond_wait(&cond2, &mutex);
    printf("Main thread has lock\n");
    exit(0);
}