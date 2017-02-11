/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#include "pa2_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int initStr(pa1_str* self, size_t numSegments, size_t segmentSize) {
    self->str = calloc(numSegments*segmentSize+1, sizeof(char));
    self->index = 0;
    self->segmentIndex = 0;
    self->length = numSegments*segmentSize;
    self->numSegments = numSegments;
    self->segmentSize = segmentSize;
    self->numSegmentsChecked = 0;
    self->numSegmentsValid = 0;

    return 0;
}

int destroyStr(pa1_str* self) {
    free(self->str);
    free(self);
    return 0;
}

char* readStr(pa1_str* self) {
    return self->str;
}

int writeStr(pa1_str* self, char newChar) {
    if (self->index >= self->length)
        return -1;
    #pragma omp critical
    {
        self->str[self->index] = newChar;
        self->index = self->index + 1;
        self->segmentIndex = (self->index/self->segmentSize)*self->segmentSize;
    }

    return 0;
}

/**
 * Threads will simultaneously poll this function until the string is fully checked.
 */
char* getSegmentToCheck(pa1_str* self) {
    char *segment;

    #pragma omp critical
    {
        if (self->numSegmentsChecked == self->numSegments) {
            segment = NULL;
        } else {
            segment = self->str + (self->numSegmentsChecked * self->segmentSize);
            self->numSegmentsChecked++;
        }
    }
    return segment;
}

void incrementValidSegments(pa1_str* self) {
    #pragma omp critical
    {
        self->numSegmentsValid++;
    }
}
