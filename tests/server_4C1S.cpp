#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

Client* client1;
Client* client2;
Client* client3;
Client* client4;

uv_thread_t tid1, tid2, tid3, tid4;

void make_Reqs(void* arg){
    uv_thread_t tid = uv_thread_self();
    if(uv_thread_equal(&tid, &tid1)){
        client1->makeFileReq("test.txt");
    } else if(uv_thread_equal(&tid, &tid2)) {
        client2->makeFileReq("test.txt");
    } else if(uv_thread_equal(&tid, &tid3)) {
        client3->makeFileReq("test.txt");
    } else if(uv_thread_equal(&tid, &tid4)) {
        client4->makeFileReq("test.txt");
    } 
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

    uv_fs_mkdir(server->loop, &req, "./test3_dir", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server->loop, &req, "./test3_dir", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);

    uv_fs_mkdir(server->loop, &req, "./test4_dir", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server->loop, &req, "./test4_dir", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);

        #ifdef _WIN32
        client1 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "client1", "./test_dir/");
        client2 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "client2", "./test2_dir/");
        client3 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "client3", "./test3_dir/");
        client4 = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "client4", "./test4_dir/");
    #else
        client1 = new Client("lo", "127.0.0.1", S_PORT, "client1", "./test_dir/");
        client2 = new Client("lo", "127.0.0.1", S_PORT, "client2", "./test2_dir/");
        client3 = new Client("lo", "127.0.0.1", S_PORT, "client3", "./test3_dir/");
        client4 = new Client("lo", "127.0.0.1", S_PORT, "client4", "./test4_dir/");
    #endif

    tid1 = uv_thread_self();
    uv_thread_create(&tid2, make_Reqs, NULL);
    uv_thread_create(&tid3, make_Reqs, NULL);
    uv_thread_create(&tid4, make_Reqs, NULL);
    make_Reqs(NULL);

    uv_thread_join(&tid2);
    uv_thread_join(&tid3);
    uv_thread_join(&tid4);

    //uv_sleep(9000);
    uv_fs_t req2, req3, req4;
    uv_fs_access(client1->loop, &req, "./test_dir/test.txt", F_OK, NULL);
    uv_fs_access(client1->loop, &req2, "./test2_dir/test.txt", F_OK, NULL);
    uv_fs_access(client1->loop, &req3, "./test3_dir/test.txt", F_OK, NULL);
    uv_fs_access(client1->loop, &req4, "./test4_dir/test.txt", F_OK, NULL);

    if(req.result || req2.result || req3.result || req4.result){
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test2_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test2_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test3_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test3_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client1->loop, &req, "./test4_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client1->loop, &req, "./test4_dir", NULL);
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
    uv_fs_unlink(client1->loop, &req, "./test3_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client1->loop, &req, "./test3_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client1->loop, &req, "./test4_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client1->loop, &req, "./test4_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client1->loop, &req, "./test.txt", NULL);
    uv_fs_req_cleanup(&req);
    return 0;
}