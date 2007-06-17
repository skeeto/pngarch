#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <byteswap.h>

#include "datpng.h"

int datpng_read(char *filename, datpng_info *dat_info, 
		void **data, size_t *data_size)
{
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  
  FILE *fp = fopen(filename, "rb");
  
  png_init_io(png_ptr, fp);
  
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  png_bytep *row_pointers;
  row_pointers = png_get_rows(png_ptr, info_ptr);
  
  /* Extra data size from the header. */
  *data_size = (size_t)((int)*((int *)row_pointers[0]));
  
  fprintf(stderr, "SIZE: %d\n", *data_size);
  
  *data = calloc(1, *data_size);
  
  int byte_depth = 1; /* 8-bit depth */
  int width = dat_info->data_width;
  if (width == 0)
    width = info_ptr->width;
  int rowbytes = width * 3 * byte_depth;

  int amt = 0, i = 0;
  int offset, data_len;
  while (amt < *data_size)
    {
      if (i == 0)
	{
	  offset = 4;
	}
      else
	offset = 0;

      data_len = rowbytes - offset;
      if (data_len + amt > *data_size)
	data_len = *data_size - amt;
      
      memcpy(*data + amt, row_pointers[i] + offset, data_len);
      amt += data_len;
      i++;
    }
  
  fclose(fp);
  
  return 0;
}
