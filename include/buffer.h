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
	
	/*
	 * Closes a circular buffer.
	 */
	extern void buffer_close(buffer_t buf);
	
	/*
	 * Gets an item from a circular buffer.
	 */
	extern unsigned buffer_get(buffer_t buf);
	
	/*
	 * Opens a circular buffer.
	 */
	extern buffer_t buffer_open(unsigned size);
	
	/*
	 * Puts an item in a circular buffer.
	 */
	extern void buffer_put(buffer_t buf, unsigned item);

#endif /* BUFFER_H_ */
