CC = gcc
CFLAGS = -std=gnu99 -Wall -fopenmp
LDFLAGS = -fopenmp
LDLIB =
EXE = pa2.x
OBJ = pa2.o pa_str.o

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIB) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXE) $(OBJ)

run: $(EXE)
	./$(EXE) 0 3 6 3 b c a
