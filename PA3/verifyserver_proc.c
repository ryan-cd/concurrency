#include <stdio.h>
#include "verifyserver.h"

int *rpc_initverifyserver_1_svc(char **args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitVerifyServer\n");

	return &result;
}

int *rpc_getseg_1_svc(int *thread, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_GetSeg\n");

	return &result;
}