#ifndef DATPNG_H
#define DATPNG_H

#include <stdlib.h>
#include <stdio.h>
#include <png.h>

/* Describes how data should be structured in the PNG. */
typedef struct datpng_info
{
  int bit_depth;
  int checksum;
  int x_pos;
  int y_pos;
  int data_width;
  int data_height;
  int png_width;
  int png_height;
} datpng_info;

extern int header_size;
extern short cur_ver;

int datpng_write(FILE *outfile, datpng_info *dat_info, 
		 void *data, size_t data_size);

int datpng_read(FILE *infile, datpng_info *dat_info, 
		void **data, size_t *data_size);

#endif
