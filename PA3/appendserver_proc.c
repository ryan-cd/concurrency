#include <stdio.h>
#include "appendserver.h"

int *rpc_initappendserver_1_svc(AppendArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitAppendServer\n");

	return &result;
}

int *rpc_append_1_svc(char *letter, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_Append\n");

	return &result;
}