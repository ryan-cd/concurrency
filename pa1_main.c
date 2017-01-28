#include <stdio.h> // printf
#include <stdlib.h> // malloc, atoi, size_t
//#include <string.h>
#include <stdbool.h> // bool
#include <unistd.h> // usleep

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h> // bool

#include "pa1_str.h"

struct threadParams {
    int id;
    char letter;
    pa1_str *str;
    size_t segLength; // L
    size_t numSegments; // M
    size_t property; // i
};

bool checkProperty(char *segment, size_t length, char *c, size_t property) {
    // Count the occurences of each letter in c.
    int occurances[3] = {0,0,0};
    for (size_t i = 0; i < length; i++) {
        for (int j = 0; j < 3; j++) {
            if (segment[i] == c[j]) {
                occurances[j]++;
            }
        }
    }
    printf("occurances[c] = {%d, %d, %d}\n", occurances[0], occurances[1], occurances[2]); // debug

    bool isValid = false;
    switch (property) {
        case 0:
            printf("Checking property 0\n");
            if (occurances[0] + occurances[1] == occurances[2]) {
                isValid = true;
            }
            break;
        case 1:
            printf("Checking property 1\n");
            if (occurances[0] + 2 * occurances[1] == occurances[2]) {
                isValid = true;
            }
            break;
        case 2:
            printf("Checking property 2\n");
            if (occurances[0] * occurances[1] == occurances[2]) {
                isValid = true;
            }
            break;
        case 3:
            printf("Checking property 3\n");
            if (occurances[0] - occurances[1] == occurances[2]) {
                isValid = true;
            }
            break;
        default:
            printf("Error: Invalid property");
            break;
    }
    return isValid;
}

void *threadFunc(void *p)
{
    struct threadParams *params = (struct threadParams *)p;

    printf("Hello. Thread: %d\n", params->id);

    while (params->str->index+2 < params->numSegments*params->segLength) { // |S| < M * L
        // Sleep for a random period between 100ms and 500ms.
        unsigned int microseconds = (rand() % (500000 + 1 - 100000)) + 100000; // Biased due to modulus.
        printf("Sleeping for %d usecs\n", microseconds);
        usleep(microseconds);
        // Attempt to acquire resource S and write a letter.
        writeStr(params->str, params->letter);
    }

    // while(params->str->numSegmentsChecked < params->str->numSegments) {
    //     checkProperty(params->str, params->property);
    // }
    /* Alternatively way:
     * Where checkProperty returns 0 if there are no segments left to check.
     */
    char *segment = NULL;
    while((segment = getSegmentToCheck(params->str)) != NULL ) {
        if (checkProperty(segment, params->segLength, params->str->c, params->property)) {
            params->str->incrementValidSegments(params->str);
        }
    }


    return NULL;
}

int main(int argc, char **argv)
{
    size_t property; // i is the index of the property Fi which each segment of S needs to satisfy.
    size_t numThreads; // N is the number of threads.
    size_t segLength; // L is the length of each segment of S.
    size_t numSegments; // M is the number of segments in S to generate.
    char c[3]; // c[i], i in {0, 1, 2}, are the letters to be used in the property check.
    
    if (argc < 7)
    {
        printf(
        "Missing arguments.\n\n"
        "Usage: %s i N L M c_0 c_1 c_2\n\n"
        "Parameters:\n"
        "\t i:   the index of the property Fi which each segment of S needs to satisfy.\n"
        "\t N:   the number of threads.\n"
        "\t L:   the length of each segment of S.\n"
        "\t M:   the number of segments in S to generate.\n"
        "\t c_i: the letters to be used in the property check.\n\n"
        "Example: ./pa1.x 0 3 6 3 b c a\n"
        , argv[0]);
        exit(1);
    }
    else
    {
        // The user will see all of their mistakes before the program exits.
        bool errorEncountered = false;
        // Used to error check first before casting to unsigned int.
        int tempInt;

        tempInt = atoi(argv[1]);
        if (tempInt < 0 || tempInt > 2) {
            printf("Error: i must be in {0,1,2}.\n");
            errorEncountered = true;
        } else {
            property = tempInt; // cast to size_t
        }

        tempInt = atoi(argv[2]);
        if (tempInt < 3 || tempInt > 8) {
            printf("Error: N must be in the range [3, 8].\n");
            errorEncountered = true;
        } else {
            numThreads = tempInt;
        }

        tempInt = atoi(argv[3]);
        if (tempInt < 0) {
            printf("Error: L must be greater than 0.\n");
            errorEncountered = true;
        } else {
            segLength = tempInt;
        }

        tempInt = atoi(argv[4]);
        if (tempInt < 0) {
            printf("Error: M must be greater than 0.\n");
            errorEncountered = true;
        } else {
            numSegments = tempInt;
        }

        c[0] = *argv[5];
        c[1] = *argv[6];
        c[2] = *argv[7];
        for (int i = 0; i < 3; ++i) {
            if (c[i] < 'a' || c[i] > 'a'+7) { // 'a'+7 is 'h'
                printf("Error: c_i must be between the letters %c and %c.\n", 'a', 'a'+7);
                errorEncountered = true;
            }
        }

        // Exit if any error was encountered.
        if (errorEncountered) {
            printf("Exiting.\n");
            exit(1);
        }
    }

    //?? Verify inputs?
    //?? Check if numThreads between 3 and 8?

    // Initialize string
    pa1_str* str = malloc(sizeof(pa1_str));
    initStr(str, numSegments, segLength, c);

    // Threads
    pthread_t threads[numThreads];
    struct threadParams params[numThreads];

    for (int i = 0; i < numThreads; ++i) {
        params[i].id = i;
        params[i].letter = 'a'+i;
        params[i].str = str;
        params[i].segLength = segLength;
        params[i].numSegments = numSegments;
        params[i].property = property;
        pthread_create(&threads[i], NULL, threadFunc, &params[i]);
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Finish and clean up
    printf("Final string: %s\n", readStr(str));
    printf("Valid segments: %d\n", str->numSegmentsValid);
    free(str->str);
    free(str);

    return 0;
}