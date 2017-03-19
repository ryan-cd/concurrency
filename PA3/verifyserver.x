/**
 * RPC InitVerifyServer to send [N, L, M] parameters to the verify server.
 */

struct VerifyArgs {
	int numThreads; /* N */
	int segLength; /* L */
	int numSegments; /* M */
};

struct LLString {
    int bytesLeft;
    opaque buffer[1024];
    struct LLString *next;
};

program RPC_VerifyServer {
     version RPC_VerifyServer_VERS {
        int RPC_InitVerifyServer(VerifyArgs) = 1;
        LLString RPC_GetSeg(int) = 2;
        LLString RPC_GetString(int) = 3;
 	 } = 1;
} = 0x20000002;
