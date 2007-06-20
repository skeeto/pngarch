#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <stdint.h>
#include <netinet/in.h>

#include "config.h"

#include "datpng.h"

int header_size = 10;
short cur_ver = 0;

int datpng_write(FILE *outfile, datpng_info *dat_info, 
		 void *data, size_t data_size)
{
  int byte_depth = 1;
  int rowbytes = dat_info->data_width * 3 * byte_depth; 
  int csum = dat_info->checksum;
  
  int width = dat_info->png_width;
  
  /* XXX special case needed */
  int rp_height = (data_size + header_size) / rowbytes + 1;
  if (csum)
    rp_height += (rp_height / rowbytes) + 1;
  
  /* If png height is defined, data will take that height no matter
     what. Extra data is truncated. */
  if (dat_info->png_height > 0 &&
      rp_height > dat_info->png_height)
    rp_height = dat_info->png_height;
  
  /* Image data */
  png_bytep *row_pointers; 
  row_pointers = (png_bytep *)
    malloc(sizeof(png_bytep) * rp_height);
  
  /* Initialize png structs. */
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, outfile);
  
  /* Generate the header. */
  uint8_t header[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  header[1] = (uint8_t)dat_info->bit_depth;
  if (csum)
    header[0] = 128;
  header[0] += cur_ver;
  
  /* Length of data in network order. */
  uint32_t size = (int)data_size;
  size = htonl(size);
  memcpy(header + 2, &size, 4);

  /* Width of data in network order. */
  uint32_t hdr_width = width;
  hdr_width = htonl(hdr_width);
  memcpy(header + 6, &hdr_width, 4);
  
  /* Load data into row_pointers image data. */
  int data_len, offset;
  int i;
  int datbytes = rowbytes - csum;
  void *next_data = data;
  for (i = 0; i < rp_height; i++)
    {
      row_pointers[i] = (png_bytep) calloc(1, rowbytes);
      
      if (i == 0)
	{
	  /* Write the header. */
	  memcpy(row_pointers[0], &header, header_size);
	  offset = header_size;
	}
      else
	offset = 0;
      
      /* 8-bit depth only: */ 

      data_len = (i + 1) * datbytes - header_size;
      if (data_len > data_size)
	data_len = data_size - (data_len - datbytes);
      else
	data_len = datbytes - offset;
      
      if (data_len < 0)
	data_len = 0;
      
      memcpy(row_pointers[i] + offset, next_data, data_len);
      next_data += data_len;
      
      /* Calculate the checksum. */
      if (csum)
	{
	  int j;
	  uint8_t sum = 0;
	  for (j = 0; j < datbytes; j++)
	    sum += row_pointers[i][j];

	  row_pointers[i][rowbytes - 1] = sum;
	}
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
  
  return EXIT_SUCCESS;
}
