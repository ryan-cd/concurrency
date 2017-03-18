#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // usleep
#include <omp.h> // openmp

#include "appendserver.h"
#include "verifyserver.h"

void InitAppendServer(char *hostname, AppendArgs args)
{
    // Connect to RPC_AppendServer
    CLIENT *clnt = clnt_create(hostname, RPC_AppendServer, RPC_AppendServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror("clnt_create");
        printf("Exiting!\n");
        exit(1);
    }
    // Call RPC_InitAppendServer
    int *result = rpc_initappendserver_1(&args, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "rpc_initappendserver_1");
        printf("Exiting!\n");
        exit(1);
    }
    clnt_destroy(clnt);
}

void InitVerifyServer(char *hostname, VerifyArgs args)
{
    // Connect to RPC_VerifyServer
    CLIENT *clnt = clnt_create(hostname, RPC_VerifyServer, RPC_VerifyServer_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(hostname);
        printf("Exiting!\n");
        exit(1);
    }
    // Call RPC_InitVerifyServer
    int *result = rpc_initverifyserver_1(&args, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "rpc_initverifyserver_1");
        printf("Exiting!\n");
        exit(1);
    }
    clnt_destroy(clnt);
}

void threadFunc(char *hostname, int thread)
{
    // Connect to RPC_AppendServer
    CLIENT *appendClient = appendClient = clnt_create(hostname, RPC_AppendServer, RPC_AppendServer_VERS, "udp");
    if (appendClient == NULL) {
        clnt_pcreateerror(hostname);
        printf("Thread #%d: unable to connect to AppendServer!\n", thread);
        exit(1);
    }
    // Connect to RPC_VerifyServer
    CLIENT *verifyClient = verifyClient = clnt_create(hostname, RPC_VerifyServer, RPC_VerifyServer_VERS, "udp");
    if (verifyClient == NULL) {
        clnt_pcreateerror(hostname);
        printf("Thread #%d: unable to connect to AppendServer!\n", thread);
        exit(1);
    }

    // Append letters until the string is completely built
    char letter = 'a' + thread;
    int *result = NULL;
    while (1) {
        // Sleep for a random period between 100ms and 500ms
        unsigned int microseconds = (rand() % (500000 + 1 - 100000)) + 100000;
        usleep(microseconds);
        // Append letter
        result = rpc_append_1(&letter, appendClient);
        if (result == NULL) {
            clnt_perror(appendClient, "Unable to append");
            break;
        } else {
            if (*result == -1) {
                break;
            }
        }
    }

    // Verify the string
    // TODO

    // Clean up
    clnt_destroy(appendClient);
    clnt_destroy(verifyClient);
}

int main(int argc, char **argv)
{
    int numThreads = 4;
    char *hostname1 = "localhost";
    char *hostname2 = "verifyserver";

    AppendArgs args;
	args.property = 0;
	args.segLength = 4;
	args.numSegments = 20;
	args.c0 = 'a';
    args.c1 = 'b';
    args.c2 = 'c';

    VerifyArgs vArgs;
    vArgs.numThreads = numThreads;
    vArgs.segLength = 6;
    vArgs.numSegments = 3;

    InitAppendServer(hostname1, args);
    InitVerifyServer(hostname1, vArgs);

    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < numThreads; ++i) {
        threadFunc(hostname1, i);
    }

    return 0;
}