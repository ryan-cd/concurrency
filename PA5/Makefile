.DEFAULT_GOAL := all
CC = gcc
CFLAGS = -std=c99 -g
LDFLAGS =
LDLIBS =
NVCC = nvcc
CUDA_CFLAGS = --device-c -arch=sm_20
CUDA_LDFLAGS = -rdc=true -arch=sm_20
# `--device-c` === `-rdc=true -c`

# FILES
EXE = pa5.x
SRC = pa5.cu ppmFile.c
OBJ = pa5.o ppmFile.o

# MAKE ALL
all: $(EXE)

# LINK
$(EXE): $(OBJ)
	$(NVCC) $(CUDA_LDFLAGS) $^ $(LDLIBS) -o $@

# COMPILE
%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.o: %.cu
	$(NVCC) $(CUDA_CFLAGS) -c $^ -o $@

# CLEAN
clean:
	rm $(EXE) $(OBJ)
