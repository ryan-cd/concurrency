#ifndef __PA1_STR__
#define __PA1_STR__

#include <stddef.h>

typedef struct _pa1_str {
    char* str;
    size_t index;
    size_t length;
    int (*init)(struct _pa1_str* self, size_t length);
    char* (*read)(struct _pa1_str* self);
    int (*write)(struct _pa1_str* self, char newChar);
} pa1_str;

void stringTest();



#endif