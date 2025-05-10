#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

int main(int argc, char** argv){
    Protonet* p = Init();
    Server* server;
    Client* client;

    #ifdef _WIN32
        server = new Server("Loopback Pseudo-Interface 1");
    #else
        server = new Server("lo");
    #endif

    uv_fs_t req;

    uv_fs_open(server->loop, &req, "./test.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
    int fd = req.result;
    uv_fs_req_cleanup(&req);
    int i = 1600;
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

        #ifdef _WIN32
        client = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", S_PORT, "", "./test_dir/");
    #else
        client = new Client("lo", "127.0.0.1", S_PORT, "", "./test_dir/");
    #endif

    client->makeFileReq("test.txt");

    //uv_sleep(9000);
    uv_fs_access(client->loop, &req, "./test_dir/test.txt", F_OK, NULL);

    if(req.result){
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client->loop, &req, "./test_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client->loop, &req, "./test_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client->loop, &req, "./test.txt", NULL);
        uv_fs_req_cleanup(&req);
        return -1;
    }

    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client->loop, &req, "./test_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test.txt", NULL);
    uv_fs_req_cleanup(&req);
    return 0;
}