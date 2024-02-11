#include <cstdlib>
#include <cstdbool>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>

extern "C" {
	#include "client.h"
	#include "server.h"
	#include "core.h"
};

#ifdef __APPLE__
	#define inter "lo0"
#elif __unix__
	#define inter "lo"
#endif

int main(int argc, char* argv[]){
	
	Init();
	Client* test1 = Cl_Init(inter, "bob");

	if(test1 == NULL){
		printf("Client failed to init");
		return -1;
	}

	Server* test2 = S_Init(inter, "", "dave", ".");
	
	if(test2 == NULL){
		printf("Server failed to init");
		return -1;
	}

	free(test1);
	free(test2);

	Stop();

	printf("All tests ok");	
	return 0;
}
