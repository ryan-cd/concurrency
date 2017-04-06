#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "ppmFile.h"

int clamp(int value, int min, int max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

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

    int blurRadius = strtol(argv[1], NULL, 10);
    char *inputFile = argv[2];
    char *outputFile = argv[3];

    // Read in PPM
    Image *cleanImage = ImageRead(inputFile);

    // Scatter

    // SEQUENTIAL VERSION
    int height = cleanImage->height;
    int width = cleanImage->width;
    Image *blurredImage = ImageCreate(width, height);

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
                        sum += ImageGetPixel(cleanImage, x, y, channel);
                        numPixels += 1;
                    }
                }
                sum = clamp(sum/numPixels, 0, 255);
                // Write average into output
                ImageSetPixel(blurredImage, col, row, channel, sum);
            }
        }
    }

    // Gather
    ImageWrite(blurredImage, outputFile);

    // Finalize the MPI environment.
    MPI_Finalize();
}