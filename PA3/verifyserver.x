/**
 * RPC InitVerifyServer to send [N, L, M] parameters to the verify server.
 */
program RPC_VerifyServer {
     version RPC_VerifyServer_VERS {
        int RPC_InitVerifyServer(string) = 1;
        int RPC_GetSeg(int) = 2;
 	 } = 1;
} = 0x20000002;
