/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#include "pa1_str.h"
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
    pthread_mutex_init(&self->mutex, NULL);
    pthread_mutex_init(&self->checkMutex, NULL);
    return 0;
}

int destroyStr(pa1_str* self) {
    free(self->str);
    pthread_mutex_destroy(&self->mutex);
    pthread_mutex_destroy(&self->checkMutex);
    free(self);
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
    self->segmentIndex = (self->index/self->segmentSize)*self->segmentSize;
    pthread_mutex_unlock(&self->mutex);

    return 0;
}

/**
 * Threads will simultaneously poll this function until the string is fully checked.
 */
char* getSegmentToCheck(pa1_str* self) {
    char *segment;

    pthread_mutex_lock(&self->mutex);
    if (self->numSegmentsChecked == self->numSegments) {
        segment = NULL;
    } else {
        segment = self->str + (self->numSegmentsChecked * self->segmentSize);
        self->numSegmentsChecked++;
    }
    pthread_mutex_unlock(&self->mutex);

    return segment;
}

void incrementValidSegments(pa1_str* self) {
    pthread_mutex_lock(&self->checkMutex);
    self->numSegmentsValid++;
    pthread_mutex_unlock(&self->checkMutex);
}
