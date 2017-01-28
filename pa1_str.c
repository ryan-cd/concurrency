#include "pa1_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int initStr(pa1_str* self, size_t numSegments, size_t segmentSize, char* c) {
    self->str = calloc(numSegments*segmentSize+1, sizeof(char));
    self->length = numSegments*segmentSize;
    self->c = c;
    self->index = 0;
    self->numSegments = numSegments;
    self->segmentSize = segmentSize;
    self->numSegmentsChecked = 0;
    self->numSegmentsValid = 0;

    return 0;
}

char* readStr(pa1_str* self) {
    return self->str;
}

int writeStr(pa1_str* self, char newChar) {
    if (self->index >= self->length)
        return -1;
    pthread_mutex_lock(&self->mutex);
    self->str[self->index] = newChar;
    self->index = self->index + 1;
    pthread_mutex_unlock(&self->mutex);

    return 0;
}

void runTask(pa1_str* self, char letter) {
    printf("Thread %c starts and sees: %s\n", letter, read(self));
    for (int i = 0; i < 100; i++) {
        write(self, letter);
    }

    printf("Thread %c finishes and sees: %s\n", letter, read(self));
}

/**
 * Threads will simultaneously poll this function until the string is fully checked.
 */
bool checkProperty(pa1_str* self, size_t property) {

    if (self->numSegmentsChecked == self->numSegments) {
        return false;
    }

    pthread_mutex_lock(&self->mutex);
    size_t startIndex = self->numSegmentsChecked * self->segmentSize;

    // Count the occurences of each letter in c.
    int occurances[3] = {0,0,0};
    for (size_t i = startIndex; i < startIndex+self->segmentSize; i++) {
        for (int j = 0; j < 3; j++) {
            if (self->str[i] == self->c[j]) {
                occurances[j]++;
            }
        }
    }
    printf("occurances[c] = {%d, %d, %d}\n", occurances[0], occurances[1], occurances[2]); // debug

    bool isValid = false;
    switch (property) {
        case 0: 
            printf("Checking property 0\n");
            /*
            bool hasA = false;
            bool hasB = false;
            bool hasC = false;
            printf("Checking from %d to %d\n", startIndex, startIndex+self->segmentSize);
            for (int i = startIndex; i < startIndex+self->segmentSize; i++) {
                if(self->str[i] == 'a')
                    hasA = true;
                if(self->str[i] == 'b')
                    hasB = true;
                if(self->str[i] == 'c')
                    hasC = true;
            }
            if(hasA && hasB && hasC) {
                returnValue = true;
                self->numSegmentsValid++;
            }
            */
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

    if (isValid) {
        self->numSegmentsValid++;
    }
    self->numSegmentsChecked++;
    pthread_mutex_unlock(&self->mutex);
    return true;
}

size_t readIndex(struct _pa1_str* self) {
    return 0;
}

size_t readSegmentsChecked(struct _pa1_str* self) {
    return 0;
}
size_t readSegmentsValid(struct _pa1_str* self) {
    return 0;
}
