#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int main()
{
  FILE *fp = fopen("sample_small.png", "rb");
  
  /* Initialize png structs */
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);  
  png_init_io(png_ptr, fp);
  png_infop info_ptr = png_create_info_struct(png_ptr);  

  /* Read in the png */
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  int png_h = (int)info_ptr->height;
  int png_w = (int)info_ptr->width;
  printf("Height: %d\n", png_h);
  printf("Depth : %d\n", (int)info_ptr->bit_depth);
  
  png_bytep *row_pointers;
  row_pointers = png_get_rows(png_ptr, info_ptr); 
  
  int i, j, k;
  for (i = 0; i < png_h; i++)
    {
      for (j = 0; j < png_w; j++)
	{
	  printf("[ ");
	  for (k = 0; k < 3; k++)
	    {
	      printf("%d ", row_pointers[i][(j * 3)+k]);
	    }
	  printf(" ] ");
	}
      printf("\n");
    }
  
  fclose(fp);
  
  /*  png_destroy_read_struct(&png_ptr); */
  
  FILE *fw = fopen("out.png", "wb");
  
  png_structp pngw_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  png_init_io(pngw_ptr, fw);
  png_infop infow_ptr = png_create_info_struct(pngw_ptr);  
  
  if (setjmp(png_jmpbuf(pngw_ptr)))
    {
      /* If we get here, we had a problem reading the file */
      printf("ERROR\n");
    }
  
  /* First pixel red */
  row_pointers[0][0] = 255;
  row_pointers[0][1] = 0;
  row_pointers[0][2] = 0;
  
  infow_ptr->height = info_ptr->height;
  infow_ptr->width = info_ptr->width;
  //infow_ptr = info_ptr;

  //png_write_image(pngw_ptr, row_pointers);
  png_write_png(pngw_ptr, infow_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);
  png_write_end(pngw_ptr, infow_ptr);
  
  fclose(fw);
  
  return EXIT_SUCCESS;
}
