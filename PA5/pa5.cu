/*
 * Don Pham - phamd
 * Ryan Davis - davisr3
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cuda.h>
#include "ppmFile.h"

//This error checking macro is from http://stackoverflow.com/questions/14038589/what-is-the-canonical-way-to-check-for-errors-using-the-cuda-runtime-api
#define CUDA_CHECK_ERROR(response) { check((response), __FILE__, __LINE__); }
inline void check(cudaError_t returnCode, const char *file, int line)
{
   if (returnCode != cudaSuccess) 
   {
       fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(returnCode), file, line);
       exit(returnCode);
   }
}

__device__ int clamp(int value, int min, int max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

__global__ void blur(int world_size, int blurRadius, int sectionWidth, int sectionHeight,
                     int remainderRows, unsigned char *cleanImageData, unsigned char *blurredImageData)
{
    int id = (blockIdx.z * gridDim.x * gridDim.y + blockIdx.y * gridDim.x + blockIdx.x) * blockDim.x + threadIdx.x;
    int imageHeight = sectionHeight * world_size + remainderRows;
    int imageWidth = sectionWidth;

    int sectionByteSize = sectionWidth * sectionHeight * 3;

    // For the rest of the processes
    unsigned char *cleanImagePtr = NULL;
    unsigned char *cleanImageEndPtr = cleanImageData + imageWidth * imageHeight * 3;

    // Pointer to the beginning of each process's unpadded section
    cleanImagePtr = cleanImageData + id * sectionByteSize; // id was 'i' in the for loop

    // paddedHeight is clamped so that it doesn't pass the absolute image bounds
    int rowsAbove = clamp((cleanImagePtr - cleanImageData) / 3 / sectionWidth, 0, INT_MAX);
    int rowsBelow = clamp((cleanImageEndPtr - cleanImagePtr - sectionByteSize) / 3 / sectionWidth, 0, INT_MAX);
    int paddedHeight = sectionHeight
                    + clamp(rowsAbove, 0, blurRadius)
                    + clamp(rowsBelow, 0, blurRadius + ((id == world_size - 1) ? remainderRows : 0));

    // Shift the pointer for the above-padding
    cleanImagePtr -= sectionWidth * clamp(rowsAbove, 0, blurRadius) * 3;

    // Pointers for each process to work with
    unsigned char *cleanSection = cleanImagePtr;
    unsigned char *blurredSection = blurredImageData + id * sectionByteSize;

    // Adjust sectionHeight for the last process, after calculating bounds
    if (id == world_size - 1) {
        sectionHeight += remainderRows;
    }

    printf("Hello world from %i!\n", id);

    // Do work on the image sections
    int topPaddingOffset = clamp(rowsAbove, 0, blurRadius);
    int bottomPaddingOffset = sectionHeight + topPaddingOffset;

    for (int row = topPaddingOffset; row < bottomPaddingOffset; ++row) {
        for (int col = 0; col < sectionWidth; ++col) {
            // Bounds
            int minX = clamp(col - blurRadius, 0, sectionWidth);
            int maxX = clamp(col + blurRadius, 0, sectionWidth);
            int minY = clamp(row - blurRadius, 0, paddedHeight);
            int maxY = clamp(row + blurRadius, 0, paddedHeight);

            // For each channel (r,g,b)
            for (int channel = 0; channel < 3; ++channel) { // TODO: flip loops for performance
                int sum = 0;
                int numPixels = 0;
                // Take average of pixels
                for (int y = minY; y <= maxY; ++y) {
                    for (int x = minX; x <= maxX; ++x) {
                        sum += cleanSection[(y * sectionWidth + x) * 3 + channel];
                        numPixels += 1;
                    }
                }
                sum = clamp(sum/numPixels, 0, 255);
                // Write average into output
                blurredSection[((row - topPaddingOffset) * sectionWidth + col) * 3 + channel] = sum;
            }
        }
    }
}

int main(int argc, char** argv) {
    int world_size = 32;
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

    // Variables passed from the root process
    int sectionWidth; // The section width (without padding).
    int sectionHeight; // The section height (without padding).

    cleanImage = ImageRead(inputFile);
    blurredImage = ImageCreate(cleanImage->width, cleanImage->height);
    cleanImageData = cleanImage->data;
    blurredImageData = blurredImage->data;

    sectionWidth = cleanImage->width;
    sectionHeight = cleanImage->height / world_size;
    remainderRows = cleanImage->height % world_size;
    unsigned char *cleanImageDataDevice = NULL;
    unsigned char *blurredImageDataDevice = NULL;
    CUDA_CHECK_ERROR(cudaMalloc ((void **) &cleanImageDataDevice, sizeof(unsigned char) * 3 * cleanImage->width * cleanImage->height));

    CUDA_CHECK_ERROR(cudaMalloc ((void **) &blurredImageDataDevice, sizeof(unsigned char) * 3 * cleanImage->width * cleanImage->height));
    CUDA_CHECK_ERROR(cudaMemcpy(cleanImageDataDevice, cleanImageData, sizeof(unsigned char) * 3 * cleanImage->width * cleanImage->height, cudaMemcpyHostToDevice));

    CUDA_CHECK_ERROR(cudaMemcpy(blurredImageDataDevice, blurredImageData, sizeof(unsigned char) * 3 * cleanImage->width * cleanImage->height, cudaMemcpyHostToDevice));
    blur<<<world_size, 1>>>(world_size, blurRadius, sectionWidth, sectionHeight, remainderRows, cleanImageDataDevice, blurredImageDataDevice);
    CUDA_CHECK_ERROR(cudaMemcpy(blurredImageData, blurredImageDataDevice, sizeof(unsigned char) * 3 * cleanImage->width * cleanImage->height, cudaMemcpyDeviceToHost));

    CUDA_CHECK_ERROR(cudaDeviceSynchronize());

    ImageWrite(blurredImage, outputFile);


    // Clean up
    CUDA_CHECK_ERROR(cudaFree ((void *) cleanImageDataDevice));
    CUDA_CHECK_ERROR(cudaFree ((void *) blurredImageDataDevice));
    free(cleanImage->data);
    free(cleanImage);
    free(blurredImage->data);
    free(blurredImage);

    return 1;
}
