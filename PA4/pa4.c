/*
 * Don Pham - phamd
 * Ryan Davis - davisr3
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
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
    unsigned char *cleanImageData = NULL; // Pointer to cleanImage->data
    unsigned char *blurredImageData = NULL; // Pointer to blurredImage->data
    int remainderRows = 0;

    // For gatherv
    int *rcounts = NULL; // Size of each section.
    int *displs = NULL; // Displacement of each section.

    // Variables passed from the root process
    int sectionWidth; // The section width (without padding).
    int sectionHeight; // The section height (without padding).
    int sendByteSize; // Number of bytes for the section (including padding).

    // Variables set in each process
    int sectionByteSize; // The size in bytes of the section (without padding).
    int paddedHeight; // The section height after padding the image.
    Image *cleanSection = NULL;
    Image *blurredSection = NULL;

    if (world_rank == 0)
    {
        cleanImage = ImageRead(inputFile);
        blurredImage = ImageCreate(cleanImage->width, cleanImage->height);
        cleanImageData = cleanImage->data;
        blurredImageData = blurredImage->data;

        sectionWidth = cleanImage->width;
        sectionHeight = cleanImage->height/world_size;
        remainderRows = cleanImage->height % world_size;

        // Set up sizes for gatherv
        rcounts = malloc(world_size * sizeof(int));
        displs = malloc(world_size * sizeof(int));
        for (int i = 0; i < world_size; i++) {
            rcounts[i] = sectionWidth * sectionHeight * 3;
            displs[i] = i * rcounts[i];
        } // The last section is potentially larger than the rest
        rcounts[world_size-1] = sectionWidth * (sectionHeight + remainderRows) * 3;

        // Send parameters
        for (int i = 1; i < world_size; i++) {
            if (i == world_size - 1) { // Last process gets the remainder rows
                sectionHeight += remainderRows;
            }
            MPI_Send(&sectionWidth, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sectionHeight, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        // Reset sectionHeight for the root process
        sectionHeight -= remainderRows;
    }
    else
    {
        MPI_Recv(&sectionWidth, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sectionHeight, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Process variables
    sectionByteSize = sectionWidth * sectionHeight * 3;
    blurredSection = ImageCreate(sectionWidth, sectionHeight);

    // Scatter the image
    if (world_rank == 0)
    {
        // For root process
        paddedHeight = clamp(sectionHeight + ((world_size == 1) ? 0 : blurRadius), 0, cleanImage->height);
        cleanSection = ImageCreate(sectionWidth, paddedHeight);
        free(cleanSection->data);
        cleanSection->data = cleanImageData; // Directly use cleanImageData

        // For the rest of the processes
        unsigned char *cleanImagePtr = NULL;
        unsigned char *cleanImageEndPtr = cleanImageData + cleanImage->width * cleanImage->height * 3;

        for (int i = 1; i < world_size; i++) {
            // Pointer to the beginning of each process's unpadded section
            cleanImagePtr = cleanImageData + i * sectionByteSize;

            // paddedHeight is clamped so that it doesn't pass the absolute image bounds
            int rowsAbove = clamp((cleanImagePtr - cleanImageData) / 3 / sectionWidth, 0, INT_MAX);
            int rowsBelow = clamp((cleanImageEndPtr - cleanImagePtr - sectionByteSize) / 3 / sectionWidth, 0, INT_MAX);
            paddedHeight = sectionHeight
                            + clamp(rowsAbove, 0, blurRadius)
                            + clamp(rowsBelow, 0, blurRadius + ((i == world_size - 1) ? remainderRows : 0));

            // Shift the pointer for the above-padding
            cleanImagePtr -= sectionWidth * clamp(rowsAbove, 0, blurRadius) * 3;

            // Calculate the size that the process will recieve
            sendByteSize = sectionWidth * paddedHeight * 3;

            MPI_Send(&sendByteSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(cleanImagePtr, sendByteSize, MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&sendByteSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        paddedHeight = sendByteSize / sectionWidth / 3;
        cleanSection = ImageCreate(sectionWidth, paddedHeight); // Allocate space for the data
        MPI_Recv(cleanSection->data, sendByteSize, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Do work on the image sections
    int topPaddingOffset = (world_rank == 0) ? 0 : blurRadius;
    int bottomPaddingOffset = sectionHeight + ((world_rank == 0) ? 0 : blurRadius);

    for (int row = topPaddingOffset; row < bottomPaddingOffset; ++row) {
        for (int col = 0; col < sectionWidth; ++col) {
            // Bounds
            int minX = clamp(col - blurRadius, 0, sectionWidth);
            int maxX = clamp(col + blurRadius, 0, sectionWidth);
            int minY = clamp(row - blurRadius, 0, paddedHeight);
            int maxY = clamp(row + blurRadius, 0, paddedHeight);

            // For each channel (r,g,b)
            for (int channel = 0; channel < 3; ++channel) {
                int sum = 0;
                int numPixels = 0;
                // Take average of pixels
                for (int y = minY; y <= maxY; ++y) {
                    for (int x = minX; x <= maxX; ++x) {
                        //sum += ImageGetPixel(cleanSection, x, y, channel);
                        sum += cleanSection->data[(y * sectionWidth + x) * 3 + channel]; // In-lined
                        numPixels += 1;
                    }
                }
                sum = clamp(sum/numPixels, 0, 255);
                // Write average into output
                //ImageSetPixel(blurredSection, col, row - topPaddingOffset, channel, sum);
                blurredSection->data[((row - topPaddingOffset) * sectionWidth + col) * 3 + channel] = sum;  // In-lined
            }
        }
    }

    // Gather
    MPI_Gatherv(blurredSection->data, sectionByteSize, MPI_UNSIGNED_CHAR, blurredImageData, rcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Write output to file
    if (world_rank == 0) {
        ImageWrite(blurredImage, outputFile);
    }

    // Clean up
    if (world_rank == 0) {
        free(cleanImage->data);
        free(cleanImage);
        free(blurredImage->data);
        free(blurredImage);
        free(cleanSection); // cleanSection->data was already freed
        free(blurredSection->data);
        free(blurredSection);
        free(rcounts);
        free(displs);
    } else {
        free(cleanSection->data);
        free(cleanSection);
        free(blurredSection->data);
        free(blurredSection);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
