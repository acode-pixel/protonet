#ifndef CLIENT_WRAPPER
#define CLIENT_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif

void* Client_new(char* inter, char name[], char* IP);           // Create a new MyClass object
void Client_delete(void* client); // Call printMessage method

#ifdef __cplusplus
}
#endif

#endif