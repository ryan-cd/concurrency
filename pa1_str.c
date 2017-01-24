#include "pa1_str.h"
#include <stdlib.h>
#include <string.h>

int init(pa1_str* self, size_t length) {
    (self)->str = calloc(++length, sizeof(char));
    (self)->length = length;
    (self)->index = 0;
    
    return 0;
}

char* read(pa1_str* self) {
    return self->str;
}

int write(pa1_str* self, char newChar) {
    self->str[self->index] = newChar;
    self->index = self->index + 1;
    return 0;
}