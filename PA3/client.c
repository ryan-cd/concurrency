#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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

size_t threadFunc(char *hostname1, char *hostname2, int thread)
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
    size_t numSegmentsValid = 1;

    // Clean up
    clnt_destroy(appendClient);
    clnt_destroy(verifyClient);

    return numSegmentsValid;
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
    size_t property = 0; // i is the index of the property Fi which each segment of S needs to satisfy.
    size_t numThreads = 0; // N is the number of threads.
    size_t segLength = 0; // L is the length of each segment of S.
    size_t numSegments = 0; // M is the number of segments in S to generate.
    char c[3]; // c[i], i in {0, 1, 2}, are the letters to be used in the property check.
    char *hostname1; // Hostname of the append server.
    char *hostname2; // Hostname of the verify server.

    ////////////////////////////////////////////////////////////////////////////
    if (argc < 7)
    {
        printInstructions();
        exit(1);
    }
    else
    {
        // The user will see all of their mistakes before the program exits.
        bool errorEncountered = false;

        property = strtol(argv[1], NULL, 10);
        if (property < 0 || property > 3) {
            fprintf(stderr, "Error: i must be in {0,1,2,3}.\n");
            errorEncountered = true;
        }

        numThreads = strtol(argv[2], NULL, 10);
        if (numThreads < 3 || numThreads > 8) {
            fprintf(stderr, "Error: N must be in the range [3, 8].\n");
            errorEncountered = true;
        }

        segLength = strtol(argv[3], NULL, 10);
        if (segLength <= 0) {
            fprintf(stderr, "Error: L must be greater than 0.\n");
            errorEncountered = true;
        }

        numSegments = strtol(argv[4], NULL, 10);
        if (numSegments <= 0) {
            fprintf(stderr, "Error: M must be greater than 0.\n");
            errorEncountered = true;
        }

        c[0] = *argv[5];
        c[1] = *argv[6];
        c[2] = *argv[7];

        for (int i = 0; i < sizeof(c); ++i) {
            if (c[i] < 'a' || c[i] > 'a'+7) { // 'a'+7 is 'h'
                fprintf(stderr, "Error: c_i must be between the letters %c and %c.\n", 'a', 'a'+7);
                errorEncountered = true;
            }
        }

        hostname1 = argv[8];
        hostname2 = argv[9];

        // Check M/N has no remainder.
        if (numSegments % numThreads != 0) {
            fprintf(stderr, "Error: M must be evenly divisible by N.\n");
            errorEncountered = true;
        }

        // Check if segments of length L with alphabet size N can have all strings valid.
        bool validStringPossible = false;
        if (numThreads >= 4) {
            // It's always possible to make a valid string with 4 or more letters.
            validStringPossible = true;
            goto segmentCheckEnd;
        }
        for (int c0 = 0; c0 <= segLength; c0++) {
            for (int c1 = 0; c1 <= segLength; c1++) {
                for (int c2 = 0; c2 <= segLength; c2++) {
                    if (segLength != c0 + c1 + c2) {
                        continue;
                    }

                    switch(property) {
                        case 0:
                            if (((c0 + c1 == c2) && (numThreads >= 3)))
                            {
                                validStringPossible = true;
                                goto segmentCheckEnd;
                            }
                            break;
                        case 1:
                            if (((c0 + 2*c1 == c2) && (numThreads >= 3)))
                            {
                                validStringPossible = true;
                                goto segmentCheckEnd;
                            }
                            break;
                        case 2:
                            if (((c0 * c1 == c2) && (numThreads >= 3)))
                            {
                                validStringPossible = true;
                                goto segmentCheckEnd;
                            }
                            break;
                        case 3:
                            if (((c0 - c1 == c2) && (numThreads >= 3)))
                            {
                                validStringPossible = true;
                                goto segmentCheckEnd;
                            }
                            break;
                        default:
                            printf("Invalid check\n");
                            errorEncountered = true;
                            break;
                    }
                }
            }
        }
        segmentCheckEnd: ; // label to an empty statement

        if (!validStringPossible) {
            errorEncountered = true;
            fprintf(stderr, "Error: A valid string is impossible to create with the given parameters.\n");
        }

        // Exit if any error was encountered.
        if (errorEncountered) {
            printInstructions();
            exit(1);
        }
    }
    ////////////////////////////////////////////////////////////////////////////

    AppendArgs appendArgs;
    appendArgs.property = property;
    appendArgs.segLength = segLength;
    appendArgs.numSegments = numSegments;
    for (int i = 0; i < 3; i++)
        appendArgs.c[i] = c[i];

    VerifyArgs verifyArgs;
    verifyArgs.numThreads = numThreads;
    verifyArgs.segLength = segLength;
    verifyArgs.numSegments = numSegments;

    InitAppendServer(hostname1, appendArgs);
    InitVerifyServer(hostname1, verifyArgs);

    size_t numSegmentsValid = 0;
    #pragma omp parallel for num_threads(numThreads) reduction(+: numSegmentsValid)
    for (int i = 0; i < numThreads; ++i) {
        numSegmentsValid = threadFunc(hostname1, hostname2, i);
    }

    // Print final values
    printf("Final string (formatted): ");
    for (int i = 0; i < numSegments*segLength; i++) {
        if (i%segLength == 0) {
            printf(" ");
        }
        printf("%c", 'x'/*finalString[i]*/);
    }
    printf("\n");
    printf("Final string (unformatted): %s\n", "finalString"/*finalString*/);
    printf("Valid segments: %ld\n", numSegmentsValid);

    // Write to file
    FILE *file = fopen("out.txt", "w");
    if (file == NULL)
    {
        printf("File could not be opened\n");
        exit(1);
    }
    fprintf(file, "%s\n%ld\n", "finalString" /*finalString*/, numSegmentsValid);

    return 0;
}