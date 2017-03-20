/**
 *  Don Pham - phamd
 *  Ryan Davis - davisr3
 */

struct VerifyArgs {
    int numThreads; /* N */
    int segLength; /* L */
    int numSegments; /* M */
};

const LLBUFSIZE = 1024;

struct LLString {
    int bytesLeft;
    opaque buffer[LLBUFSIZE]; /* Opaque is an arbitrary sequence of bytes. */
    struct LLString *next;
};

program RPC_VerifyServer {
    version RPC_VerifyServer_VERS {
        int RPC_InitVerifyServer(VerifyArgs) = 1;
        LLString RPC_GetSeg(int) = 2;
        LLString RPC_GetString(int) = 3;
    } = 1;
} = 0x20000002;
