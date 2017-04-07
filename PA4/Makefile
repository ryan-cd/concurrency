.DEFAULT_GOAL := all
MPICC = mpicc
CFLAGS = -std=c99 -g
LDFLAGS =
LDLIBS =

# FILES
EXE = pa4.x
SRC = pa4.c ppmFile.c
OBJ = $(SRC:%.c=%.o)

# EXAMPLE RUN
run: $(EXE)
	mpirun -np 2 ./$(EXE) 5 fox.ppm fox_blur.ppm

# MAKE ALL
all: $(EXE)

# LINK
$(EXE): $(OBJ)
	$(MPICC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# COMPILE
%.o: %.c
	$(MPICC) $(CFLAGS) -c $^ -o $@

# CLEAN
clean:
	rm $(EXE) $(OBJ)
