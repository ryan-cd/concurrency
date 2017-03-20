/**
 *  Don Pham - phamd
 *  Ryan Davis - davisr3
 */

 struct AppendArgs {
    int property; /* f */
    int segLength; /* L */
    int numSegments; /* M */
    int numThreads; /* N */
    char c[3]; /* C0, C1, C2 */
    char *verifyHostname; /* hostname2 */
};

program RPC_AppendServer {
    version RPC_AppendServer_VERS {
        int RPC_InitAppendServer(AppendArgs) = 1;
        int RPC_Append(char) = 2;
    } = 1;
} = 0x20000001;
