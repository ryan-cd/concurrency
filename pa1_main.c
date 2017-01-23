#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

struct threadParams {
    int id;
    char letter;
};

void *threadFunc(void *p)
{
    struct threadParams *params = (struct threadParams *)p;
    printf("Hello. Thread: %d\n", params->id);
}

int main(int argc, char **argv)
{
    int index; // i is the index of the property Fi which each segment of S needs to satisfy.
    int numThreads; // N is the number of threads.
    int segLength; // L is the length of each segment of S.
    int numSegments; // M is the number of segments in S to generate.
    char c[3]; // c[i], i in {0, 1, 2}, are the letters to be used in the property check.

    char *sharedString;

    if (argc < 7)
    {
        printf("Missing arguments.\n");
        //exit(1);
    } else
    {
        index = atoi(argv[1]);
        numThreads = atoi(argv[2]);
        segLength = atoi(argv[3]);
        numSegments = atoi(argv[4]);
        c[0] = *argv[5];
        c[1] = *argv[6];
        c[2] = *argv[7];
    }

    //?? Verify inputs?
    //?? Check if numThreads between 3 and 8?

    // Threads
    pthread_t threads[numThreads]; //?? Put on stack or heap?
    struct threadParams params[3];
    
    for (int i = 0; i < 3; ++i) { 
        params[i].id = i;
        params[i].letter = 'a';
        pthread_create(&threads[i], NULL, (void *)threadFunc, (void *)&params[i]);
    }

    for (int i = 0; i < 3; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}