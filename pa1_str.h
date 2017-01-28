#ifndef __PA1_STR__
#define __PA1_STR__

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h> // bool

typedef struct _pa1_str {
    char* str;
    size_t index;
    size_t length;
    char* c;
    pthread_mutex_t mutex;
    pthread_mutex_t checkMutex;
    size_t segmentSize;
    size_t numSegments;
    size_t numSegmentsChecked;
    size_t numSegmentsValid;
    int (*initStr)(struct _pa1_str* self, size_t numSegments, size_t segmentSize, char c);
    char* (*readStr)(struct _pa1_str* self);
    int (*writeStr)(struct _pa1_str* self, char newChar);
    void (*runTask)(struct _pa1_str* self, char letter);
    char* (*getSegmentToCheck)(struct _pa1_str* self);
    void (*incrementValidSegments)(struct _pa1_str* self);
    size_t (*readIndex)(struct _pa1_str* self);
    size_t (*readSegmentsChecked)(struct _pa1_str* self);
    size_t (*readSegmentsValid)(struct _pa1_str* self);
} pa1_str;



#endif