.DEFAULT_GOAL := all

CC = gcc
CFLAGS = -Wall -Wno-unused-variable
LDLIBS = -lnsl

CLIENT = client
APPEND = server_append
VERIFY = server_verify
RPC = .phony

SOURCES_CLIENT = client.c appendserver_clnt.c appendserver_xdr.c verifyserver_clnt.c verifyserver_xdr.c
SOURCES_APPEND = appendserver_proc.c appendserver_svc.c appendserver_clnt.c appendserver_xdr.c
SOURCES_VERIFY = verifyserver_proc.c verifyserver_svc.c verifyserver_clnt.c verifyserver_xdr.c

all : $(RPC) $(CLIENT) $(APPEND) $(VERIFY)

# TODO: split the compile and linking steps

$(CLIENT): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(APPEND): $(SOURCES_APPEND)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(VERIFY): $(SOURCES_VERIFY)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# RPCGEN
$(RPC): appendserver.x verifyserver.x
	rpcgen appendserver.x
	rpcgen verifyserver.x

# CLEANUP
clean:
	rm $(CLIENT) $(APPEND) $(VERIFY) \
		appendserver_svc.c appendserver_clnt.c appendserver_xdr.c appendserver.h \
		verifyserver_svc.c verifyserver_clnt.c verifyserver_xdr.c verifyserver.h


