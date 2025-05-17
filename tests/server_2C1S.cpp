#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

uv_barrier_t bar;
Client* client1;
Client* client2;

void make_Reqs(void* arg){
    if(strcmp((char*)arg, "1") == 0){
        client1->makeFileReq("test.txt");
    } else {
        client2->makeFileReq("test.txt");
    }
    uv_barrier_wait(&bar);
}

int main(int argc, char** argv){
    Protonet* p = Init();
    Server* server;

    #ifdef _WIN32
        server = new Server("Loopback Pseudo-Interface 1");
    #else
        server = new Server("lo");
    #endif

    uv_fs_t req;

    uv_fs_open(server->loop, &req, "./test.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
    int fd = req.result;
    uv_fs_req_cleanup(&req);
    int i = 191;
    char* data = (char*)malloc(65536*i);
    uv_random(NULL, NULL, data, 65536*i, 0, NULL);
    uv_buf_t buf = uv_buf_init(data, 65536*i);
    uv_fs_write(server->loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_close(server->loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);
    free(data);

    uv_fs_mkdir(server->loop, &req, "./test_dir", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server->loop, &req, "./test_dir", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);

    uv_fs_mkdir(server->loop, &req, "./test2_dir", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server->loop, &req, "./test2_dir", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);

        #ifdef _WIN32
        client1 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "", "./test_dir/");
        client2 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "", "./test2_dir/");
    #else
        client1 = new Client("lo", "127.0.0.1", S_PORT, "", "./test_dir/");
        client2 = new Client("lo", "127.0.0.1", S_PORT, "", "./test2_dir/");
    #endif

    uv_barrier_init(&bar, 2);

    uv_thread_t tid;
    uv_thread_create(&tid, make_Reqs, (void*)"1");
    make_Reqs((char*)"2");

    uv_thread_join(&tid);

    //uv_sleep(9000);
    uv_fs_t req2;
    uv_fs_access(client1->loop, &req, "./test_dir/test.txt", F_OK, NULL);
    uv_fs_access(client1->loop, &req2, "./test2_dir/test.txt", F_OK, NULL);

    if(req.result || req2.result){
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test2_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test2_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test.txt", NULL);
        uv_fs_req_cleanup(&req);
        return -1;
    }

    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client1->loop, &req, "./test_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client1->loop, &req, "./test_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client1->loop, &req, "./test2_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client1->loop, &req, "./test2_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client1->loop, &req, "./test.txt", NULL);
    uv_fs_req_cleanup(&req);
    return 0;
}