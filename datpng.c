#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "datpng.h"

static int img_width = 1024;
static int verbose_flag;

int main(int argc, char **argv)
{
  int c;
  
  while (1)
    {
      static struct option long_options[] =
	{
	  /* These options set a flag. */
	  {"help",    no_argument,       0, 'h'},
	  {"verbose", no_argument,       0, 'v'},
	  {"extract", no_argument,       0, 'x'},
	  {"create",  no_argument,       0, 'c'},
	  {"raw",     no_argument,       0, 'r'},
	  {"simple",  no_argument,       0, 's'},
	  {"file",    required_argument, 0, 'f'},
	  {0, 0, 0, 0}
	};
      /* getopt_long stores the option index here. */
      int option_index = 0;
      
      c = getopt_long (argc, argv, "hvxcrsf:",
		       long_options, &option_index);
      
      /* Detect the end of the options. */
      if (c == -1)
	break;
      
      switch (c)
	{
             case 'h':
               break;
     
             case 'v':
               break;
     
             case 'x':
               break;
     
             case 'c':
               break;
     
             case 'r':
               break;
     
             case 's':
               break;
     
             case 'f':
               break;
     
             case '?':
               /* getopt_long already printed an error message. */
               break;
     
             default:
               abort ();
             }
         }
  
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
