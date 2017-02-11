/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#include "pa_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void initStr(pa_str* self, size_t numSegments, size_t segmentSize) {
    self->str = calloc(numSegments*segmentSize+1, sizeof(char));
    self->index = 0;
    self->segmentIndex = 0;
    self->length = numSegments*segmentSize;
    self->numSegments = numSegments;
    self->segmentSize = segmentSize;
    self->numSegmentsChecked = 0;
    self->numSegmentsValid = 0;
}

void destroyStr(pa_str* self) {
    free(self->str);
    free(self);
}

/**
 * Returns the pointer to the internal char array.
 */
char* readStr(pa_str* self) {
    return self->str;
}

void writeStr(pa_str* self, char newChar, char enforcementChars[3], size_t enforcementProperty) {
    #pragma omp critical
    {
        // If there is space to write
        if (self->index < self->length) {
            // If the letter can be added with respect to the enforcement rules
            if (canWrite(newChar, &self->str[self->segmentIndex], self->segmentSize, enforcementChars, enforcementProperty)) {
                self->str[self->index] = newChar;
                self->index = self->index + 1;
                self->segmentIndex = (self->index/self->segmentSize)*self->segmentSize;
            }
        }
    }
}

/**
 * Threads will simultaneously poll this function until the string is fully checked.
 */
char* getSegmentToCheck(pa_str* self) {
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

void incrementValidSegments(pa_str* self) {
    #pragma omp atomic
    self->numSegmentsValid++;
}

/**
 * Returns whether the letter can be added with respect to the enforcement property.
 * Used within the critical section of writeStr.
 */
bool canWrite(char letter, char* segment, size_t segLength, char c[3], size_t property) {
    size_t c0Initial = 0;
    size_t c1Initial = 0;
    size_t c2Initial = 0;
    size_t cxInitial = 0;

    if (letter == c[0]) {
        c0Initial++;
    } else if (letter == c[1]) {
        c1Initial++;
    } else if (letter == c[2]) {
        c2Initial++;
    }

    if ((letter != c[0]) && (letter != c[1]) && (letter != c[2])) {
        cxInitial++;
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
            cxInitial++;
        }
    }

    for (int c0 = c0Initial; c0 <= segLength; c0++) {
        for (int c1 = c1Initial; c1 <= segLength; c1++) {
            for (int c2 = c2Initial; c2 <= segLength; c2++) {
                for (int cx = cxInitial; cx <= segLength; cx++) {
                    if (segLength != c0 + c1 + c2 + cx) {
                        continue;
                    }

                    switch(property) {
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
                            printf("Invalid property to check\n");
                            return false;
                    }
                }
            }
        }
    }

    return false;
}
