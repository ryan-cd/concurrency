/**
 * Remote procedure RPC_InitAppendServer to send [f, L, M, C0, C1, C2,
 * host_name2] parameters to the append server.
 */

 struct AppendArgs {
	int property; /* f */
	int segLength; /* L */
	int numSegments; /* M */
	char c0;
    char c1;
    char c2;
};

program RPC_AppendServer {
     version RPC_AppendServer_VERS {
        int RPC_InitAppendServer(AppendArgs) = 1;
        int RPC_Append(char) = 2;
 	 } = 1;
} = 0x20000001;
