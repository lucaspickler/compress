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

#include <buffer.h>
#include <dictionary.h>
#include <global.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util.h>

/* 
 * Parameters.
 */
#define RADIX 256 /* Radix of input data. */
#define WIDTH  12 /* Width of code word.  */

/*============================================================================*
 *                           Bit Buffer Reader/Writer                         *
 *============================================================================*/

/*
 * Writebits arguments.
 */
struct writebits_args
{
	FILE *out;   /* Output file.  */
	buffer_t in; /* Input buffer. */
};

/*
 * Writes data to a file.
 */
static void *lzw_writebits(void *args)
{
	int bits;    /* Working bits. */
	FILE *out;   /* Output file.  */
	buffer_t in; /* Input buffer. */
	unsigned n;  /* Current bit.  */
	int buf;     /* Buffer.       */
	
	in = ((struct writebits_args *)args)->in;
	out = ((struct writebits_args *)args)->out;
	
	n = 0;
	buf = 0;
	
	/*
	 * Read data from input buffer
	 * and write to output file.
	 */
	while ((bits = buffer_get(in)) != EOF)
	{	
		buf  = buf << WIDTH;
		buf |= bits & ((1 << WIDTH) - 1);
		n += WIDTH;
				
		/* Flush bytes. */
		while (n >= 8)
		{
			fputc((buf >> (n - 8)) & 0xff, out);
			n -= 8;
		}
	}
	
	if (n > 0) {
		fputc((buf << (8 - n)) & 0xff, out);
	}
	
	return (NULL);
}

/*============================================================================*
 *                           Bit Buffer Reader/Writer                         *
 *============================================================================*/

/*
 * readbits arguments.
 */
struct readbits_args
{
	FILE *in;     /* Input file.    */
	buffer_t out; /* Output buffer. */
};

/*
 * Reads data from a file.
 */
static void *lzw_readbits(void *args)
{
	int bits;     /* Working bits.  */
	FILE *in;     /* Input file.    */
	buffer_t out; /* Output buffer. */
	unsigned n;   /* Current bit.   */
	int buf;      /* Buffer.        */
	
	in = ((struct readbits_args *)args)->in;
	out = ((struct readbits_args *)args)->out;
	
	n = 0;
	buf = 0;
	
	/*
	 * Read data from input file
	 * and write to output buffer.
	 */
	while ((bits = fgetc(in)) != EOF)
	{	
		buf = buf << 8;
		buf |= bits & 0xff;
		n += 8;
				
		/* Flush bytes. */
		while (n >= WIDTH)
		{
			buffer_put(out, (buf >> (n - WIDTH)) & ((1 << WIDTH) - 1));
			n -= WIDTH;
		}
	}
			
	buffer_put(out, EOF);
	
	return (NULL);
}

/*============================================================================*
 *                                Readbyte                                    *
 *============================================================================*/

/*
 * Readbyte arguments.
 */
struct readbytes_args
{
	FILE *in;
	buffer_t out;
};

/*
 * Reads data from a file.
 */
static buffer_t lzw_readbytes(FILE *in)
{
	int ch;
	buffer_t inbuf;
	
	inbuf = buffer_open(1 << WIDTH);

	/* Read data from file to a buffer. */
	while ((ch = fgetc(in)) != EOF)
		buffer_put(inbuf, ch & 0xff);
	
	buffer_put(inbuf, EOF);

	return (inbuf);
}

/*============================================================================*
 *                                writebytes                                  *
 *============================================================================*/

/*
 * writebytes() arguments.
 */
struct writebytes_args
{
	FILE *out;
	buffer_t in;
};

/*
 * Writes data to a file.
 */
static void *lzw_writebytes(void *args)
{
	int ch;      /* Working character. */
	buffer_t in; /* Input buffer.      */
	FILE *out;   /* Output file.       */

	in = ((struct writebytes_args *)args)->in;
	out = ((struct writebytes_args *)args)->out;
	
	/*
	 * Read data from file and feed
	 * buffer.
	 */
	while ((ch = buffer_get(in)) != EOF) {
		fputc(ch, out);
	}

	return (NULL);
}

/*============================================================================*
 *                      lzw_compress()  and lzw_decompress()                  *
 *============================================================================*/

/*
 * lzw_compress() arguments.
 */
struct compress_args
{
	buffer_t in;
	buffer_t out;
};

/*
 * Initializes lzw dictionary.
 */
static code_t lzw_init(dictionary_t dict, int radix)
{
	int i;
	
	for (i = 0; i < radix; i++) {
		dictionary_add(dict, 0, i, i);
	}
	
	return (i);
}

/*
 * Compress data.
 */
static void *lzw_compress(void *args)
{	
	buffer_t in, out;  /* Input and output buffers.   */
	unsigned ch;       /* Working character.          */
	int i, ni;         /* Working entries.            */
	code_t code;       /* Current code.               */
	dictionary_t dict; /* Dictionary.                 */
	
	in = ((struct compress_args *)args)->in;
	out = ((struct compress_args *)args)->out;
	
	dict = dictionary_create(1 << WIDTH);
	
	i = 0;
	code = lzw_init(dict, RADIX);

	/* Compress data. */
	ch = buffer_get(in);
	while (ch != EOF)
	{	
		ni = dictionary_find(dict, i, (char)ch);
		
		/* Find longest prefix. */
		if (ni >= 0)
		{			
			ch = buffer_get(in);
			i = ni;
		
			/* Next character. */
			if (ch != EOF) {
				continue;
			}
		}
		
		buffer_put(out, dict->entries[i].code);
		
		if (code == ((1 << WIDTH) - 1))
		{	
			i = 0;
			dictionary_reset(dict);
			code = lzw_init(dict, RADIX);
			buffer_put(out, RADIX);
			continue;
		}
		
		dictionary_add(dict, i, ch, ++code);
		i = 0;
	}
	
	buffer_put(out, EOF);

	dictionary_destroy(dict);
	
	return (NULL);
}

/*
 * lzw_decompress() arguments.
 */
struct decompress_args
{
	buffer_t in;
	buffer_t out;
};

/*
 * 
 */
char *buildstr(char *base, char ch)
{
	char *s, *p;
	
	p = s = smalloc((strlen(base) + 2)*sizeof(char));
	
	while (*base != '\0') {
		*p++ = *base++;
	}
	
	*p++ = ch;
	*p = '\0';
	
	return (s);
}

/*
 * Decompress data.
 */
void *lzw_decompress(void *args)
{
	unsigned i;        /* Loop index.                 */
	char *s, *p;
	char **st;         /* String table.               */
	buffer_t in, out;  /* Input and output buffers.   */
	unsigned code;
	
	in = ((struct decompress_args *)args)->in;
	out = ((struct decompress_args *)args)->out;
	
	st = smalloc(((1 << WIDTH) + 2)*sizeof(char *));
	
	/* Initializes the symbol table. */
	for (i = 0; i < RADIX; i++) {
		st[i] = buildstr("", i);
	}
	
	st[i++] =  buildstr("", ' ');
	
	code = buffer_get(in);
	
	/* Broken file. */
	if (code >= i) {
		error("broken file");
	}
	
	s = st[code];
	
	/* Decompress data. */
	while (1)
	{
		/* Output current string. */
		for (p = s; *p != '\0'; p++) {
			buffer_put(out, (unsigned)(*p & 0xff));
		}
		
		code = buffer_get(in);
		
		/* End of input. */
		if (code == EOF) {
			break;
		}
		
		/* Reset symbol table. */
		if (code == RADIX)
		{			
			while (i > 0) {
				free(st[--i]);
			}
			
			/* Initializes the symbol table. */
			for (i = 0; i < RADIX; i++) {
				st[i] = buildstr("", i);
			}
			
			st[i++] =  buildstr("", ' ');
			
			code = buffer_get(in);
	
			/* Broken file. */
			if (code >= i) {
				error("broken file");
			}
			
			s = st[code];
			
			continue;
		}
		
		/* Broken file. */
		if (code > i) {
			error("broken file");
		}
		
		p = st[code];
		
		if (i == code) {
			p = buildstr(s, s[0]);
		}
		
		st[i++] = buildstr(s, p[0]);
		
		s = p;
	}
	
	buffer_put(out, EOF);
	
	/* House keeping. */
	while (i > 0) {
		free(st[--i]);
	}
	free(st);
	
	return (NULL);
}


/*============================================================================*
 *                                   LZW                                      *
 *============================================================================*/

/*
 * 
 */
void lzw(FILE *input, FILE *output, int compress)
{
	buffer_t inbuf;  /* Input buffer.  */
	buffer_t outbuf; /* Output buffer. */
	
	/* Compress mode. */
	if (compress)
	{
		inbuf = lzw_readbytes(input);
		outbuf = lzw_compress(inbuf);
		lzw_writebits(outbuf);
	}
	
	/* Decompress mode. */
	else
	{	
		inbuf = lzw_readbits(input);
		outbuf = lzw_decompress(inbuf);
		lzw_writebytes(outbuf);
	}
	
	buffer_destroy(outbuf);
	buffer_destroy(inbuf);
}
