/**
 * Remote procedure RPC_InitAppendServer to send [f, L, M, C0, C1, C2,
 * host_name2] parameters to the append server.
 */
program RPC_AppendServer {
     version RPC_AppendServer_VERS {
        int RPC_InitAppendServer(string) = 1;
        int RPC_Append(char) = 2;
 	 } = 1;
} = 0x20000001;
