#include <stdlib.h>
#include <uv.h>
#include "proto/proto.hpp"

int main(int argc, char** argv){
    Protonet* p = Init();
    Server* server;
    Client* client;

    #ifdef _WIN32
        server = new Server("Loopback Pseudo-Interface 1", ".", S_PORT, "server");
    #else
        server = new Server("lo", ".", S_PORT, "server");
    #endif

    uv_fs_t req;

    uv_fs_open(server->loop, &req, "./test.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
    int fd = req.result;
    uv_fs_req_cleanup(&req);
    char data[] = "Test baby woooooooo";
    uv_buf_t buf = uv_buf_init(data, strlen(data));
    uv_fs_write(server->loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_close(server->loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);

    uv_fs_open(server->loop, &req, "./test2.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
    fd = req.result;
    uv_fs_req_cleanup(&req);
    char data2[] = "Test2 baby woooooooo";
    buf = uv_buf_init(data2, strlen(data2));
    uv_fs_write(server->loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_close(server->loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);

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
    client->makeFileReq("test2.txt");
    uv_fs_access(client->loop, &req, "./test_dir/test2.txt", F_OK, NULL);

    if(req.result){
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client->loop, &req, "./test_dir/test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_rmdir(client->loop, &req, "./test_dir", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client->loop, &req, "./test.txt", NULL);
        uv_fs_req_cleanup(&req);
        uv_fs_unlink(client->loop, &req, "./test2.txt", NULL);
        uv_fs_req_cleanup(&req);
        return -1;
    }

    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test_dir/test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test_dir/test2.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(client->loop, &req, "./test_dir", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test.txt", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_unlink(client->loop, &req, "./test2.txt", NULL);
    uv_fs_req_cleanup(&req);
    return 0;
}