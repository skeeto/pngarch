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
} datpng_info;

int datpng_write(char *filename, datpng_info *dat_info, void *data);

#endif
