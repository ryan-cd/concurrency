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

void threadFunc(char *hostname1, char *hostname2, int thread)
{
    // Connect to RPC_AppendServer
    CLIENT *appendClient = appendClient = clnt_create(hostname1, RPC_AppendServer, RPC_AppendServer_VERS, "udp");
    if (appendClient == NULL) {
        clnt_pcreateerror(hostname1);
        printf("Thread #%d: unable to connect to AppendServer!\n", thread);
        exit(1);
    }
    // Connect to RPC_VerifyServer
    CLIENT *verifyClient = verifyClient = clnt_create(hostname2, RPC_VerifyServer, RPC_VerifyServer_VERS, "udp");
    if (verifyClient == NULL) {
        clnt_pcreateerror(hostname2);
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

void printInstructions() {
    fprintf(stderr,
        "\nUsage: ./client i N L M c_0 c_1 c_2 hostname_1 hostname_2\n"
        "Parameters:\n"
        "\t i:   (0<=i<=3) The index of the property Fi which each segment of S needs to satisfy.\n"
        "\t N:   (3<=N<=8) The number of threads.\n"
        "\t L:   (0 < L)   The length of each segment of S.\n"
        "\t M:   (0 < M)   The number of segments in S to generate.\n"
        "\t c_i: (0<=i<=2) The letters to be used in the property check.\n"
        "\t hostname_1:    The hostname of the Append server.\n"
        "\t hostname_2:    The hostname of the Verify server.\n"
        "Example: ./client 0 3 6 3 a b c mills moore\n");
}

int main(int argc, char **argv)
{
    if (argc < 7)
    {
        printInstructions();
        exit(1);
    }
    int numThreads = strtol(argv[2], NULL, 10);
    char *hostname1 = argv[8];
    char *hostname2 = argv[9];

    AppendArgs appendArgs;
    appendArgs.property = strtol(argv[1], NULL, 10);
    appendArgs.segLength = strtol(argv[3], NULL, 10);
    appendArgs.numSegments = strtol(argv[4], NULL, 10);
    appendArgs.c0 = *argv[5];
    appendArgs.c1 = *argv[6];
    appendArgs.c2 = *argv[7];

    VerifyArgs verifyArgs;
    verifyArgs.numThreads = numThreads;
    verifyArgs.segLength = appendArgs.segLength;
    verifyArgs.numSegments = appendArgs.numSegments;

    InitAppendServer(hostname1, appendArgs);
    InitVerifyServer(hostname1, verifyArgs);

    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < numThreads; ++i) {
        threadFunc(hostname1, hostname2, i);
    }

    return 0;
}