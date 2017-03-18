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

void InitVerifyServer(char* hostname, VerifyArgs args)
{
    CLIENT *clnt;
    int  *result;

    clnt = clnt_create(hostname, RPC_VerifyServer, RPC_VerifyServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(hostname);
        exit(1);
    }

    result = rpc_initverifyserver_1(&args, clnt);

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

    VerifyArgs vArgs;
    vArgs.numThreads = 3;
    vArgs.segLength = 6;
    vArgs.numSegments = 3;

    InitAppendServer(hostname1, args);
    InitVerifyServer(hostname1, vArgs);

    CLIENT *clnt;
    int  *result;

    clnt = clnt_create(hostname1, RPC_VerifyServer, RPC_VerifyServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(hostname1);
        exit(1);
    }
    char letter = 'a';
    result = rpc_append_1(&letter, clnt);
    if (result == NULL) {
        clnt_perror(clnt, hostname1);
    }
    else {
        printf("Result of sending a character: %d\n", *result);
    }

    return 0;
}

