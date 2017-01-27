#ifndef __PA1_STR__
#define __PA1_STR__

#include <stddef.h>
#include <pthread.h>

typedef struct _pa1_str {
    char* str;
    size_t index;
    size_t length;
    pthread_mutex_t mutex;
    int (*initStr)(struct _pa1_str* self, size_t length);
    char* (*readStr)(struct _pa1_str* self);
    int (*writeStr)(struct _pa1_str* self, char newChar);
    void (*runTask)(struct _pa1_str* self, char letter);
} pa1_str;



#endif