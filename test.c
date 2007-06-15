#include <stdio.h>
#include <stdlib.h>
#include <png.h>


int width = 800;
int data_size;

int main()
{
  png_bytep *row_pointers;

  int rp_size = 64;
  data_size = 0;

  row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * rp_size);
  
  size_t amt;
  while (!feof(stdin))
    {
      row_pointers[data_size] = (png_bytep)malloc(width * 3);
      amt = fread(row_pointers[data_size], 1, width * 3, stdin);
      data_size++;
      if (data_size > rp_size)
	{
	  rp_size *= 2;
	  row_pointers = (png_bytep *)
	    realloc(row_pointers, sizeof(png_bytep) * rp_size);
	}
    }
  
  if (amt < width * 3)
    {
      int i;
      for (i = amt; i < width * 3; i++)
	{
	  row_pointers[data_size - 1][i+0] = 0;
	  row_pointers[data_size - 1][i+1] = 0;
	  row_pointers[data_size - 1][i+2] = 0;
	}
    }
  if (amt == 0)
    data_size--;
  
  FILE *fw = fopen("out.png", "wb");
  
  /* Initialize png structs. */
  png_structp pngw_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  png_infop infow_ptr = png_create_info_struct(pngw_ptr);
  png_init_io(pngw_ptr, fw);
  
  /* Set image meta data. */
  infow_ptr->width = width;
  infow_ptr->height = data_size;
  infow_ptr->valid = 0;
  infow_ptr->rowbytes = width * 3;
  infow_ptr->palette = NULL;
  infow_ptr->num_palette = 0;
  infow_ptr->num_trans = 0;
  infow_ptr->bit_depth = 8;
  infow_ptr->color_type = PNG_COLOR_TYPE_RGB;
  infow_ptr->compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  infow_ptr->filter_type = PNG_FILTER_TYPE_DEFAULT;
  infow_ptr->interlace_type = PNG_INTERLACE_NONE;
  
  png_write_info(pngw_ptr, infow_ptr);
  
  if (setjmp(png_jmpbuf(pngw_ptr)))
    printf("[write_png_file] Error during write\n");

  png_write_image(pngw_ptr, row_pointers);
  png_write_end(pngw_ptr, NULL);
  
  printf("Done.\n");
  fclose(fw);
  
  return EXIT_SUCCESS;
}
