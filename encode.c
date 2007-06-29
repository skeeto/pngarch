#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <stdint.h>
#include <math.h>
#include <netinet/in.h>

#include "config.h"

#include "datpng.h"

int header_size = 10;
short cur_ver = 0;

png_colorp gen_palette ();

int datpng_write (FILE * outfile, datpng_info * dat_info,
		  void *data, size_t data_size)
{
  png_colorp palette = NULL;
  int num_palette;

  int exit_stat = EXIT_SUCCESS;

  /* Check the bit depth. */
  if (dat_info->bit_depth != 8)
    return PNGDAT_INVALID_BIT_DEPTH;

  int pixel_width, color_type;
  switch (dat_info->color_type)
    {
    case PNGDAT_CT_AUTO:
      pixel_width = 3;
      color_type = PNG_COLOR_TYPE_RGB;
      break;

    case PNGDAT_CT_RGB:
      pixel_width = 3;
      color_type = PNG_COLOR_TYPE_RGB;
      break;

    case PNGDAT_CT_PALETTE:
      pixel_width = 1;
      color_type = PNG_COLOR_TYPE_PALETTE;
      break;

    default:
      return PNGDAT_INVALID_COLOR_TYPE;
      break;
    }

  int csum = dat_info->checksum;

  int x_pos = dat_info->x_pos;
  int y_pos = dat_info->y_pos;
  int data_width = dat_info->data_width;
  int data_height = dat_info->data_height;
  int img_width = dat_info->png_width;
  int img_height = dat_info->png_height;

  /* No constraints defined. */
  if (img_width == 0 && img_height == 0 &&
      data_width == 0 && data_height == 0)
    return PNGDAT_MISSING_CONSTRAINT;

  /* Propagate width value. */
  if (img_width == 0 && data_width > 0)
    img_width = data_width + x_pos;
  if (img_width > 0 && data_width == 0)
    data_width = img_width - x_pos;

  /* Propagate height value. */
  if (img_height == 0 && data_height > 0)
    img_height = data_height - y_pos;
  if (img_height > 0 && data_height == 0)
    data_height = img_height + y_pos;

  if (img_width == 0 && data_width == 0)
    {
      /* Calculate a width. */
      data_width = (int) ceil ((data_size + header_size)
			       / (data_height * pixel_width * 1.0));
      if (csum)
	data_width = (int) ceil ((data_size + header_size + data_height)
				 / (data_height * pixel_width * 1.0));
      img_width = data_width + x_pos;
    }
  if (img_height == 0 && data_height == 0)
    {
      /* Calculate a height. */
      data_height = (int) ceil ((data_size + header_size)
				/ (data_width * pixel_width * 1.0));
      if (csum)
	data_height = (int) ceil ((data_size + header_size + data_height)
				  / (data_width * pixel_width * 1.0));
      img_height = data_height + y_pos;
    }

  png_bytep *row_pointers;

  /* Set up the image data. */
  png_infop infoin_ptr;
  png_structp pngin_ptr;
  if (dat_info->insert)
    {
      pngin_ptr = png_create_read_struct
	(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      infoin_ptr = png_create_info_struct (pngin_ptr);
      png_init_io (pngin_ptr, outfile);
      png_read_png (pngin_ptr, infoin_ptr, PNG_TRANSFORM_STRIP_ALPHA, NULL);

      row_pointers = png_get_rows (pngin_ptr, infoin_ptr);

      rewind (outfile);

      /* Check dimensions. */
      img_width = infoin_ptr->width;
      img_height = infoin_ptr->height;
      if (data_width + x_pos > img_width)
	data_width = img_width - x_pos;
      if (data_height + y_pos > img_height)
	data_height = img_height - y_pos;

      /* Check color type. */
      switch (infoin_ptr->color_type)
	{
	case PNG_COLOR_TYPE_PALETTE:
	  pixel_width = 1;
	  color_type = PNG_COLOR_TYPE_PALETTE;
	  break;

	case PNG_COLOR_TYPE_RGB:
	  pixel_width = 3;
	  color_type = PNG_COLOR_TYPE_RGB;
	  break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	  pixel_width = 3;
	  color_type = PNG_COLOR_TYPE_RGB;
	  break;

	default:
	  /* XXX free resources */
	  return PNGDAT_INVALID_COLOR_TYPE;
	  break;
	}

      /* Check the bit depth. */
      if (infoin_ptr->bit_depth != 8)
	{
	  /* XXX free resources */
	  return PNGDAT_INVALID_BIT_DEPTH;
	}
    }
  else
    {
      row_pointers = (png_bytep *) malloc (sizeof (png_bytep) * img_height);
    }

  int rowbytes = img_width * pixel_width;
  int datbytes = data_width * pixel_width - csum;

  if (datbytes * data_height < data_size)
    {
      data_size = datbytes * data_height - header_size;
      exit_stat = PNGDAT_TRUNCATED;
    }

  /* Initialize png structs. */
  png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL,
						 NULL, NULL);
  png_infop info_ptr = png_create_info_struct (png_ptr);
  png_init_io (png_ptr, outfile);

  /* Generate the header. */
  uint8_t header[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  header[1] = (uint8_t) dat_info->bit_depth;
  if (csum)
    header[0] = 128;
  header[0] += cur_ver;

  /* Length of data in network order. */
  uint32_t size = (int) data_size;
  size = htonl (size);
  memcpy (header + 2, &size, 4);

  /* Width of data in network order. */
  uint32_t hdr_width = data_width;
  hdr_width = htonl (hdr_width);
  memcpy (header + 6, &hdr_width, 4);

  /* Load data into row_pointers image data. */
  int data_len, offset;
  void *next_data = data;
  int i;
  for (i = 0; i < img_height; i++)
    {
      if (!dat_info->insert)
	row_pointers[i] = (png_bytep) calloc (1, rowbytes);

      /* Write the header. */
      if (i == y_pos)
	{
	  memcpy (row_pointers[y_pos] +
		  (x_pos * pixel_width), &header, header_size);
	  offset = header_size;
	}
      else
	offset = 0;

      /* 8-bit depth only: */

      data_len = (i - y_pos + 1) * datbytes - header_size;
      if (data_len > data_size)
	data_len = data_size - (data_len - datbytes);
      else

	data_len = datbytes - offset;

      if (data_len < 0)
	data_len = 0;

      int write_out;
      write_out = 0;
      if (i >= y_pos && data_len > 0)
	{
	  memcpy (row_pointers[i] + offset + (x_pos * pixel_width),
		  next_data, data_len);
	  next_data += data_len;
	  write_out = 1;
	}

      /* Calculate the checksum. */
      if (csum && write_out)
	{
	  int j;
	  uint8_t sum = 0;
	  for (j = (x_pos * pixel_width);
	       j < datbytes + (x_pos * pixel_width); j++)
	    sum += row_pointers[i][j];

	  row_pointers[i][(x_pos * pixel_width) + datbytes] = sum;
	}
    }

  /* Determine a palette. */
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
      palette = gen_palette ();
      info_ptr->palette = palette;
      info_ptr->num_palette = num_palette;
      png_set_PLTE (png_ptr, info_ptr, palette, num_palette);
    }

  /* Set image meta data. */
  info_ptr->width = img_width;
  info_ptr->height = img_height;
  info_ptr->valid = 0;
  info_ptr->rowbytes = rowbytes;
  info_ptr->num_trans = 0;
  info_ptr->bit_depth = dat_info->bit_depth;
  info_ptr->color_type = color_type;
  info_ptr->compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  info_ptr->filter_type = PNG_FILTER_TYPE_DEFAULT;
  info_ptr->interlace_type = PNG_INTERLACE_NONE;

  png_write_info (png_ptr, info_ptr);

  if (setjmp (png_jmpbuf (png_ptr)))
    printf ("[write_png_file] Error during write\n");

  png_write_image (png_ptr, row_pointers);
  png_write_end (png_ptr, NULL);

  return exit_stat;
}

/* Generates a 256 color palette */
png_colorp gen_palette ()
{
  png_colorp palette = (png_colorp) malloc (256 * sizeof (png_color));

  int i;

  /* red */
  for (i = 0; i < 64; i++)
    palette[i].red = i * 4;
  for (i = 64; i < 192; i++)
    palette[i].red = 255;
  for (i = 192; i < 256; i++)
    palette[i].red = (255 - i) * 4;

  /* green */
  for (i = 0; i < 64; i++)
    palette[i].green = (64 - i) * 4;
  for (i = 64; i < 128; i++)
    palette[i].green = (i - 64) * 4;
  for (i = 128; i < 256; i++)
    palette[i].green = 255;

  /* blue */
  for (i = 0; i < 64; i++)
    palette[i].blue = 255;
  for (i = 64; i < 128; i++)
    palette[i].blue = (128 - i) * 4;
  for (i = 128; i < 192; i++)
    palette[i].blue = (i - 128) * 4;
  for (i = 192; i < 256; i++)
    palette[i].blue = 255;

  /* palette = calloc(1,256 * sizeof(png_color)); */
  return palette;
}
