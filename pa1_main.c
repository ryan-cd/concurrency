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
    char* c;
    pa1_str *str;
    size_t segLength; // L
    size_t numSegments; // M
    size_t property; // i
};

bool canWrite(char letter, char* segment, size_t segLength, char c[3], size_t property) {
    bool validStringPossible = false;
    size_t c0Initial = 0;
    size_t c1Initial = 0;
    size_t c2Initial = 0;
    size_t cx = 0;

    if (letter == c[0]) {
        c0Initial++;
    } else if (letter == c[1]) {
        c1Initial++;
    } else if (letter == c[2]) {
        c2Initial++;
    }

    if ((letter != c[0]) && (letter != c[1]) && (letter != c[2])) {
        cx++;
    }

    for (int i = 0; i < segLength; i++) {
        if (c[0] == segment[i]) {
            c0Initial++;
        }
        else if (c[1] == segment[i]) {
            c1Initial++;
        }
        else if (c[2] == segment[i]) {
            c2Initial++;
        }
        // No more characters in this segment
        else if (segment[i] == 0) {
            break;
        //There is a letter not in c[3]
        } else {
            cx++;
        }
    }

    for (int c0 = c0Initial; c0 < segLength - cx; c0++) {
        for (int c1 = c1Initial; c1 < segLength - cx; c1++) {
            for (int c2 = c2Initial; c2 < segLength - cx; c2++) {
                if (segLength - cx != c0 + c1 + c2) {
                    continue;
                }

                switch(property){
                    case 0:
                        if (c0 + c1 == c2)
                        {
                            return true;
                        }
                        break;
                    case 1:
                        if (c0 + 2*c1 == c2)
                        {
                            return true;
                        }
                        break;
                    case 2:
                        if (c0 * c1 == c2)
                        {
                            return true;
                        }
                        break;
                    case 3:
                        if (c0 - c1 == c2)
                        {
                            return true;
                        }
                        break;
                    default:
                        printf("Invalid check\n");
                        return false;
                }
            }
        }
    }

    return false;
}

bool checkProperty(char *segment, size_t length, char *c, size_t property) {
    // Count the occurences of each letter in c.
    int occurences[3] = {0,0,0};
    for (size_t i = 0; i < length; i++) {
        for (int j = 0; j < 3; j++) {
            if (segment[i] == c[j]) {
                occurences[j]++;
            }
        }
    }
    printf("occurences[c] = {%d, %d, %d}\n", occurences[0], occurences[1], occurences[2]); // debug

    bool isValid = false;
    switch (property) {
        case 0:
            printf("Checking property 0\n");
            if (occurences[0] + occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 1:
            printf("Checking property 1\n");
            if (occurences[0] + 2 * occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 2:
            printf("Checking property 2\n");
            if (occurences[0] * occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 3:
            printf("Checking property 3\n");
            if (occurences[0] - occurences[1] == occurences[2]) {
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

    while (params->str->index < params->numSegments*params->segLength) { // |S| < M * L
        // Sleep for a random period between 100ms and 500ms.
        unsigned int microseconds = (rand() % (500000 + 1 - 100000)) + 100000; // Biased due to modulus.
        //printf("Sleeping for %d usecs\n", microseconds);
        usleep(microseconds);
        // Attempt to acquire resource S and write a letter.
        if (canWrite(params->letter, &params->str->str[params->str->segmentIndex], params->segLength, params->c, params->property)) {
            writeStr(params->str, params->letter);
        }
    }

    // while(params->str->numSegmentsChecked < params->str->numSegments) {
    //     checkProperty(params->str, params->property);
    // }
    /* Alternatively way:
     * Where checkProperty returns 0 if there are no segments left to check.
     */
    char *segment = NULL;
    while((segment = getSegmentToCheck(params->str)) != NULL) {
        if (checkProperty(segment, params->segLength, params->str->c, params->property)) {
            incrementValidSegments(params->str);
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
        "\t i:   (0<=i<=3) The index of the property Fi which each segment of S needs to satisfy.\n"
        "\t N:   (3<=N<=8) The number of threads.\n"
        "\t L:   (0 < L)   The length of each segment of S.\n"
        "\t M:   (0 < M)   The number of segments in S to generate.\n"
        "\t c_i: (0<=i<=2) The letters to be used in the property check.\n\n"
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
        if (tempInt < 0 || tempInt > 3) {
            printf("Error: i must be in {0,1,2,3}.\n");
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

        /*
        Segment Check
        Check that segments of length L with alphabet size N can have all strings valid
        */
        if (segLength < 1) {
            errorEncountered = true;
        }

        bool validStringPossible = false;
        if (numThreads >= 4) {
            validStringPossible = true;
            goto segmentCheckEnd;
        }
        for (int c0 = 0; c0 < segLength; c0++) {
            for (int c1 = 0; c1 < segLength; c1++) {
                for (int c2 = 0; c2 < segLength; c2++) {
                    if (segLength != c0 + c1 + c2) {
                        continue;
                    }

                    switch(property){
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
        segmentCheckEnd:

        if (!validStringPossible) {
            errorEncountered = true;
            printf("*A valid string is NOT possible*\n");
        } else {
            printf("*A valid string IS possible*\n");
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
        params[i].c = c;
        params[i].str = str;
        params[i].segLength = segLength;
        params[i].numSegments = numSegments;
        params[i].property = property;
        pthread_create(&threads[i], NULL, threadFunc, &params[i]);
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Print final values
    printf("Final string: %s\n", readStr(str));
    printf("Valid segments: %d\n", str->numSegmentsValid);

    //Write to file
    FILE *file = fopen("out.txt", "w");
    if (file == NULL)
    {
        printf("File could not be opened\n");
        exit(1);
    }
    fprintf(file, "%s\n%d", readStr(str), str->numSegmentsValid);

    //Clean up
    fclose(file);
    destroyStr(str);

    return 0;
}