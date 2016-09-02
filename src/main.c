/*
 * Copyright(C) 2014-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of LZW.
 * 
 * LZW is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * LZW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with LZW. If not, see <http://www.gnu.org/licenses/>.
 */

#include <global.h>
#include <stdlib.h>
#include <stdio.h>
#include <util.h>

/* Command line arguments. */
static int compress = 1; /* Compress?         */
char *infile = NULL;     /* Input file name.  */
char *outfile = NULL;    /* Output file name. */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("\nUsage: tar [options] <input file> <output file>\n\n");
	printf("Brief: archives files.\n\n");
	printf("Options:\n");
	printf("  -c, --create  create a new archive\n");
	printf("  -x, --extract extract all files from archive\n");
	printf("      --lzw     use Lempel-Ziv-Welch algorithm\n");
	
	exit(EXIT_SUCCESS);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		
		/* Parse option. */
		if (arg[0] == '-') {
			switch (arg[1])
			{
				/* Compress. */
				case 'c':
					compress = 1;
					break;
				
				/* Decompress. */
				case 'x':
					compress = 0;
					break;
			}
		}
		
		/* Get input file name. */
		else if (infile == NULL) {
			infile = arg;
		}
		
		/* Get output file name. */
		else if (outfile == NULL) {
			outfile = arg;
		}	
	}
	
	/* Missing input file. */
	if (infile == NULL) {
		warning("missing input file");
		usage();
	}
	
	/* Missing output file. */
	if (outfile == NULL) {
		warning("missing output file");
	}
}

/*
 * tar - File Archiver
 */
int main(int argc, char **argv)
{
	FILE *input;  /* Input file.  */
	FILE *output; /* Output file. */
	
	readargs(argc, argv);
	
	/* Open input file. */
	input = fopen(infile, "r");
	if (input == NULL) {
		error("cannot open input file");
	}
	
	/* Open output file. */
	output = fopen(outfile, "w");
	if (output == NULL) {
		error("cannot open output file");
	}

	lzw(input, output, compress);

	/* House keeping. */
	fclose(input);
	fclose(output);
	
	return (EXIT_SUCCESS);
}
