#include "pa1_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int initStr(pa1_str* self, size_t length) {
    (self)->str = calloc(length+1, sizeof(char));
    (self)->length = length;
    (self)->index = 0;
    
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