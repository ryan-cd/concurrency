#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "verifyserver.h"
#include <unistd.h>

#define PORT 1337
#define BUFLEN 2076

struct sockaddr_in si_me, si_other;
int socketID;
char rcvBuf[BUFLEN];
char sndBuf[BUFLEN];
char strIP[BUFLEN];
char myStr[BUFLEN];
int slen;
int stringReceived = 0;

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
    //printf("Received packet from %s:%d\nMessage: %s\n\n",
            //inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), rcvBuf);

    close(socketID);
	return NULL;
}

/* Citation: the code to initialize the UDP connection
was provided in receiver.cpp and main.c from the course
site */
void setupUDP()
{
	socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
        exit(0);
    }
    else
        printf("Socket bound to: %d\n", PORT);

	printf("Trying to make thread.\n");
	pthread_t udpThread;
	if(pthread_create(&udpThread, NULL, &receive, NULL)) {
		printf("Error creating thread");
	} else {
		printf("UDP Thread created.\n");
	}
}


VerifyArgs verifyArgs;

int *rpc_initverifyserver_1_svc(VerifyArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitVerifyServer\n");
	setupUDP();

	// Copy into global
	verifyArgs = *args;

	return &result;
}

LLString *rpc_getseg_1_svc(int *thread, struct svc_req *req)
{
	static LLString result;
	printf("RPC_GetSeg\n");

	return &result;
}

LLString *rpc_getstring_1_svc(int *thread, struct svc_req *req)
{
	static LLString llstring;
	llstring.bytesLeft = 0;
	printf("RPC_GetString\n");

	if (stringReceived != 1) {
		printf("Requested string too soon. Resend the request.\n");
		return &llstring; // Client should read that bytesLeft is 0 and re-request.
	}

	/* Parse char * to LLString */
	int totalLength = verifyArgs.segLength*verifyArgs.numSegments;
	int leftoverLength = totalLength; // Bytes left to write
	int parsed = 0; // Bytes written so far
	int llbufsize = 1024; // Size of LLString buffer chunks

	// Stuff up to llbufsize bytes into the LLString
	llstring.bytesLeft = leftoverLength;
	int used = (leftoverLength < llbufsize) ? leftoverLength : llbufsize;
	memcpy(llstring.buffer, &rcvBuf[parsed], used);
	parsed += used;
	leftoverLength -= used;

	// While the char * is larger than llbufsize, create linked lists
	LLString *head = &llstring;
	while (leftoverLength > 0) {
		LLString *nextPart = malloc(sizeof(LLString));
		nextPart->bytesLeft = leftoverLength;
		int used = (leftoverLength < llbufsize) ? leftoverLength : llbufsize;
		memcpy(nextPart->buffer, &rcvBuf[parsed], used);
		parsed += used;
		leftoverLength -= used;

		head->next = nextPart;
		head = nextPart;
	}

	return &llstring;
}