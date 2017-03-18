#include <stdio.h>
#include "appendserver.h"

#define OTHERPORT 1337
#define BUFLEN 2076

int *rpc_initappendserver_1_svc(AppendArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitAppendServer\n");

	return &result;
}

int *rpc_append_1_svc(char *letter, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_Append\n");

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

	//strIP = "127.0.0.1";
	struct sockaddr_in sis;
	int sislen=sizeof(sis);
	memset((char *) &sis, 0, sizeof(sis));
	sis.sin_family = AF_INET;
	sis.sin_port = htons(OTHERPORT);
	inet_aton("127.0.0.1", &sis.sin_addr);

    /*while (1) {
        printf("Enter a message:");
        scanf("%s", myStr);
        sendto(s, myStr, sizeof(myStr), 0, (struct sockaddr *)&sis, sislen);
    }*/


	return &result;
}