/************************************************************************
**
** NAME:        steganography.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**				Justin Yokota - Starter Code
**				Farhad Alemi - Full Implementation
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

#define MAX_INTENSITY 255
#define FAIL_CODE -1
#define NUM_ARGS 2

//frees portion of allocated space.
extern void freePortion(Color **img_arr, size_t limit);

//helper method which validates entries
extern void validateNotNULL(void *args);

//Determines what color the cell at the given row/col should be. This should not affect Image, and should allocate space for a new Color.
Color *evaluateOnePixel(Image *image, int row, int col)
{
    Color   *pixel;
    uint8_t bitColor;

	validateNotNULL(image);

    bitColor = (*(image->image + (row * image->cols + col)))->B;
	pixel = (Color *) malloc(sizeof(uint32_t) * 3);
	if (pixel == NULL) {
	    return NULL;
	}

	if (bitColor % 2 == 0) {
	    pixel->R = 0;
	    pixel->G = 0;
	    pixel->B = 0;
	} else {
        pixel->R = MAX_INTENSITY;
        pixel->G = MAX_INTENSITY;
        pixel->B = MAX_INTENSITY;
	}
	return pixel;
}

//Given an image, creates a new image extracting the LSB of the B channel.
Image *steganography(Image *image)
{
    Image *deciphered_img;

    if (image == NULL) {
        return NULL;
    }

    deciphered_img = (Image *) malloc(sizeof(struct Image));
    if (deciphered_img == NULL) {
        return NULL;
    }

    deciphered_img->image = (Color **) malloc(sizeof(uint32_t) * 3 * image->rows * image->cols);
    if (deciphered_img->image == NULL) {
        free(deciphered_img);
        return NULL;
    }
    deciphered_img->cols = image->cols;
    deciphered_img->rows = image->rows;

    for (int i = 0; i < deciphered_img->rows * deciphered_img->cols; ++i) {
        *(deciphered_img->image + i) = evaluateOnePixel(image, i / image->cols, i % image->cols);
        if (*(deciphered_img->image + i) == NULL) {
            freePortion(deciphered_img->image, i - 1);
            free(deciphered_img);
            return NULL;
        }
    }
    return deciphered_img;
}

/*
Loads a file of ppm P3 format from a file, and prints to stdout (e.g. with printf) a new image, 
where each pixel is black if the LSB of the B channel is 0, 
and white if the LSB of the B channel is 1.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a file of ppm P3 format (not necessarily with .ppm file extension).
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!
*/
int main(int argc, char **argv)
{
	Image *img, *deciphered_img;

    if (argc != NUM_ARGS) {
        return FAIL_CODE;
    }

	img = readData(argv[1]);
    if (img == NULL) {
        return FAIL_CODE;
    }
	deciphered_img = steganography(img);
    if (deciphered_img == NULL) {
        freeImage(img);
        return FAIL_CODE;
    }

	writeData(deciphered_img);
    freeImage(img);
	freeImage(deciphered_img);
}

