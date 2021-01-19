/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				Farhad Alemi
**
**
** DATE:        2020-08-15
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "imageloader.h"

#define FILE_TYPE "P3"
#define MAX_INTENSITY 255
#define FAIL_CODE -1


//helper method which validates entries
void validateNotNULL(void *args) {
    if (args == NULL) {
        exit(FAIL_CODE);
    }
}

//frees portion of allocated space.
void freePortion(Color **img_arr, size_t limit) {
    for (int i = 0; i < limit; ++i) {
        free(*(img_arr + i));
    }
}

//Opens a .ppm P3 image file, and constructs an Image object. 
//You may find the function fscanf useful.
//Make sure that you close the file with fclose before returning.
Image *readData(char *filename) 
{
    char     file_type[255];
    FILE     *file_ptr;
    Image    *img;
    uint32_t rows, cols, max_intensity;

    file_ptr = fopen( filename, "r");
    if (file_ptr == NULL) {
        printf("file %s not found...\n", filename);
        return NULL;
    }

    img = malloc(sizeof(Image));
    if (img == NULL) {
        fclose(file_ptr);
        return NULL;
    }
    fscanf(file_ptr, "%s %u %u %u", file_type, &cols, &rows, &max_intensity);

    if (strcmp(file_type, FILE_TYPE) != 0) {
        free(img);
        fclose(file_ptr);
        return NULL;
    }

    img->rows = rows;
    img->cols = cols;
    img->image = (Color **) malloc(sizeof(uint32_t) * 3 * rows * cols);
    if (img->image == NULL) {
        free(img);
        fclose(file_ptr);
        return NULL;
    }

    for (int i = 0; i < rows * cols; ++i) {
        *(img->image + i) = (Color *) malloc(sizeof(uint32_t) * 3);
        Color *pixel = *(img->image + i);
        if (pixel == NULL) {
            freePortion(img->image, i);
            free(img);
            fclose(file_ptr);
            return NULL;
        }

        int stream_signal = fscanf(file_ptr, "%hhu %hhu %hhu", &pixel->R, &pixel->G, &pixel->B);

        if(stream_signal == EOF) {
            freePortion(img->image, i);
            free(img);
            fclose(file_ptr);
            return NULL;
        }
    }
    fclose(file_ptr);

    return img;
}

//Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the image's data.
void writeData(Image *image)
{
    printf("%s\n", FILE_TYPE);
    printf("%u %u\n", image->cols, image->rows);
    printf("%u\n", MAX_INTENSITY);

    for (int i = 0; i < image->rows * image->cols; ++i) {
        Color *pixel = *(image->image + i);
        printf("%3hhu %3hhu %3hhu", pixel->R, pixel->G, pixel->B);
        if ((i + 1) % image->cols == 0) {
            printf("\n");
        } else {
            printf("   ");
        }
    }
}

//Frees an image
void freeImage(Image *image)
{
    for (int i = 0; i < (image->rows * image->cols); ++i) {
        free(*(image->image + i));
    }
    free(image->image);
	free(image);
}

