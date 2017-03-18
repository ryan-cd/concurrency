#include <stdio.h>
#include <pthread.h>
#include "appendserver.h"

#define OTHERPORT 1337
#define BUFLEN 2076

/* String */
typedef struct {
    char* str;
    unsigned int length;
    unsigned int index;
} Str;

/* Globals */
Str string;

/* RPC Procedure: int RPC_InitAppendServer(AppendArgs) */
int *rpc_initappendserver_1_svc(AppendArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitAppendServer\n");

	// Init string
	string.length = args->numSegments * args->segLength;
	if (string.str == NULL) {
		string.str = calloc(string.length + 1, sizeof(char));
	} else {
		memset(string.str, 0, string.length + 1);
	}
	string.index = 0;

	return &result;
}

/* RPC Procedure: int RPC_Append(char) */
int *rpc_append_1_svc(char *letter, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_Append\n");
	printf("string [%d/%d]: %s \n", string.index, string.length, string.str);

	// Append letter if there is space
	if (string.index < string.length) {
		string.str[string.index++] = *letter;
		result = 0;
	} else {
		result = -1;
	}

	struct sockaddr_in si_me, si_other;

	int s;
	char rcvBuf[BUFLEN];
	char sndBuf[BUFLEN];
	char strIP[BUFLEN];
	char myStr[BUFLEN];
	int slen = sizeof(si_other);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1){
		printf("Socket failed to construct.");
		exit(0);
	}
	else
		printf("UDP Socket created successfully.\n");

	struct sockaddr_in sis;
	int sislen=sizeof(sis);
	memset((char *) &sis, 0, sizeof(sis));
	sis.sin_family = AF_INET;
	sis.sin_port = htons(OTHERPORT);
	inet_aton("localhost", &sis.sin_addr);


	strncpy(myStr, "Example_Final_string", BUFLEN);
	int sendStatus = sendto(s, myStr, sizeof(myStr), 0, (struct sockaddr *)&sis, sislen);

	close(s);

	return &result;
}
