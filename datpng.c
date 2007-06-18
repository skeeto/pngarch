#include <stdio.h>
#include <stdlib.h>
#include "datpng.h"

int img_width = 1024;

int main()
{
  encode_dat("out.png");
  
  decode_dat("out.png");
  
  fprintf(stderr, "Done.\n");
  
  exit(EXIT_SUCCESS);
}

int decode_dat(char *filename)
{
  datpng_info data_info;

  data_info.bit_depth = 8;
  data_info.checksum = 0;
  data_info.x_pos = 0;
  data_info.y_pos = 0;
  data_info.data_width = img_width;
  data_info.data_height = 0;
  data_info.png_width = img_width;
  data_info.png_height = 0;

  size_t buffer_size = 0;
  void *buffer;
  FILE *fp = fopen(filename, "rb");
  datpng_read(fp, &data_info, &buffer, &buffer_size);
  fclose(fp);
  
  printf(buffer);
  
  return 0;
}

int encode_dat(char *filename)
{
  /* Prepare input buffer */
  size_t read_size = 1024;
  size_t buffer_max = 1024;
  size_t buffer_size = 0;
  void *buffer = malloc(buffer_max);
  
  size_t amt;
  while (!feof(stdin))
    {
      amt = fread(buffer + buffer_size, 1, read_size, stdin);
      
      buffer_size += amt;
      
      if (buffer_size >= buffer_max - read_size)
	{
	  buffer_max *= 2;
	  buffer = realloc(buffer, buffer_max);
	}
    }
  
  datpng_info data_info;
  
  data_info.bit_depth = 8;
  data_info.checksum = 0;
  data_info.x_pos = 0;
  data_info.y_pos = 0;
  data_info.data_width = img_width;
  data_info.data_height = 0;
  data_info.png_width = img_width;
  data_info.png_height = 0;
  
  FILE *fp = fopen(filename, "wb");
  datpng_write(fp, &data_info, buffer, buffer_size);
  fclose(fp);
  
  return 0;
}
