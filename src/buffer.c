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

/*
Modificado por Lucas Pickler Knaul em 09/10/2016.
Modificações marcadas com as tags <lpk> </lpk>.
 */

#include <stdio.h>
#include <assert.h>
#include <buffer.h>
#include <pthread.h>
#include <util.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
 * Buffer.
 */
struct buffer
{
  sem_t* qtt;//<lpk> semáforo pertence ao buffer </lpk>
	unsigned *data; /* Data.                        */
	unsigned size;  /* Max size (in elements).      */
	unsigned first; /* First element in the buffer. */
	unsigned last;  /* Last element in the buffer.  */
};

/*
 * Creates a buffer.
 */
struct buffer *buffer_create(unsigned size)
{
	struct buffer *buf;
	
	/* Sanity check. */
	assert(size > 0);

	buf = smalloc(size*sizeof(struct buffer));
	
	/* Initialize buffer. */
	buf->qtt = smalloc(sizeof(sem_t));//<lpk> alocação de memória ao ponteiro do semáforo. </lpk>
	int errnum; //<lpk> guardará possível erro na criação do semáforo. </lpk>
	if((errnum=sem_init(buf->qtt, 0, 0))){ //<lpk>inicializacao do semaforo do buffer buf com 0 atributos de compartilhamento e valor inicial 0.</lpk>
	  //<lpk> printa informações de debug. </lpk>
	 printf("Error creating semaphore: %d,\n", errnum);
	  printf("Exiting.\n");
	  // <lpk> sai do processo </lpk>
	  exit(-1);
	}
	buf->size = size;
	buf->data = smalloc(size*sizeof(unsigned));
	buf->first = 0;
	buf->last = 0;

	return (buf);
}

/*
 * Destroys a buffer.
 */
void buffer_destroy(struct buffer *buf)
{
	/* Sanity check. */
	assert(buf != NULL);
	
	/* House keeping. */
	free(buf->data);
	sem_destroy(buf->qtt); //<lpk> destrói semáforo do buffer </lpk>
	free(buf);
}

/*
 * Puts an item in a buffer.
 */
void buffer_put(struct buffer *buf, unsigned item)
{
  //<lpk> informações de debug </lpk>
  int debug_semsize;
  sem_getvalue(buf->qtt,&debug_semsize);
  printf("thread: %d | ",(int)pthread_self()); //<lpk> pid da thread. </lpk>
  printf("buffer_put: %d. | ",debug_semsize); //<lpk> tamanho do semáforo do buffer buf. </lpk>
  printf("from semaphor: %d\n | ",(int)buf->qtt); //<lpk> valor do ponteiro para o semáforo para identificação. </lpk>
	/* Sanity check. */
	assert(buf != NULL);

	/* Expand buffer. */
	if (buf->last == buf->size)
	{
		buf->data = srealloc(buf->data, 2*buf->size*sizeof(unsigned));
		buf->size *= 2;
	}

	buf->data[buf->last++] = item;
	sem_post(buf->qtt);//<lpk> elemento foi inserido. incrementa semáforo permitindo a uma outra thread consumir o elemento. </lpk>
}

/*
 * Gets an item from a buffer.
 */
unsigned buffer_get(struct buffer *buf)
{
  int debug_semsize;
  sem_getvalue(buf->qtt,&debug_semsize);
  printf("thread: %d | ",(int)pthread_self());
  printf("buffer_get: %d. | ",debug_semsize);
  printf("from semaphor: %d\n | ",(int)buf->qtt);
	unsigned item;
	
	/* Sanity check. */
	assert(buf != NULL);
	
	sem_wait(buf->qtt);//<lpk> se semáforo não está zerado, existem elementos a ser consumidos, entao decrementa semáforo e consome elemento. Se semáforo está zerado, buffer está vazio, então processo dorme até buffer_put enchê-lo e incrementar semáforo. </lpk>
	item = buf->data[buf->first++];
	return (item);
}
