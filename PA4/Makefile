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
run: $(EXE) fox.ppm
	mpirun -np 2 ./$(EXE) 5 fox.ppm fox_blur.ppm

# Test for remainder section and no radius
test1: $(EXE) fox.ppm
	mpirun -np 17 ./$(EXE) 0 fox.ppm fox_blur.ppm

# Test for proper padding when (blurRadius > sectionHeight)
test2: $(EXE) fox.ppm
	mpirun -np 32 ./$(EXE) 200 fox.ppm fox_blur.ppm

# Test for one process
test3: $(EXE) fox.ppm
	mpirun -np 1 ./$(EXE) 5 fox.ppm fox_blur.ppm

# Test for 4k image
test4: $(EXE) fox_4k.ppm
	mpirun -np 16 ./$(EXE) 10 fox_4k.ppm fox_4k_blur.ppm

fox_4k.ppm: fox.ppm
	convert $< -resize 3840x2160 $@

# Test for 10x10 image and when (world_size > sectionHeight)
test5: $(EXE) calibrate.ppm
	mpirun -np 20 ./$(EXE) 3 calibrate.ppm calibrate_blur.ppm

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
