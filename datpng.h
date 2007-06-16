#ifndef DATPNG_H
#define DATPNG_H

#include <stdlib.h>
#include <stdio.h>
#include <png.h>

/* Describes how data should be structured in the PNG. */
typedef struct datpng_info
{
  char *filename;
  int bit_depth;
  int checksum;
  int x_pos;
  int y_pos;
  int data_width;
  int data_height;
  int png_width;
  int png_height;
} datpng_info;

int datpng_write(char *filename, datpng_info *dat_info, 
		 void *data, size_t data_size);

#endif
