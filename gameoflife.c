/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				Farhad Alemi - Full Implementation
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

#define FAIL_CODE -1
#define NUM_ARGS 3
#define RULE_BASE 16
#define NUM_NEIGHBORS 8
#define CHANNEL_WIDTH 8
#define MAX_RULE 0x3FFFF

//frees portion of allocated space.
extern void freePortion(Color **img_arr, size_t limit);

//Wraps the index.
size_t wrapIndex(int index, int maxValExc) {
    if (index < 0) {
        index += maxValExc;
    }
    if (index >= maxValExc) {
        index %= maxValExc;
    }
    return index;
}

//returns the pixel at the given row and col.
Color *getPixel(Image *img, int row, int col, int maxCol) {
    return *(img->image + (row * maxCol + col));
}

//returns the neighbors for given pixel.
Color **getNeighbors(Image *img, int row, int col) {
    Color **neighbors = (Color **) malloc(sizeof(uint32_t) * 3 * NUM_NEIGHBORS); // 8 neighbors in a 2-D plane.
    if (neighbors == NULL) {
        return NULL;
    }

    *(neighbors + 0) = getPixel(img, wrapIndex(row - 1, img->rows), wrapIndex(col - 1, img->cols), img->cols);
    *(neighbors + 1) = getPixel(img, wrapIndex(row - 1, img->rows), wrapIndex(col + 0, img->cols), img->cols);
    *(neighbors + 2) = getPixel(img, wrapIndex(row - 1, img->rows), wrapIndex(col + 1, img->cols), img->cols);

    *(neighbors + 3) = getPixel(img, wrapIndex(row - 0, img->rows), wrapIndex(col - 1, img->cols), img->cols);
    *(neighbors + 4) = getPixel(img, wrapIndex(row - 0, img->rows), wrapIndex(col + 1, img->cols), img->cols);

    *(neighbors + 5) = getPixel(img, wrapIndex(row + 1, img->rows), wrapIndex(col - 1, img->cols), img->cols);
    *(neighbors + 6) = getPixel(img, wrapIndex(row + 1, img->rows), wrapIndex(col + 0, img->cols), img->cols);
    *(neighbors + 7) = getPixel(img, wrapIndex(row + 1, img->rows), wrapIndex(col + 1, img->cols), img->cols);

    return neighbors;
}

//Returns the bit pattern for number of live neighbors.
size_t numLiveNeighbors(Color **neighbors, size_t bit) {
    size_t numAlive = 0;

    if (bit >= 2 * CHANNEL_WIDTH) {
        for (int j = 0; j < NUM_NEIGHBORS; ++j) {
            numAlive = (((*(neighbors + j))->R & (1U << (bit - 2 * CHANNEL_WIDTH))) != 0) ? numAlive + 1 : numAlive;
        }
    } else if (bit >= CHANNEL_WIDTH) {
        for (int j = 0; j < NUM_NEIGHBORS; ++j) {
            numAlive = (((*(neighbors + j))->G & (1U << (bit - CHANNEL_WIDTH))) != 0) ? numAlive + 1 : numAlive;
        }
    } else {
        for (int j = 0; j < NUM_NEIGHBORS; ++j) {
            numAlive = (((*(neighbors + j))->B & (1U << bit)) != 0) ? numAlive + 1 : numAlive;
        }
    }
    return (1U << numAlive);
}

//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row
//as adjacent to the bottom row and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule) {
    Color  *newPixel, **neighbors;
    size_t neighborsBitPattern;

    neighbors = getNeighbors(image, row, col);
    if (neighbors == NULL) {
        return NULL;
    }

    newPixel = (Color *) malloc(sizeof(uint32_t) * 3);
    if (newPixel == NULL) {
        return NULL;
    }

    newPixel->R = 0;
    for (int index = 3 * CHANNEL_WIDTH - 1; index >= 2 * CHANNEL_WIDTH; --index) {
        size_t newBit;

        size_t currAlive = getPixel(image, row, col, image->cols)->R & (1U << (index - 2 * CHANNEL_WIDTH));
        neighborsBitPattern = numLiveNeighbors(neighbors, index);

        if (currAlive) {
            neighborsBitPattern = neighborsBitPattern << (NUM_NEIGHBORS + 1U);
        }

        newBit = ((neighborsBitPattern & rule) != 0) ? 1 : 0;
        newPixel->R = newPixel->R | newBit;

        if (index != 2 * CHANNEL_WIDTH) {
            newPixel->R = newPixel->R << 1U;
        }
    }

    newPixel->G = 0;
    for (int index = 2 * CHANNEL_WIDTH - 1; index >= CHANNEL_WIDTH; --index) {
        size_t newBit;

        size_t currAlive = getPixel(image, row, col, image->cols)->G & (1U << (index - CHANNEL_WIDTH));
        neighborsBitPattern = numLiveNeighbors(neighbors, index);

        if (currAlive) {
            neighborsBitPattern = neighborsBitPattern << (NUM_NEIGHBORS + 1U);
        }

        newBit = ((neighborsBitPattern & rule) != 0) ? 1 : 0;
        newPixel->G = newPixel->G | newBit;

        if (index != CHANNEL_WIDTH) {
            newPixel->G = (newPixel->G << 1U);
        }
    }

    newPixel->B = 0;
    for (int index = CHANNEL_WIDTH - 1; index >= 0; --index) {
        size_t newBit;

        size_t currAlive = getPixel(image, row, col, image->cols)->B & (1U << index);
        neighborsBitPattern = numLiveNeighbors(neighbors, index);

        if (currAlive) {
            neighborsBitPattern = neighborsBitPattern << (NUM_NEIGHBORS + 1U);
        }

        newBit = ((neighborsBitPattern & rule) != 0) ? 1 : 0;
        newPixel->B = newPixel->B | newBit;

        if (index != 0) {
            newPixel->B = (newPixel->B << 1U);
        }
    }
    free(neighbors);

    return newPixel;
}

// The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
// You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule) {
    Image *img_next;

    if (image == NULL) {
        return NULL;
    }

    img_next = (Image *) malloc(sizeof(struct Image));
    if (img_next == NULL) {
        return NULL;
    }

    img_next->image = (Color **) malloc(sizeof(uint32_t) * 3 * image->rows * image->cols);
    if (img_next->image == NULL) {
        free(img_next);
        return NULL;
    }

    img_next->cols = image->cols;
    img_next->rows = image->rows;

    for (int i = 0; i < img_next->rows * img_next->cols; ++i) {
        *(img_next->image + i) = evaluateOneCell(image, i / image->cols, i % image->cols, rule);
        if (*(img_next->image + i) == NULL) {
            freePortion(img_next->image, i - 1);
            free(img_next);
            return NULL;
        }
    }
    return img_next;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
    char  **_ = NULL;
    Image *img, *img_next;

    if (argc != NUM_ARGS || strtol(argv[2], _, RULE_BASE) > MAX_RULE) {
        printf("usage: ./gameOfLife filename rule\n");
        printf("filename is an ASCII PPM file (type P3) with maximum value 255.\n");
        printf("rule is a hex number beginning with 0x; Life is 0x1808.");
        return FAIL_CODE;
    }

    img = readData(argv[1]);
    if (img == NULL) {
        return FAIL_CODE;
    }

    img_next = life(img, strtol(argv[2], _, RULE_BASE));
    if (img_next == NULL) {
        freeImage(img);
        return FAIL_CODE;
    }

    writeData(img_next);
    freeImage(img);
    freeImage(img_next);
}

