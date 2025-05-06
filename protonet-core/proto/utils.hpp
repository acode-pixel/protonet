#ifndef UTILS_H
#define UTILS_H

#include <string>

extern "C" {
    #include "./log.h"
}


using namespace std;

int getFileHashSHA256(char* filepath, uv_loop_t* loop, void* out);
void getHex(void* hex, int size, char* out);

#endif