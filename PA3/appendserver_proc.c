#include <stdio.h>
#include <pthread.h>
#include "appendserver.h"

/* String */
typedef struct {
    char* str;
    unsigned int length;
    unsigned int index;
} Str;

/* Globals */
Str string;

/* RPC Procedure: int RPC_InitAppendServer(AppendArgs) */
int *rpc_initappendserver_1_svc(AppendArgs *args, struct svc_req *req)
{
	static int result = 0;
	printf("RPC_InitAppendServer\n");

	// Init string
	string.length = args->numSegments * args->segLength;
	string.str = calloc(string.length + 1, sizeof(char));
	string.index = 0;

	return &result;
}

/* RPC Procedure: int RPC_Append(char) */
int *rpc_append_1_svc(char *letter, struct svc_req *req)
{
	static int result = 0;
	printf("RPC_Append\n");
	printf("string: %s\n", string);

	// Append letter if there is space
	if (string.index < string.length) {
		string.str[string.index++] = *letter;
	} else {
		result = -1;
	}

	return &result;
}
