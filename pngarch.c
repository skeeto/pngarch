#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "datpng.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
char *version = PACKAGE_VERSION;
#else
char *version = "";
#endif

/* Settings */
static int data_width = 0;
static int data_height = 0;
static int img_width = 0;
static int img_height = 0;
static int x_pos = 0;
static int y_pos = 0;
static int bit_depth = 8;

/* Option flags */
static int verbose_flag = 0;
static int brief_flag = 0;
static int list_flag = 0;
static int square_flag = 1;
static int insert_flag = 0;
static int auto_detect = 1;
static int checksum = 1;

char *progname;			/* Program name. */

int decode_dat (char *infile, char *outfile);
int encode_dat (char *infile, char *outfile);
int auto_dat (char *filename, char *outfile);

void print_version ()
{
  printf ("pngarch, version %s\n", version);
  printf ("Copyright (C) 2007 Chris Wellons\n");
  printf ("This is free software; see the source code for "
	  "copying conditions.\n");
  printf ("There is ABSOLUTELY NO WARRANTY; not even for "
	  "MERCHANTIBILITY or\n");
  printf ("FITNESS FOR A PARTICULAR PURPOSE.\n");
}

int print_usage (int exit_status)
{
  printf ("Usage: %s [OPTION ...] [FILE ...]\n\n", progname);
  printf ("Options:\n\n");
  printf ("  -c, --create            Create PNG Archive\n");
  printf ("  -x, --extract           Extract PNG Archive\n");
  printf ("  -v, --verbose           Enable verbose output\n");
  printf ("  -b, --brief             Enable verbose output\n");
  printf
    ("  -o, --output            Set the output file name (invokes -i)\n");
  printf ("  -X, --x-position        Distance of data from image left\n");
  printf ("  -Y, --y-position        Distance of data from image top\n");
  printf ("  -V, --version           Display version information\n");
  printf ("  -!, --help              Display this help text\n");

  /* Archive options. */
  printf ("\nArchiving Options:\n\n");
  printf ("  -n, --no-checksum       Do not add checksum to data\n");
  printf ("  -W, --data-width        Width of data in the image\n");
  printf ("  -H, --data-height       Height of data in the image\n");
  printf ("  -w, --img-width         Width of the image\n");
  printf ("  -h, --img-height        Height of the image\n");
  printf ("  -d, --bit-depth         PNG bit depth (default %d)\n",
	  bit_depth);
  printf ("  -i, --insert            Insert data into exiting images\n");

  /* Extraction options. */
  printf ("\nExtraction Options:\n\n");
  printf ("  -t, --list              List internal filename\n");
  printf ("  --auto-detect           Attempt to find image data "
	  "automatically. (default)\n");
  printf ("  --no-auto-detect        Do not attempt to find image data "
	  "automatically.\n");
  printf ("  -n, --no-checksum       Ignore bad checksums\n");

  /* Examples */
  printf ("\n");
  printf ("Examples:\n\n");
  printf ("This example creates a PNG image named `some_file.txt.png'\n\n");
  printf ("  %s some_file.txt\n", progname);
  printf ("\nAnd this example inserts the file `file.o' at location (10, 15) "
	  "and data width 50 in the existing\nimage `my_img.png'\n\n");
  printf ("  %s -X 10 -Y 15 -w 50 -o my_img.png file.o\n", progname);

  exit (exit_status);
}

int main (int argc, char **argv)
{
  progname = argv[0];

  int auto_mode = 1;		/* Auto detect mode. */
  int create_mode = 0;		/* Create archive. */
  int extract_mode = 0;		/* Extract archive. */
  int exit_stat = EXIT_SUCCESS;	/* Program exit status */
  char *outfile = NULL;		/* User selected output file. */
  int c;			/* Current option. */

  while (1)
    {
      static struct option long_options[] = {
	/* *INDENT-OFF* */
	/* These options set a flag. */
	{"help",        no_argument,       0, '!'},
	{"version",     no_argument,       0, 'V'},
	{"verbose",     no_argument,       0, 'v'},
	{"brief",       no_argument,       0, 'b'},
	{"output",      required_argument, 0, 'o'},
	{"extract",     no_argument,       0, 'x'},
	{"create",      no_argument,       0, 'c'},
	{"data-width",  required_argument, 0, 'W'},
	{"data-height", required_argument, 0, 'H'},
	{"img-width",   required_argument, 0, 'w'},
	{"img-height",  required_argument, 0, 'h'},
	{"list",        no_argument,       0, 't'},
	{"no-checksum", no_argument,       0, 'n'},
	{"x-position",  required_argument, 0, 'X'},
	{"y-position",  required_argument, 0, 'Y'},
	{"bit-depth",   required_argument, 0, 'd'},
	{"insert",      no_argument,       0, 'i'},

	/* Auto-detect options. */
	{"--no-auto-detect", no_argument, &auto_detect, 0},
	{"--auto-detect",    no_argument, &auto_detect, 1},
	{0, 0, 0, 0}
	/* *INDENT-ON* */
      };

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "vbVxcw:h:tno:!iX:Y:H:W:",
		       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
	case '!':		/* help */
	  print_usage (EXIT_SUCCESS);
	  break;

	case 'V':		/* version */
	  print_version ();
	  exit (EXIT_SUCCESS);
	  break;

	case 'v':		/* verbose */
	  verbose_flag = 1;
	  break;

	case 'b':		/* brief */
	  brief_flag = 1;
	  break;

	case 'o':		/* output filename */
	  outfile = optarg;
	  insert_flag = 1;
	  break;

	case 'i':		/* insert */
	  insert_flag = 1;
	  break;

	case 'x':		/* extract */
	  if (!create_mode)
	    {
	      extract_mode = 1;
	      auto_mode = 0;
	    }
	  else
	    {
	      fprintf (stderr,
		       "%s: cannot specify more than one -xc option\n",
		       progname);
	      print_usage (EXIT_FAILURE);
	    }
	  break;

	case 'c':		/* create */
	  if (!extract_mode)
	    {
	      create_mode = 1;
	      auto_mode = 0;
	    }
	  else
	    {
	      fprintf (stderr,
		       "%s: cannot specify more than one -xc option\n",
		       progname);
	      print_usage (EXIT_FAILURE);
	    }
	  break;

	case 'W':		/* data width */
	  data_width = atoi (optarg);
	  square_flag = 0;
	  break;

	case 'H':		/* data height */
	  data_height = atoi (optarg);
	  square_flag = 0;
	  break;

	case 'w':		/* image width */
	  img_width = atoi (optarg);
	  square_flag = 0;
	  break;

	case 'h':		/* image height */
	  img_height = atoi (optarg);
	  square_flag = 0;
	  break;

	case 'X':		/* data x position */
	  x_pos = atoi (optarg);
	  square_flag = 0;
	  auto_detect = 0;
	  break;

	case 'Y':		/* data y position */
	  y_pos = atoi (optarg);
	  auto_detect = 0;
	  square_flag = 0;
	  break;

	case 't':		/* list filename */
	  list_flag = 1;
	  break;

	case 'n':		/* no checksum */
	  checksum = 0;
	  break;

	case 'd':		/* image bit depth */
	  bit_depth = atoi (optarg);
	  break;

	case '?':		/* error */
	  print_usage (EXIT_FAILURE);
	  break;

	default:
	  fprintf (stderr, "You should not see this. File a bug report!\n");
	  abort ();
	}
    }

  if ((outfile != NULL) && (argc - optind > 1))
    {
      fprintf (stderr, "%s: cannot specify more than one input file with"
	       " -o option\n", progname);
      exit (EXIT_FAILURE);
    }

  int ret = 0;			/* Return value. */

  /* Extract from the archive. */
  if (extract_mode == 1)
    {
      int i;
      for (i = optind; i < argc; i++)
	ret = ret || decode_dat (argv[i], outfile);

      if (argc == optind)
	ret = decode_dat ("-", outfile);
    }

  /* Create new archive. */
  if (create_mode == 1)
    {
      int i;
      for (i = optind; i < argc; i++)
	ret = ret || encode_dat (argv[i], outfile);

      if (argc == optind)
	ret = encode_dat ("-", outfile);
    }

  /* Automode - method based on filename extension. */
  if (auto_mode)
    {
      if (argc == optind)
	ret = encode_dat ("-", outfile);
      else
	{
	  int i;
	  for (i = optind; i < argc; i++)
	    {
	      ret = ret || auto_dat (argv[i], outfile);
	    }
	}
    }

  if (ret != 0)
    exit_stat = EXIT_FAILURE;

  exit (exit_stat);
}

int decode_dat (char *filename, char *outfile)
{
  /* Set the necessary meta data. */
  datpng_info data_info;
  data_info.bit_depth = bit_depth;
  data_info.checksum = 0;
  if (!auto_detect)
    {
      data_info.x_pos = x_pos;
      data_info.y_pos = y_pos;
      data_info.data_width = data_width;
      data_info.data_height = data_height;
      data_info.png_width = img_width;
      data_info.png_height = img_height;
      data_info.checksum = checksum;
      data_info.no_warnings = brief_flag;
    }
  else
    {
      data_info.x_pos = 0;
      data_info.y_pos = 0;
      data_info.data_width = 0;
      data_info.data_height = 0;
      data_info.png_width = 0;
      data_info.png_height = 0;
      data_info.checksum = 1;
      data_info.no_warnings = 1;
    }

  FILE *fp;
  if (strcmp (filename, "-") == 0)
    fp = stdin;
  else
    {
      fp = fopen (filename, "rb");
      if (fp == NULL)
	{
	  fprintf (stderr, "%s: Failed to open file %s - %s\n",
		   progname, filename, strerror (errno));
	  return 1;
	}
    }

  /* Get data from the PNG file. */
  size_t buffer_size = 0;
  void *buffer;
  int ret;
  if (auto_detect)
    {
      ret = datpng_autoread (fp, &data_info, &buffer, &buffer_size);
      if (ret > 0)
	{
	  fprintf (stderr, "%s: failed to find data in PNG\n", progname);
	  return 1;
	}
    }
  else
    {
      ret = datpng_read (fp, &data_info, &buffer, &buffer_size);
      if (ret > 0)
	{
	  fprintf (stderr, "%s: data in PNG corrupt\n", progname);
	  return 1;
	}
    }

  if (strcmp (filename, "-") != 0)
    fclose (fp);

  /* Unload the data into the file named by the internal filename. */
  if (outfile == NULL)
    outfile = strdup (buffer);
  if (outfile == NULL)
    {
      fprintf (stderr, "%s: failed strdup - %s\n", progname,
	       strerror (errno));
      exit (EXIT_FAILURE);
    }

  if (list_flag)
    {
      if (strlen (outfile) == 0)
	printf ("(stdout)\n");
      else
	printf ("%s\n", outfile);
      return 0;
    }

  FILE *fw;
  if (strlen (outfile) == 0)
    fw = stdout;
  else
    {
      fw = fopen (outfile, "wb");
      if (fw == NULL)
	{
	  fprintf (stderr, "%s: Failed to open file %s - %s\n",
		   progname, outfile, strerror (errno));
	  return 1;
	}
    }

  int boff = strlen (outfile) + 1;
  fwrite (buffer + boff, buffer_size - boff, 1, fw);

  if (strlen (outfile) != 0)
    fclose (fw);

  if (verbose_flag && strlen (outfile) != 0)
    fprintf (stderr, "Extracted file %s\n", outfile);

  return 0;
}

int encode_dat (char *infile, char *outfile)
{
  if (verbose_flag)
    fprintf (stderr, "Encoding %s\n", infile);

  int file_exist = 0;
  if (insert_flag)
    {
      FILE *fc = fopen (infile, "rb");
      if (fc != NULL)
	{
	  file_exist = 1;
	  fclose (fc);
	}
    }

  /* Prepare input buffer */
  size_t read_size = 1024;	/* Reading in 1 kbyte at a time. */
  size_t buffer_max = 1024 * 64;	/* Initial buffer at 64 kbytes. */
  size_t buffer_size = 0;
  void *buffer = malloc (buffer_max);

  FILE *fin;			/* input file pointer */
  if (strcmp (infile, "-") == 0)
    {
      if (outfile == NULL)
	outfile = "-";
      buffer_size += 1;
      strncpy (buffer, "", buffer_max);
      fin = stdin;
    }
  else
    {
      buffer_size += (strlen (infile) + 1);
      strncpy (buffer, infile, buffer_max);

      fin = fopen (infile, "rb");
      if (fin == NULL)
	{
	  fprintf (stderr, "%s: Failed to open file %s - %s\n",
		   progname, infile, strerror (errno));
	  return 1;
	}

      if (outfile == NULL)
	{
	  outfile = (char *) malloc (strlen (infile) + 5);
	  if (outfile == NULL)
	    {
	      fprintf (stderr, "%s: Failed to malloc outfilename - %s\n",
		       progname, strerror (errno));
	      exit (EXIT_FAILURE);
	    }
	  snprintf (outfile, strlen (infile) + 5, "%s.png", infile);
	}
    }

  /* Read file into the buffer. */
  if (verbose_flag)
    fprintf (stderr, "Loading file %s\n", infile);
  while (!feof (fin))
    {
      /* Double buffer size when too small. */
      if (buffer_size >= buffer_max - read_size)
	{
	  buffer_max *= 2;
	  buffer = realloc (buffer, buffer_max);
	}

      size_t amt;
      amt = fread (buffer + buffer_size, 1, read_size, fin);
      buffer_size += amt;
    }

  if (strcmp (infile, "-") != 0)
    fclose (fin);

  /* Make image square. */
  if (square_flag)
    data_width = ceil (sqrt (buffer_size / 3.0));

  /* Setup meta data. */
  datpng_info data_info;
  data_info.bit_depth = bit_depth;
  data_info.checksum = checksum;
  data_info.x_pos = x_pos;
  data_info.y_pos = y_pos;
  data_info.data_width = data_width;
  data_info.data_height = data_height;
  data_info.png_width = img_width;
  data_info.png_height = img_height;
  data_info.no_warnings = brief_flag;
  data_info.insert = file_exist;

  FILE *fp;
  if (strcmp (outfile, "-") == 0)
    fp = stdout;
  else
    {
      char *o_mode = "wb";
      if (file_exist)
	o_mode = "r+b";
      fp = fopen (outfile, o_mode);
      if (fp == NULL)
	{
	  fprintf (stderr, "%s: Failed to open file %s - %s",
		   progname, infile, strerror (errno));
	  return 1;
	}
    }

  /* Finally we get to write the data out to a PNG. */
  int ret;
  ret = datpng_write (fp, &data_info, buffer, buffer_size);
  if (ret == PNGDAT_TRUNCATED && !brief_flag)
    fprintf (stderr, "Warning: data has been truncated.\n");

  if (strcmp (outfile, "-") != 0)
    fclose (fp);

  free (buffer);

  return 0;
}

/* Autodetect what to do by filename extension. */
int auto_dat (char *filename, char *outfile)
{
  if (strstr (filename, ".png") == filename + strlen (filename) - 4 ||
      strstr (filename, ".PNG") == filename + strlen (filename) - 4)
    return decode_dat (filename, outfile);
  else
    return encode_dat (filename, outfile);
}
