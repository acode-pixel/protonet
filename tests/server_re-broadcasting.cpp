#include <stdlib.h>
#include <uv.h>
#include <proto/proto.hpp>

int main(int argc, char** argv){
    Protonet* proto = Init();
    Server* server1;
    Server* server2;
    Server* server3;
    Client* client;

    uv_fs_t req;
    #ifdef _WIN32
    server2 = new Server("Loopback Pseudo-Interface 1");
    #else
    server2 = new Server("lo");
    #endif

    uv_fs_mkdir(server2->loop, &req, "./server1", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server2->loop, &req, "./server1", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_mkdir(server2->loop, &req, "./server3", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server2->loop, &req, "./server3", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_mkdir(server2->loop, &req, "./client", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_chmod(server2->loop, &req, "./client", S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH, NULL);
    uv_fs_req_cleanup(&req);

    uv_fs_open(server2->loop, &req, "./test.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
    int fd = req.result;
    uv_fs_req_cleanup(&req);
    int i = 191;
    char* data = (char*)malloc(65536*i);
    uv_random(NULL, NULL, data, 65536*i, 0, NULL);
    uv_buf_t buf = uv_buf_init(data, 65536*i);
    uv_fs_write(server2->loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_close(server2->loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);

    #ifdef _WIN32
    server1 = new Server("Loopback Pseudo-Interface 1", "./server1/", 5654, "", "127.0.0.1");
    server3 = new Server("Loopback Pseudo-Interface 1", "./server3/", 5653, "", "127.0.0.1", 5654);
    #else
    server1 = new Server("lo", "./server1/", 5654, "", "127.0.0.1");
    server3 = new Server("lo", "./server3/", 5653, "", "127.0.0.1", 5654);
    #endif

    #ifdef _WIN32
    client = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", 5653, "", "./client/");
    #else
    client = new Client("lo", "127.0.0.1", 5653, "", "./client/");
    #endif

    client->makeFileReq("test.txt");

    uv_fs_rmdir(server2->loop, &req, "./server1", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(server2->loop, &req, "./server3", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(server2->loop, &req, "./client", NULL);
    uv_fs_req_cleanup(&req);

    return 0;
}