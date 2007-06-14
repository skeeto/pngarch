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
  
  return EXIT_SUCCESS;
}
