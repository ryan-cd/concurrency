#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "verifyserver.h"
#include <unistd.h>

#define PORT 1337
#define BUFLEN 2076

VerifyArgs verifyArgs;
const int llbufsize = 1024; // Size of LLString buffer chunks
int segmentsChecked;

struct sockaddr_in si_me, si_other;
int socketID;
char rcvBuf[BUFLEN];
char sndBuf[BUFLEN];
char strIP[BUFLEN];
char myStr[BUFLEN];
int slen;
int stringReceived;

/* Receive the UDP packet of the final string */
void *receive(void *input) {
    printf("Waiting to receive string...\n");
    int recVal;
    recVal = recvfrom(socketID, rcvBuf, BUFLEN, 0, (struct sockaddr *)&si_other, (socklen_t*)&slen);
    if (recVal == -1){
        printf("The value of receive was -1. Exiting.\n");
        exit(0);
    }

    stringReceived = 1;
    printf("String received: %s\n", rcvBuf);

    close(socketID);
    return NULL;
}

/* Citation: the code to initialize the UDP connection
was provided in receiver.cpp and main.c from the course
site */
void setupUDP()
{
    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // Make the socket reusable
    if (setsockopt(socketID, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 },
        sizeof(int)) < 0)
    {
        printf("Unable to set the reuse option for the socket\n");
    }
    slen = sizeof(si_other);
    if (socketID == -1)
    {
        printf("Socket could not be initialized. Exiting.\n");
        exit(0);
    }
    else
    {
        printf("Socket created.\n");
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    int bindVal = bind(socketID, (struct sockaddr*)&si_me, sizeof(si_me));
    if (bindVal == -1)
    {
        printf("Port could not be bound\n");
        exit(0);
    }
    else
        printf("Socket bound to: %d\n", PORT);

    printf("Trying to make thread.\n");
    pthread_t udpThread;
    if(pthread_create(&udpThread, NULL, &receive, NULL)) {
        printf("Error creating thread\n");
    } else {
        printf("UDP Thread created.\n");
    }
}

/* Parse char * to LLString */
LLString *createLLString(char *source, int length) {
    LLString *llstring;

    int leftoverLength = length; // Bytes left to write
    int parsed = 0; // Bytes written so far

    LLString *head = NULL;
    while (leftoverLength > 0) {
        LLString *nextPart = malloc(sizeof(LLString));
        nextPart->bytesLeft = leftoverLength;
        int used = (leftoverLength < llbufsize) ? leftoverLength : llbufsize;
        memcpy(nextPart->buffer, &source[parsed], used);
        parsed += used;
        leftoverLength -= used;

        if (head == NULL) {
            llstring = nextPart;
        } else {
            head->next = nextPart;
        }
        head = nextPart;
    }

    return llstring;
}

void freeLLString(LLString *llstring)
{
    LLString *temp;
    while (llstring->next != NULL) {
        temp = llstring;
        llstring = llstring->next;
        free(temp);
    }
}

int *rpc_initverifyserver_1_svc(VerifyArgs *args, struct svc_req *req)
{
    static int result = 1;
    printf("RPC_InitVerifyServer\n");
    verifyArgs = *args;
    stringReceived = 0;
    segmentsChecked = 0;

    setupUDP();

    return &result;
}

LLString *rpc_getseg_1_svc(int *thread, struct svc_req *req)
{
    static LLString result; // For passing messages through result.bytesLeft
    static LLString **realResult = NULL; // A result buffer for each thread

    result.bytesLeft = 0;
    printf("RPC_GetSeg: Thread #%d\n", *thread);

    if (stringReceived != 1) {
        printf("Requested string too soon. Resend the request.\n");
        return &result; // Client should read that bytesLeft is 0 and re-request.
    }

    if (realResult == NULL) {
        realResult = calloc(verifyArgs.numThreads, sizeof(LLString));
    }

    if (segmentsChecked < verifyArgs.numSegments) {
        if (realResult[*thread] != 0) {
            freeLLString(realResult[*thread]); // Free memory of previous segment given the same thread
        }
        realResult[*thread] = createLLString(&rcvBuf[segmentsChecked*verifyArgs.segLength], verifyArgs.segLength);
        segmentsChecked += 1;
        return realResult[*thread];
    } else {
        result.bytesLeft = -1; // Client should read -1 and stop requesting segments
        printf("No more segments.\n");
        return &result;
    }
}

LLString *rpc_getstring_1_svc(int *thread, struct svc_req *req)
{
    static LLString result; // For passing messages through result.bytesLeft
    static LLString *realResult;

    result.bytesLeft = 0;
    printf("RPC_GetString\n");

    if (stringReceived != 1) {
        printf("Requested string too soon. Resend the request.\n");
        return &result; // Client should read that bytesLeft is 0 and re-request.
    }

    int stringLen = verifyArgs.segLength * verifyArgs.numSegments;

    if (realResult != NULL) {
        freeLLString(realResult);  // Free memory if a string already exists
    }
    realResult = createLLString(rcvBuf, stringLen);

    return realResult;
}