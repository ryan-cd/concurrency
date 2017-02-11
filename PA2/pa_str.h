/**
 *  Ryan Davis - davisr3
 *  Don Pham - phamd
 */
#ifndef __PA_STR__
#define __PA_STR__

#include <stddef.h>
#include <stdbool.h>

typedef struct _pa_str {
    char* str;
    size_t index;
    size_t segmentIndex; // index of the beginning of the current segment
    size_t length;
    size_t segmentSize;
    size_t numSegments;
    size_t numSegmentsChecked;
    size_t numSegmentsValid;
} pa_str;

// Constructor & Destructor
void initStr(pa_str* self, size_t numSegments, size_t segmentSize);
void destroyStr(pa_str* self);

// Read & Write
char* readStr(pa_str* self);
void writeStr(pa_str* self, char newChar, char enforcementChars[3], size_t enforcementProperty);

// Checking
char* getSegmentToCheck(pa_str* self);
void incrementValidSegments(pa_str* self);

// Enforcement
bool canWrite(char letter, char* segment, size_t segLength, char c[3], size_t property);

#endif
