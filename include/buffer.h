/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <buffer.h> - Circular buffer library.
 */

#ifndef BUFFER_H_
#define BUFFER_H_

	/*
	 * Opaque pointer to a circular buffer.
	 */
	typedef struct buffer * buffer_t;

	/* Forward definitions. */
	extern void buffer_destroy(buffer_t);
	extern unsigned buffer_get(buffer_t);
	extern buffer_t buffer_create(unsigned);
	extern void buffer_put(buffer_t, unsigned);

#endif /* BUFFER_H_ */
