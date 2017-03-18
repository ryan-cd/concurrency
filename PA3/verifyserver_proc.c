#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "verifyserver.h"

#define PORT 1337
#define BUFLEN 2076

/* Citation: the code to initialize the UDP connection
was provided in receiver.cpp and main.c from the course
site */
void setupUDP() {
	struct sockaddr_in si_me, si_other;
	int socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char rcvBuf[BUFLEN];
    char sndBuf[BUFLEN];
    char strIP[BUFLEN];
    char myStr[BUFLEN];
    int slen = sizeof(si_other);
	if (socketID == -1) {
		printf("Socket could not be initialized. Exiting.\n");
		exit(0);
	} else {
		printf("Socket created.\n");
	}

	memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int bindVal = bind(socketID, (struct sockaddr*)&si_me, sizeof(si_me));
    if (bindVal == -1){
        exit(0);
    }
    else
        printf("Socket bound to: %d\n", PORT);

	int recVal;
	while (0)
    {
        printf("Waiting to receive a message...\n");
        recVal = recvfrom(socketID, rcvBuf, BUFLEN, 0, (struct sockaddr *)&si_other, (socklen_t*)&slen);
        if (recVal == -1){
            exit(0);
        }
        
        printf("Received packet from %s:%d\nMessage: %s\n\n",
               inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), rcvBuf);
	}
    close(socketID);
}

int *rpc_initverifyserver_1_svc(VerifyArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitVerifyServer\n");
	setupUDP();

	return &result;
}

int *rpc_getseg_1_svc(int *thread, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_GetSeg\n");

	return &result;
}

int *rpc_getstring_1_svc(int *thread, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_GetString\n");

	return &result;
}