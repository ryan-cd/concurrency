.DEFAULT_GOAL := all
CC = gcc
CFLAGS = -std=gnu99 -Wall -fopenmp
LDFLAGS = -fopenmp
LDLIBS = -lnsl

# EXAMPLE RUN
run:
	./client 0 4 12 4 a b c localhost localhost

# THREE PROGRAMS
CLIENT.exe = client
APPEND.exe = server_append
VERIFY.exe = server_verify

# SOURCE FILES FOR EACH PROGRAM
SOURCES_CLIENT = client.c appendserver_clnt.c appendserver_xdr.c verifyserver_clnt.c verifyserver_xdr.c
SOURCES_APPEND = appendserver_proc.c appendserver_svc.c appendserver_clnt.c appendserver_xdr.c
SOURCES_VERIFY = verifyserver_proc.c verifyserver_svc.c verifyserver_clnt.c verifyserver_xdr.c

# OBJECT FILES FOR EACH PROGRAM
OBJS_CLIENT = $(SOURCES_CLIENT:%.c=%.o)
OBJS_APPEND = $(SOURCES_APPEND:%.c=%.o)
OBJS_VERIFY = $(SOURCES_VERIFY:%.c=%.o)

# RPCGEN
RPCGEN_APPEND.input = appendserver.x
RPCGEN_VERIFY.input = verifyserver.x
RPCGEN_APPEND.output = appendserver.h appendserver_svc.c appendserver_clnt.c appendserver_xdr.c
RPCGEN_VERIFY.output = verifyserver.h verifyserver_svc.c verifyserver_clnt.c verifyserver_xdr.c

# MAKE ALL
all: $(RPCGEN_APPEND.output) $(RPCGEN_VERIFY.output) $(CLIENT.exe) $(APPEND.exe) $(VERIFY.exe)

# LINK
$(CLIENT.exe): $(OBJS_CLIENT)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(APPEND.exe): $(OBJS_APPEND)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(VERIFY.exe): $(OBJS_VERIFY)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# COMPILE
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# RPCGEN
$(RPCGEN_APPEND.output): $(RPCGEN_APPEND.input)
	rpcgen $<

$(RPCGEN_VERIFY.output): $(RPCGEN_VERIFY.input)
	rpcgen $<

# CLEANUP
clean:
	rm -f $(CLIENT.exe) $(APPEND.exe) $(VERIFY.exe) \
		$(OBJS_CLIENT) $(OBJS_APPEND) $(OBJS_VERIFY) \
		$(RPCGEN_APPEND.output) $(RPCGEN_VERIFY.output)
