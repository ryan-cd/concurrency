#include "pa1_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    if (self->index >= self->length-1)
        return -1;
    
    self->str[self->index] = newChar;
    self->index = self->index + 1;
    
    return 0;
}

void stringTest() {
    pa1_str* str = malloc(sizeof(pa1_str));
    init(str, 100);
    printf("Reading empty: %s\n\n", read(str));
    for (int i = 0; i < 98; i++) {
        write(str, 'a');
    }
    write(str, 'b');
    write(str, 'c');
    printf("Reading: %s\n\n", read(str));
}