#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "datpng.h"

static int img_width = 1024;
static int verbose_flag = 0;
char *progname;

int print_usage(int exit_status)
{
  printf("Usage: %s [options] [files]\n\n", progname);
  printf("Options:\n\n");
  printf("  -c, --create     Create archive\n");
  printf("  -x, --extract    Extract archive contents\n");
  printf("  -f, --file       Use archive file"
	 " (default \"-\" for stdin/srdout)\n");
  printf("  -V, --verbose    Enable verbose output\n");
  printf("  -v, --version    Display version information\n");
  printf("  -h, --help       Display this help text\n");

  exit(exit_status);
}

int main(int argc, char **argv)
{
  progname = argv[0];
  
  int create_mode = 0; /* Create archive. */
  int extract_mode = 0; /* Extract archive. */
  char *filename = "-"; /* Archive file. */

  int c; /* Current option. */
  
  while (1)
    {
      static struct option long_options[] =
	{
	  /* These options set a flag. */
	  {"help",    no_argument,       0, 'h'},
	  {"version", no_argument,       0, 'v'},
	  {"verbose", no_argument,       0, 'V'},
	  {"extract", no_argument,       0, 'x'},
	  {"create",  no_argument,       0, 'c'},
	  {"file",    required_argument, 0, 'f'},
	  {"list",    no_argument,       0, 't'},
	  {0, 0, 0, 0}
	};
      /* getopt_long stores the option index here. */
      int option_index = 0;
      
      c = getopt_long (argc, argv, "hvVxcf:t",
		       long_options, &option_index);
      
      /* Detect the end of the options. */
      if (c == -1)
	break;
      
      switch (c)
	{
             case 'h':
	       print_usage(EXIT_SUCCESS);
               break;

             case 'v':
               break;
     
             case 'V':
	       verbose_flag = 1;
               break;
     
             case 'x':
	       if (!create_mode)
		 extract_mode = 1;
	       else
		 {	
		   fprintf(stderr, 
			  "%s: cannot specify more than one -xc option\n", 
			  progname);
		   print_usage(EXIT_FAILURE);
		 }
               break;
     
             case 'c':
	       if (!extract_mode)
		 create_mode = 1;
	       else
		 {	
		   fprintf(stderr, 
			   "%s: cannot specify more than one -xc option\n", 
			  progname);
		   print_usage(EXIT_FAILURE);
		 }
               break;
     
             case 'f':
	       filename = optarg;
               break;
     
             case 't':
               break;
     
             case '?':
               print_usage(EXIT_FAILURE);
               break;
     
             default:
               abort ();
             }
         }

  if (extract_mode == 0 && create_mode == 0)
    {
      fprintf(stderr, "%s: Must specify one of the -xc options\n", progname);
      print_usage(EXIT_FAILURE);
    }
  
  /* Extract from the archive. */
  if (extract_mode == 1)
    {
      decode_dat(filename);
    }

  /* Create new archive. */
  if (create_mode == 1)
    {
      encode_dat(filename);
    }
  
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
