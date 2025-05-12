#include <stdlib.h>
#include <uv.h>
#include <proto/proto.hpp>

int main(int argc, char** argv){
    Protonet* proto = Init();
    Server* server1;
    Server* server2;
    Client* client;

    uv_fs_t req;
    #ifdef _WIN32
    server2 = new Server("Loopback Pseudo-Interface 1");
    #else
    server2 = new Server("lo");
    #endif

    uv_fs_mkdir(server2->loop, &req, "./server1", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_mkdir(server2->loop, &req, "./client", O_RDWR, NULL);
    uv_fs_req_cleanup(&req);

    #ifdef _WIN32
    server1 = new Server("Loopback Pseudo-Interface 1", "./server1/", 5654, "", "127.0.0.1");
    #else
    server1 = new Server("lo", "./server1/", 5654, "", "127.0.0.1");
    #endif

    #ifdef _WIN32
    client = new Client("Loopback Pseudo-Interface 1", "127.0.0.1", 5654, "", "./client/");
    #else
    client = new Server("lo", "127.0.0.1", 5654, "", "./client/");
    #endif

    uv_fs_rmdir(server2->loop, &req, "./server1", NULL);
    uv_fs_req_cleanup(&req);
    uv_fs_rmdir(server2->loop, &req, "./client", NULL);
    uv_fs_req_cleanup(&req);

    return 0;
}