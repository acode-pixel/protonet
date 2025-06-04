#ifndef CLIENT_WRAPPER
#define CLIENT_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif

MYLIB_API void* Client_new(char* inter, char* IP, int serverPort, char name[], char outpath[]);
MYLIB_API void Client_delete(void* client);

#ifdef __cplusplus
}
#endif

#endif