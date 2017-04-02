.DEFAULT_GOAL := all
MPICC = mpicc
LDFLAGS =
LDLIBS =

# FILES
EXE = pa4.x
SRC = pa4.c
OBJ = $(SRC:%.c=%.o)

# EXAMPLE RUN
run: $(EXE)
	mpirun -np 4 ./$(EXE) r fox.ppm fox_blur.ppm

# MAKE ALL
all: $(EXE)

# LINK
$(EXE): $(OBJ)
	$(MPICC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# COMPILE
%.o: %.c
	$(MPICC) -c $^ -o $@

# CLEAN
clean:
	rm $(EXE) $(OBJ)