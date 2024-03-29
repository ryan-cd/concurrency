/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#include <stdio.h> // printf
#include <stdlib.h> // malloc, atoi, size_t
#include <stdbool.h> // bool
#include <unistd.h> // usleep
#include <omp.h> // openmp
#include "pa_str.h" // pa_str

struct threadParams {
    int id;
    char letter;
    char* c;
    pa_str *str;
    size_t segLength; // L
    size_t numSegments; // M
    size_t property; // i
};

void printInstructions() {
    fprintf(stderr,
        "\nUsage: ./pa2.x i N L M c_0 c_1 c_2\n"
        "Parameters:\n"
        "\t i:   (0<=i<=3) The index of the property Fi which each segment of S needs to satisfy.\n"
        "\t N:   (3<=N<=8) The number of threads.\n"
        "\t L:   (0 < L)   The length of each segment of S.\n"
        "\t M:   (0 < M)   The number of segments in S to generate.\n"
        "\t c_i: (0<=i<=2) The letters to be used in the property check.\n"
        "Example: ./pa2.x 0 3 6 3 b c a\n");
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
    bool isValid = false;
    switch (property) {
        case 0:
            if (occurences[0] + occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 1:
            if (occurences[0] + 2 * occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 2:
            if (occurences[0] * occurences[1] == occurences[2]) {
                isValid = true;
            }
            break;
        case 3:
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

    // Loop until the string has been completely built.
    while (params->str->index < params->numSegments*params->segLength) { // |S| < M * L
        // Sleep for a random period between 100ms and 500ms.
        unsigned int microseconds = (rand() % (500000 + 1 - 100000)) + 100000; // Biased due to modulus.
        usleep(microseconds);
        // writeStr will do the enforcement.
        writeStr(params->str, params->letter, params->c, params->property);
    }

    // Do the check after the string has been completely built.
    char *segment = NULL;
    while((segment = getSegmentToCheck(params->str)) != NULL) {
        if (checkProperty(segment, params->segLength, params->c, params->property)) {
            incrementValidSegments(params->str);
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    size_t property = 0; // i is the index of the property Fi which each segment of S needs to satisfy.
    size_t numThreads = 0; // N is the number of threads.
    size_t segLength = 0; // L is the length of each segment of S.
    size_t numSegments = 0; // M is the number of segments in S to generate.
    char c[3]; // c[i], i in {0, 1, 2}, are the letters to be used in the property check.

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

        #pragma omp parallel for num_threads(sizeof(c))
        for (int i = 0; i < sizeof(c); ++i) {
            if (c[i] < 'a' || c[i] > 'a'+7) { // 'a'+7 is 'h'
                fprintf(stderr, "Error: c_i must be between the letters %c and %c.\n", 'a', 'a'+7);
                errorEncountered = true;
            }
        }

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

    // Initialize string
    pa_str* str = malloc(sizeof(pa_str));
    initStr(str, numSegments, segLength);

    // Threads
    struct threadParams params[numThreads];

    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < numThreads; ++i) {
        params[i].id = i;
        params[i].letter = 'a'+i;
        params[i].c = c;
        params[i].str = str;
        params[i].segLength = segLength;
        params[i].numSegments = numSegments;
        params[i].property = property;
        threadFunc(&params[i]);
    }

    // Print final values
    printf("Final string (formatted): ");
    for (int i = 0; i < str->length; i++) {
        if (i%segLength == 0) {
            printf(" ");
        }
        printf("%c", str->str[i]);
    }
    printf("\n");
    printf("Final string (unformatted): %s\n", readStr(str));
    printf("Valid segments: %ld\n", str->numSegmentsValid);

    // Write to file
    FILE *file = fopen("out.txt", "w");
    if (file == NULL)
    {
        printf("File could not be opened\n");
        exit(1);
    }
    fprintf(file, "%s\n%ld\n", readStr(str), str->numSegmentsValid);

    // Clean up
    fclose(file);
    destroyStr(str);

    return 0;
}
