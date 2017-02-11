/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#ifndef __PA1_STR__
#define __PA1_STR__

#include <stddef.h>
#include <stdbool.h>

typedef struct _pa1_str {
    char* str;
    size_t index;
    size_t segmentIndex; // index of the beginning of the current segment
    size_t length;
    size_t segmentSize;
    size_t numSegments;
    size_t numSegmentsChecked;
    size_t numSegmentsValid;
} pa1_str;

int initStr(struct _pa1_str* self, size_t numSegments, size_t segmentSize);
int destroyStr(struct _pa1_str* self);
char* readStr(struct _pa1_str* self);
int writeStr(struct _pa1_str* self, char newChar, char enforcementChars[3], size_t enforcementProperty);
char* getSegmentToCheck(struct _pa1_str* self);
void incrementValidSegments(struct _pa1_str* self);
bool canWrite(char letter, char* segment, size_t segLength, char c[3], size_t property);

#endif
