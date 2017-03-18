#include <stdlib.h>
#include <stdio.h>

#include "appendserver.h"
#include "verifyserver.h"


void InitAppendServer(char* hostname, AppendArgs args)
{
    CLIENT *clnt;
    int  *result;

    clnt = clnt_create(hostname, RPC_AppendServer, RPC_AppendServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(hostname);
        exit(1);
    }

    result = rpc_initappendserver_1(&args, clnt);

    if (result == NULL) {
        clnt_perror(clnt, hostname);
    }
    else {
        printf("Result: %d\n", *result);
    }

    clnt_destroy(clnt);
}

int main(int argc, char **argv)
{
    char *hostname1 = "localhost";
    char *hostname2 = "verifyserver";

    AppendArgs args;
	args.property = 0;
	args.segLength = 1;
	args.numSegments = 1;
	args.c0 = 'a';
    args.c1 = 'b';
    args.c2 = 'c';

    InitAppendServer(hostname1, args);

    return 0;
}