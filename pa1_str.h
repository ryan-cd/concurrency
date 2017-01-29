#ifndef __PA1_STR__
#define __PA1_STR__

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h> // bool

typedef struct _pa1_str {
    char* str;
    size_t index;
    size_t segmentIndex; // index of the beginning of the current segment
    size_t length;
    pthread_mutex_t mutex;
    pthread_mutex_t checkMutex;
    size_t segmentSize;
    size_t numSegments;
    size_t numSegmentsChecked;
    size_t numSegmentsValid;
    int (*initStr)(struct _pa1_str* self, size_t numSegments, size_t segmentSize, char c);
    int (*destroyStr)(struct _pa1_str* self);
    char* (*readStr)(struct _pa1_str* self);
    int (*writeStr)(struct _pa1_str* self, char newChar);
    char* (*getSegmentToCheck)(struct _pa1_str* self);
    void (*incrementValidSegments)(struct _pa1_str* self);
} pa1_str;



#endif