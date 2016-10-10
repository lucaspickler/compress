/*
 * Copyright(C) 2014-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of compress.
 * 
 * compress is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * compress is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with compress. If not, see <http://www.gnu.org/licenses/>.
 */

#include <buffer.h>
#include <dictionary.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util.h>
#include <pthread.h>//<lpk> biblioteca de POSIX Threads.</lpk>

/* 
 * Parameters.
 */
#define RADIX 256 /* Radix of input data. */
#define WIDTH  12 /* Width of code word.  */

/*============================================================================*
 *                           Bit Buffer Reader/Writer                         *
 *============================================================================*/

//<lpk> strutura de passagem de dados a ser passada a pthread.</lpk>
struct stream{
  struct buffer *buffer;
  FILE *file;
};
//</lpk>

/*
 * Writes data to a file.
 */

/*<lpk> RETIRADO: pthreads só recebem um argumento void*.
static void lzw_writebits(buffer_t in, FILE *out)
{
</lpk>*/

//<lpk> Modificado para: </lpk>
void* lzw_writebits(void* output)
{
  printf("Writebits thread created. pid: %d.\n",(int)pthread_self());//<lpk> debug </lpk>
  struct stream *_output=output;//<lpk> cast para stream </lpk>
  buffer_t in=_output->buffer;//<lpk> abre struct </lpk>
  FILE *out=_output->file;//<lpk> abre struct </lpk>
  //<lpk>código original:</lpk>
	int bits;   /* Working bits. */
	unsigned n; /* Current bit.  */
	int buf;    /* Buffer.       */
	
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
	
	if (n > 0)
		fputc((buf << (8 - n)) & 0xff, out);
	return NULL;
}

/*============================================================================*
 *                           Bit Buffer Reader/Writer                         *
 *============================================================================*/

/*
 * Reads data from a file.
 */
//<lpk> mesmo padrão de lzw_writebits. Ver lzw_writebits. </lpk>
//<lpk> modificada para receber e abrir stream </lpk>
void* lzw_readbits(void* input)
{
  printf("Readbits thread created. pid: %d.\n",(int)pthread_self());
  struct stream *_input=input;
  FILE *in=_input->file;
  buffer_t out=_input->buffer;
	int bits;   /* Working bits. */
	unsigned n; /* Current bit.  */
	int buf;    /* Buffer.       */
	
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
	return NULL;
}

/*============================================================================*
 *                                Readbyte                                    *
 *============================================================================*/

/*
 * Reads data from a file.
 */
//<lpk> mesmo padrão de lzw_writebits. Ver lzw_writebits. </lpk>
//<lpk> modificada para receber e abrir stream </lpk>
void* lzw_readbytes(void* input)
{
  printf("Readbytes thread created. pid: %d.\n",(int)pthread_self());
  struct stream *_input=input;
  FILE *infile = _input->file;
  buffer_t outbuf = _input->buffer;
	int ch;

	/* Read data from file to the buffer. */
	while ((ch = fgetc(infile)) != EOF)
		buffer_put(outbuf, ch & 0xff);
	
	buffer_put(outbuf, EOF);
	return NULL;
}

/*============================================================================*
 *                                writebytes                                  *
 *============================================================================*/

/*
 * Writes data to a file.
 */
//<lpk> mesmo padrão de lzw_writebits. Ver lzw_writebits. </lpk>
//<lpk> modificada para receber e abrir stream </lpk>
void* lzw_writebytes(void* output)
{
  printf("Writebytes thread created. pid: %d.\n",(int)pthread_self());
  struct stream *_output=output;
  buffer_t inbuf=_output->buffer;
    FILE *outfile=_output->file;  
	int ch;
	
	/* Read data from file to the buffer. */
	while ((ch = buffer_get(inbuf)) != EOF)
		fputc(ch, outfile);
	return NULL;
}

/*============================================================================*
 *                                   LZW                                      *
 *============================================================================*/

/*
 * Initializes dictionary.
 */
static code_t lzw_init(dictionary_t dict, int radix)
{
	for (int i = 0; i < radix; i++)
		dictionary_add(dict, 0, i, i);
	
	return (radix);
}

/*
 * Compress data.
 */
static void lzw_compress(buffer_t in, buffer_t out)
{
	unsigned ch;       /* Working character. */
	int i, ni;         /* Working entries.   */
	code_t code;       /* Current code.      */
	dictionary_t dict; /* Dictionary.        */
	
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
			if (ch != EOF)
				continue;
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
}

/*
 * Builds a string.
 */
static char *buildstr(char *base, char ch)
{
	char *s, *p;
	
	p = s = smalloc((strlen(base) + 2)*sizeof(char));
	
	while (*base != '\0')
		*p++ = *base++;
	
	*p++ = ch;
	*p = '\0';
	
	return (s);
}

/*
 * Decompress data.
 */
static void lzw_decompress(buffer_t in, buffer_t out)
{
	char *s, *p;   /* Working string. */
	unsigned code; /* Working code.   */
	unsigned i;    /* Loop index.     */
	char **st;     /* String table.   */
	
	st = smalloc(((1 << WIDTH) + 2)*sizeof(char *));
	
	/* Initializes the symbol table. */
	for (i = 0; i < RADIX; i++)
		st[i] = buildstr("", i);
	
	st[i++] =  buildstr("", ' ');
	
	code = buffer_get(in);
	
	/* Broken file. */
	if (code >= i)
		error("broken file");
	
	s = st[code];
	
	/* Decompress data. */
	while (1)
	{
		/* Output current string. */
		for (p = s; *p != '\0'; p++)
			buffer_put(out, (unsigned)(*p & 0xff));
		
		code = buffer_get(in);
		
		/* End of input. */
		if (code == EOF)
			break;
		
		/* Reset symbol table. */
		if (code == RADIX)
		{			
			while (i > 0)
				free(st[--i]);
			
			/* Initializes the symbol table. */
			for (i = 0; i < RADIX; i++)
				st[i] = buildstr("", i);
			
			st[i++] =  buildstr("", ' ');
			
			code = buffer_get(in);
	
			/* Broken file. */
			if (code >= i)
				error("broken file");
			
			s = st[code];
			
			continue;
		}
		
		/* Broken file. */
		if (code > i)
			error("broken file");
		
		p = st[code];
		
		if (i == code)
			p = buildstr(s, s[0]);
		
		st[i++] = buildstr(s, p[0]);
		
		s = p;
	}
	
	buffer_put(out, EOF);
	
	/* House keeping. */
	while (i > 0)
		free(st[--i]);
	free(st);
}

/*
 * Compress/Decompress a file using the LZW algorithm. 
 */
void lzw(FILE *inputFile, FILE *outputFile, int compress)
{
  /*<lpk> RETIRADO: buffers são usados em streams
    buffer_t inbuf;  // Input buffer.  
    buffer_t outbuf; // Output buffer. 

    inbuf = buffer_create(1024);
    outbuf = buffer_create(1024);
  </lpk>*/
  printf("Initializing LZW...\n");//<lpk> debug </lpk>
  pthread_t inthread, outthread;//<lpk> criação das threads </lpk>
  struct stream inattr, outattr;//<lpk> criação dos streams</lpk>

  int errnu=0;//<lpk> número do erro de criação da thread </lpk>
  inattr.buffer=buffer_create(1024);//<lpk>criacao dos buffer dentro dos seus streams</lpk>
	outattr.buffer=buffer_create(1024);
	inattr.file=inputFile;//<lpk> associação dos arquivos </lpk>
	outattr.file=outputFile;
	
	/* Compress mode. */	
	  /*<lpk> RETIRADO: funções serão chamadas em threads
	    lzw_readbytes(input, inbuf);
		lzw_compress(inbuf, outbuf);
		lzw_writebits(outbuf, output);
	  </lpk>*/
	//<lpk>lê arquivo em texto (texto a ser comprimido) com readbytes e escreve em binário (arquivo comprimidos) com writebits</lpk>
	if (compress)
	  { //<lpk> se acontecer erro, função retorna valor diferente de zero e entra no loop</lpk>
	  if((errnu=pthread_create(&inthread,
				  NULL,
				  lzw_readbytes,
				   &inattr))){
	    //<lpk> printa info de debug </lpk>
	    printf("Error creating readbytes thread.%d\n", errnu);
	    printf("Exiting process: %d.\n", (int)pthread_self());
	    //<lpk> sai do processo </lpk>
	    exit(-1);
	  }
	  //<lpk> mesmo padrão da criação da thread da readbytes acima. </lpk>
	  if((errnu=pthread_create(&outthread,
				    NULL,
				    lzw_writebits,
				     &outattr))){
	      printf("Error creating writebits thread. pid: %d.\n",errnu);
	      printf("Exiting process: %d.\n", (int)pthread_self());
	      exit(-1);
	  }
	  lzw_compress(inattr.buffer, outattr.buffer);  
	}
  //<lpk> mesmo padrão da compressão acima mas usando funçoes de leitura em binário (arquivo comprimido) com readbits e escrita em texto (arquivo de saída) com writebytes <lpk>	
	/* Decompress mode. */
	else
	{
	  if((errnu=pthread_create(&inthread,
				  NULL,
				  lzw_readbits,
				  &inattr))){
	    printf("Error creating readbits thread.%d\n", errnu);
	    printf("Existing process: %d.\n", (int)pthread_self());
	    exit(-1);
	  }
	  if((errnu=pthread_create(&outthread,
				    NULL,
				    lzw_writebytes,
				     &outattr))){
	      printf("Error creating writebytes thread. pid: %d.\n",errnu);
	     printf("Exiting process: %d.\n", (int)pthread_self());
		exit(-1);
	  }
	  lzw_decompress(inattr.buffer, outattr.buffer); 
	}
	//<lpk> aguarda as outras threads finalizarem. </lpk>
	pthread_join(inthread, NULL);
	pthread_join(outthread, NULL);

	buffer_destroy(outattr.buffer);
	buffer_destroy(inattr.buffer);
	printf("LZW Finalized\n"); //<lpk> debug </lpk>
}

