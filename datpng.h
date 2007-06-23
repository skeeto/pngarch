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
  int ignore_checksum;
  int no_warnings;
} datpng_info;

extern int header_size;
extern short cur_ver;

int datpng_write(FILE *outfile, datpng_info *dat_info, 
		 void *data, size_t data_size);

int datpng_read(FILE *infile, datpng_info *dat_info, 
		void **data, size_t *data_size);

int datpng_autoread(FILE *infile, datpng_info *dat_info, 
		    void **data, size_t *data_size);

/* Error codes - Code < 0 are warnings. Codes > 0 are errors. */
#define PNGDAT_SUCCESS 0

#define PNGDAT_MISSING_CONSTRAINT 1
#define PNGDAT_FAILED_AUTOFIND 2
#define PNGDAT_CORRUPT_DATA 3
#define PNGDAT_CORRUPT_PNG 4
#define PNGDAT_FAILED_CHECKSUM 5
#define PNGDAT_BAD_VERSION 6

#define PNGDAT_TRUNCATED -1

#endif
