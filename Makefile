CC = gcc
CFLAGS = -std=gnu99 -Wall
LDFLAGS = 
LDLIB = -lpthread
EXE = pa1.x
OBJ = pa1_main.o pa1_str.o

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIB) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXE) $(OBJ)

run: $(EXE)
	./$(EXE) 0 3 6 3 b c a
