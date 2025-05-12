#ifndef SERVER_WRAPPER
#define SERVER_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif

void* Server_new(char* inter, char Dir[], int port, char* serverName, char* peerIp, int peerPort);           // Create a new MyClass object
void Server_delete(void* server);

#ifdef __cplusplus
}
#endif

#endif