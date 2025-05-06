#ifndef SERVER_WRAPPER
#define SERVER_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif

void* Server_new(char* inter, char* serverName, char Dir[], char* peerIp);           // Create a new MyClass object
void Server_delete(void* server); // Call printMessage method

#ifdef __cplusplus
}
#endif

#endif