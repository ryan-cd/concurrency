#include <stdlib.h>
#include <stdio.h>

#include "appendserver.h"
#include "verifyserver.h"

void InitAppendServer(char *hostname, AppendArgs args)
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

void InitVerifyServer(char *hostname, VerifyArgs args)
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

int Append(char *hostname, char letter)
{
    CLIENT *clnt;
    int  *result;

    clnt = clnt_create(hostname, RPC_AppendServer, RPC_AppendServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(hostname);
        exit(1);
    }

    result = rpc_append_1(&letter, clnt);

    if (result == NULL) {
        clnt_perror(clnt, hostname);
    }

    clnt_destroy(clnt);

    return *result;
}

void threadFunc(char *hostname, int thread)
{
    int letter = 'a' + thread;
    int appendResult = 0;
    while (appendResult == 0) {
        // Sleep for a random period between 100ms and 500ms.
        unsigned int microseconds = (rand() % (500000 + 1 - 100000)) + 100000; // Biased due to modulus.
        usleep(microseconds);
        // Append letter
        appendResult = Append(hostname, letter);
    }
}

int main(int argc, char **argv)
{
    char *hostname1 = "localhost";
    char *hostname2 = "verifyserver";

    AppendArgs args;
	args.property = 0;
	args.segLength = 4;
	args.numSegments = 2;
	args.c0 = 'a';
    args.c1 = 'b';
    args.c2 = 'c';

    VerifyArgs vArgs;
    vArgs.numThreads = 3;
    vArgs.segLength = 6;
    vArgs.numSegments = 3;

    InitAppendServer(hostname1, args);
    InitVerifyServer(hostname1, vArgs);

    // Testing
    threadFunc(hostname1, 0);


    return 0;
}

