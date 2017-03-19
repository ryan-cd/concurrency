#include <stdio.h>
#include <pthread.h>
#include "appendserver.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define OTHERPORT 1337
#define BUFLEN 2076

/* String */
typedef struct {
    char* str;
    unsigned int length;
    unsigned int index;
	unsigned int segmentIndex;
} Str;

/* Globals */
Str string;
AppendArgs appendArgs;
char enforcementChars[3];
bool stringSentToVerify = false;

/* Send string to the verify server */
void sendStringToVerify()
{
	int s;
	char myStr[BUFLEN];

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1)
	{
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
	inet_aton(appendArgs.verifyHostname, &sis.sin_addr);


	strncpy(myStr, string.str, BUFLEN);
	sendto(s, myStr, sizeof(myStr), 0, (struct sockaddr *)&sis, sislen);

	close(s);
}

/**
 * Returns whether the letter can be added with respect to the enforcement property.
 * Used within the critical section of writing a character
 */
bool canWrite(char letter, char* segment, size_t segLength, char c[3], size_t property)
{
	size_t c0Initial = 0;
    size_t c1Initial = 0;
    size_t c2Initial = 0;
    size_t cxInitial = 0;

    if (letter == c[0]) {
        c0Initial++;
    } else if (letter == c[1]) {
        c1Initial++;
    } else if (letter == c[2]) {
        c2Initial++;
    }

    if ((letter != c[0]) && (letter != c[1]) && (letter != c[2])) {
        cxInitial++;
    }

    for (int i = 0; i < segLength; i++) {
        if (c[0] == segment[i]) {
            c0Initial++;
        }
        else if (c[1] == segment[i]) {
            c1Initial++;
        }
        else if (c[2] == segment[i]) {
            c2Initial++;
        }
        // No more characters in this segment
        else if (segment[i] == 0) {
            break;
        //There is a letter not in c[3]
        } else {
            cxInitial++;
        }
    }

    for (int c0 = c0Initial; c0 <= segLength; c0++) {
        for (int c1 = c1Initial; c1 <= segLength; c1++) {
            for (int c2 = c2Initial; c2 <= segLength; c2++) {
                for (int cx = cxInitial; cx <= segLength; cx++) {
                    if (segLength != c0 + c1 + c2 + cx) {
                        continue;
                    }

                    switch(property) {
                        case 0:
                            if (c0 + c1 == c2)
                            {
                                return true;
                            }
                            break;
                        case 1:
                            if (c0 + 2*c1 == c2)
                            {
                                return true;
                            }
                            break;
                        case 2:
                            if (c0 * c1 == c2)
                            {
                                return true;
                            }
                            break;
                        case 3:
                            if (c0 - c1 == c2)
                            {
                                return true;
                            }
                            break;
                        default:
                            printf("Invalid property to check\n");
                            return false;
                    }
                }
            }
        }
    }

    return false;
}

/* RPC Procedure: int RPC_InitAppendServer(AppendArgs) */
int *rpc_initappendserver_1_svc(AppendArgs *args, struct svc_req *req)
{
	static int result = 1;
	printf("RPC_InitAppendServer\n");
	appendArgs = *args;

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

	// Check if there is space to append
	if (string.index >= string.length)
	{
		result = -1;
	}
	// Add letter (using enforcement)
	else
	{
		#pragma omp critical
		{
			if (canWrite(*letter, &string.str[string.segmentIndex], appendArgs.segLength, appendArgs.c, appendArgs.property))
			{
				string.str[string.index++] = *letter;
				string.segmentIndex = (string.index/appendArgs.segLength) * appendArgs.segLength;
			}
			result = 0;
		}
	}

	if (!stringSentToVerify && string.index >= string.length)
	{
		sendStringToVerify();
		stringSentToVerify = true;
	}
	return &result;
}


