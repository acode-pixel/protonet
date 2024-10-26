#ifndef CLIENT_WRAPPER
#define CLIENT_WRAPPER

#ifdef __cplusplus
extern "C" {
    #include <uv.h>
#else
    #include <uv.h>
#endif
void* Client_new(char* inter, char name[], uv_loop_t* loop);           // Create a new MyClass object
void Client_delete(void* client); // Call printMessage method

#ifdef __cplusplus
}
#endif

#endif