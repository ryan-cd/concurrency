CC = gcc
CFLAGS = -std=c99 #-Wall
LDFLAGS = 
LDLIB = -lpthread

pa1.x: pa1_main.o pa1_str.o
	$(CC) $^ ${LDFLAGS} ${LDLIB} -o $@

%.o: %.c
	$(CC) -c ${CFLAGS} $< ${LDFLAGS} ${LDLIB}