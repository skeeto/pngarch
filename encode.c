#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <byteswap.h>

#include "datpng.h"

int data_size;

int datpng_write(char *filename, datpng_info *dat_info, 
		 void *data, size_t data_size)
{
  int byte_depth = 1;
  int rowbytes = dat_info->data_width * 3 * byte_depth;
  
  /* Prepare the image data */
  png_bytep *row_pointers; /* Image data */
  int rp_height = data_size / rowbytes + 1; /* XXX special case needed */
  
  if (dat_info->png_height > 0 &&
      rp_height > dat_info->png_height)
    rp_height = dat_info->png_height;
  
  row_pointers = (png_bytep *)
    malloc(sizeof(png_bytep) * rp_height);
  
  FILE *fw = fopen(filename, "wb");
  
  /* Initialize png structs. */
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, fw);
  
  /* Load data into row_pointers image data. */
  int data_len, offset;
  int i;
  void *next_data = data;
  for (i = 0; i < rp_height; i++)
    {
      row_pointers[i] = (png_bytep) calloc(1, rowbytes);
      
      if (i == 0)
	{
	  int size = (int)data_size;
	  memcpy(row_pointers[0], &size, 4);
	  offset = 4;
	}
      else
	offset = 0;
      
      /* 8-bit depth only: */ 

      data_len = (i + 1) * rowbytes - 4;
      if (data_len > data_size)
	data_len -= data_size;
      else
	data_len = rowbytes - offset;
      
      memcpy(row_pointers[i] + offset, next_data, data_len);
      next_data += data_len;
    }

  /* Set image meta data. */
  info_ptr->width = dat_info->png_width;
  info_ptr->height = rp_height;
  info_ptr->valid = 0;
  info_ptr->rowbytes = rowbytes;
  info_ptr->palette = NULL;
  info_ptr->num_palette = 0;
  info_ptr->num_trans = 0;
  info_ptr->bit_depth = dat_info->bit_depth;
  info_ptr->color_type = PNG_COLOR_TYPE_RGB;
  info_ptr->compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  info_ptr->filter_type = PNG_FILTER_TYPE_DEFAULT;
  info_ptr->interlace_type = PNG_INTERLACE_NONE;
  
  png_write_info(png_ptr, info_ptr);
  
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[write_png_file] Error during write\n");
  
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);
  
  fclose(fw);
  
  return EXIT_SUCCESS;
}
