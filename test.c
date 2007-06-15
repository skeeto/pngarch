#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int width = 200;
int data_size;

int main()
{
  
  FILE *fw = fopen("out.png", "wb");
  
  /* Initialize png structs. */
  png_structp pngw_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  png_infop infow_ptr = png_create_info_struct(pngw_ptr);
  png_init_io(pngw_ptr, fw);
  
  /* Set image meta data. */
  infow_ptr->width = width;
  infow_ptr->height = 1;
  infow_ptr->valid = 0;
  infow_ptr->rowbytes = 3;
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

  png_bytep row_pointers[3];
  row_pointers[0][0] = 0;
  row_pointers[0][1] = 0;
  row_pointers[0][2] = 0;

  png_write_image(pngw_ptr, row_pointers);
  png_write_end(pngw_ptr, NULL);
  
  printf("Done.\n");
  fclose(fw);
  
  return EXIT_SUCCESS;
}
