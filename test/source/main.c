#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include "client.h"
#include "server.h"
#include "core.h"

int main(int argc, char* argv[]){
	

	Client* test1 = Cl_Init(argv[2], argv[3]);
	if(test1 == NULL)
		printf("Client failed to init");
		return -1;

	Server* test2 = test2 = Init(argv[2], argv[3], argv[4], argv[5]);
	if(test2 == NULL)
		printf("Server failed to init");
		return -1;

	printf("All tests ok");	
	return 0;
}
