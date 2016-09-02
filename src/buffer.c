/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <buffer.h>
#include <pthread.h>
#include <util.h>

/*============================================================================*
 *                             Buffer Implementation                          *
 *============================================================================*/

/*
 * Buffer.
 */
struct buffer
{
	unsigned *data;            /* Data.                        */
	unsigned size;             /* Max size (in elements).      */
};

/*
 * Opens a buffer.
 */
struct buffer *buffer_create(unsigned size)
{
	struct buffer *buf;
	
	buf = smalloc(size*sizeof(struct buffer));
	
	/* Initialize buffer. */
	buf->size = size;
	buf->data = smalloc(size*sizeof(unsigned));
	
	return (buf);
}

/*
 * Closes a buffer.
 */
void buffer_destroy(struct buffer *buf)
{
	/* Sanity check. */
	assert(buf != NULL);
	
	/* House keeping. */
	free(buf->data);
	free(buf);
	
}

/*
 * Puts an item in a buffer.
 */
void buffer_put(struct buffer *buf, unsigned item)
{
	/* Sanity check. */
	assert(buf != NULL);

	buf->data[buf->last] = item;
	buf->last = (buf->last + 1)%buf->size;
}

/*
 * Gets an item from a buffer.
 */
unsigned buffer_get(struct buffer *buf)
{
	unsigned item;
	
	/* Sanity check. */
	assert(buf != NULL);
	
	item = buf->data[buf->first];
	buf->first = (buf->first + 1)%buf->size;

	return (item);
}
