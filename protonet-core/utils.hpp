#ifndef UTILS_H
#define UTILS_H

extern "C" {
    #include "./log.h"
}

#include <uv.h>
#include <string>
#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace std;

int getFileHashSHA256(char* filepath, uv_loop_t* loop, void* out);
void getHex(void* hex, int size, char* out);

#endif