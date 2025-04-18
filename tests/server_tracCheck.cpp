#include <stdlib.h>
#include "core.h"
#include "client.hpp"
#include "server.hpp"
#include <uv.h>

int main(int argc, char** argv){
    Protonet* p = Init();
    Server* server;
    Client* client;

    #ifdef _WIN32
        server = new Server("Loopback Pseudo-Interface 1", "server", ".", "");
    #else
        server = new Server("lo", "server", ".", "");
    #endif

    uv_fs_t req;

    uv_fs_open(server->loop, &req, "./test.txt", O_WRONLY | O_CREAT, 0, NULL);
    int fd = req.result;
    uv_fs_req_cleanup(&req);
    char data[] = "Test baby woooooooo";
    uv_buf_t buf = uv_buf_init(data, strlen(data));
    uv_fs_write(server->loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_close(server->loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);

    uv_fs_mkdir(server->loop, &req, "./test_dir", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);

        #ifdef _WIN32
        client = new Client("Loopback Pseudo-Interface 1", "client", "127.0.0.1", "./test_dir/");
    #else
        client = new Client("lo", "client", "127.0.0.1", "./test_dir/");
    #endif

    client->makeFileReq("test.txt");
    uv_sleep(2000);
    if(!client->trac.canDelete)
        return -1;
    
    return 0;
}