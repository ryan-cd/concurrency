#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "ppmFile.h"

int clamp(int value, int min, int max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d"
           " out of %d processors\n",
           processor_name, world_rank, world_size);

    // Command-line arguments
    int blurRadius = strtol(argv[1], NULL, 10);
    char *inputFile = argv[2];
    char *outputFile = argv[3];

    // Variables set only in the root process
    Image *cleanImage = NULL;
    Image *blurredImage = NULL;
    unsigned char *cleanImageData = NULL; // For passing into scatter/gather
    unsigned char *blurredImageData = NULL;

    // Variables passed from the root process
    int sizePerThread;
    int sectionHeight;
    int sectionWidth;
    int remainderRows;

    if (world_rank == 0) {
        cleanImage = ImageRead(inputFile);
        blurredImage = ImageCreate(cleanImage->width, cleanImage->height);
        cleanImageData = cleanImage->data;
        blurredImageData = blurredImage->data;
        sizePerThread = (cleanImage->width*cleanImage->height*3)/world_size;
        sectionWidth = cleanImage->width;
        sectionHeight = cleanImage->height/world_size;
        remainderRows = cleanImage->height % world_size;
        if (remainderRows) {
            printf("warning: height not divisible by number of processes; todo: handle it.\n");
        }
        for (int i = 1; i < world_size; i++) {
            MPI_Send(&sizePerThread, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sectionHeight, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sectionWidth, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&sizePerThread, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sectionHeight, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sectionWidth, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Last thread will handle the remaining rows
        if (remainderRows && (world_rank == world_size - 1)) {
            sectionHeight += remainderRows;
            sizePerThread += cleanImage->width*remainderRows*3;
        }
    }

    // Image Sections
    Image *cleanSection = ImageCreate(sectionWidth, sectionHeight);
    Image *blurredSection = ImageCreate(sectionWidth, sectionHeight);

    // Scatter
    MPI_Scatter(cleanImageData, sizePerThread, MPI_UNSIGNED_CHAR, cleanSection->data, sizePerThread, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    unsigned char *pointer = (unsigned char *) malloc(sizePerThread);
    if (pointer == NULL) {
        printf("Error, memory full");
    }
    if (world_rank == 0) {
        for (int i = 1; i < world_size; i++) {
            // assign pointer to point to relevant section of cleanImageData

            MPI_Send(&pointer, sizePerThread, MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD);
            printf("Clean image data 1: %d, pointer 1: %d\n", cleanImageData[1], pointer[1]);
        }
    } else {
        MPI_Recv(&pointer, sizePerThread, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    free(pointer);

    // Do work
    int height = sectionHeight;
    int width = sectionWidth;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            // Bounds
            int minX = clamp(col - blurRadius, 0, width);
            int maxX = clamp(col + blurRadius, 0, width);
            int minY = clamp(row - blurRadius, 0, height);
            int maxY = clamp(row + blurRadius, 0, height);

            // For each channel (r,g,b)
            for (int channel = 0; channel < 3; ++channel) {
                int sum = 0;
                int numPixels = 0;
                // Take average of pixels
                for (int y = minY; y < maxY; ++y) {
                    for (int x = minX; x < maxX; ++x) {
                        sum += ImageGetPixel(cleanSection, x, y, channel);
                        numPixels += 1;
                    }
                }
                sum = clamp(sum/numPixels, 0, 255);
                // Write average into output
                ImageSetPixel(blurredSection, col, row, channel, sum);
            }
        }
    }

    // Gather
    MPI_Gather(blurredSection->data, sizePerThread, MPI_UNSIGNED_CHAR, blurredImageData, sizePerThread, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Write output to file
    if (world_rank == 0) {
        ImageWrite(blurredImage, outputFile);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
