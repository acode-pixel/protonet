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
	

	Client* test1 = Cl_Init("lo", "bob");
	if(test1 == NULL){
		printf("Client failed to init");
		return -1;
	}

	Server* test2 = Init("lo", "", "dave", ".");
	if(test2 == NULL){
		printf("Server failed to init");
		return -1;
	}

	free(test1);
	free(test2);

	printf("All tests ok");	
	return 0;
}
