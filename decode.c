#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <stdint.h>
#include <netinet/in.h>

#include "datpng.h"

int datpng_read (FILE * infile, datpng_info * dat_info,
		 void **data, size_t * data_size)
{
  int x_pos = dat_info->x_pos;
  int y_pos = dat_info->y_pos;

  /* Prepare png data structures. */
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct (png_ptr);
  png_init_io (png_ptr, infile);
  png_read_png (png_ptr, info_ptr, PNG_TRANSFORM_STRIP_ALPHA, NULL);

  /* Grab the image data. */
  png_bytep *row_pointers;
  row_pointers = png_get_rows (png_ptr, info_ptr);

  /* Grab the header. */
  uint8_t header[10];
  memcpy (header, row_pointers[y_pos] + (x_pos * 3), header_size);

  /* Extract version information. */
  /*int bit_depth = (int)(header[1] & 0x0F); */
  int csum = (int) ((header[0] & 0x80) == 0x80);
  int ver = (int) (header[0] & 0x7F);
  if (ver > cur_ver)
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      if (!dat_info->no_warnings)
	fprintf (stderr, "Warning: datpng version too high\n");
      return PNGDAT_BAD_VERSION;
    }


  /* Get data length (from network order bytes) */
  *data_size = (size_t) ntohl (*((uint32_t *) (header + 2)));
  if (*data_size == 0 || *data_size > png_ptr->width * png_ptr->height * 3)
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      return PNGDAT_CORRUPT_DATA;
    }

  /* Get data length (from network order bytes) */
  int width = (int) ntohl (*((uint32_t *) (header + 6)));
  if (width > png_ptr->width + dat_info->x_pos)
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
      return PNGDAT_CORRUPT_DATA;
    }

  *data = calloc (1, *data_size);

  int datbytes = width * 3 - csum;

  int amt = 0, i = 0;
  int offset, data_len;
  while (amt < *data_size)
    {
      if (i == y_pos)
	offset = header_size;
      else
	offset = 0;

      data_len = datbytes - offset;
      if (data_len + amt > *data_size)
	data_len = *data_size - amt;

      int read_in;
      read_in = 0;
      if (i >= y_pos)
	{
	  memcpy (*data + amt, row_pointers[i] + offset + (x_pos * 3),
		  data_len);
	  amt += data_len;
	  read_in = 1;
	}

      /* Check data integrity. */
      if (csum && read_in)
	{
	  int j;
	  uint8_t sum = 0;
	  for (j = (x_pos * 3); j < datbytes + (x_pos * 3); j++)
	    sum += row_pointers[i][j];

	  if (row_pointers[i][datbytes + (x_pos * 3)] != sum)
	    {
	      if (!dat_info->no_warnings)
		fprintf (stderr, "Warning: checksum failed!\n");
	      if (dat_info->checksum)
		{
		  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		  free (*data);
		  return PNGDAT_FAILED_CHECKSUM;
		}
	    }
	}

      i++;
    }

  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
  return 0;
}

int datpng_autoread (FILE * infile, datpng_info * dat_info,
		     void **data, size_t * data_size)
{
  /* Get the PNG size. */
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct (png_ptr);
  png_init_io (png_ptr, infile);
  png_read_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  int width = png_ptr->width;
  int height = png_ptr->height;

  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

  int i, j, ret;
  for (i = 0; i < width - 1; i++)
    {
      dat_info->x_pos = i;
      for (j = 0; j < height - 1; j++)
	{
	  rewind (infile);
	  dat_info->y_pos = j;
	  ret = datpng_read (infile, dat_info, data, data_size);
	  if (ret <= 0)
	    return PNGDAT_SUCCESS;
	}
    }

  return PNGDAT_FAILED_AUTOFIND;
}
