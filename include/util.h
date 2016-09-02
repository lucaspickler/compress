/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <utility.h> - Utility library.
 */

#ifndef UTIL_H_
#define UTIL_H_

	#include <stdlib.h>

	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);
	
	/*
	 * Prints a warning message.
	 */
	extern void warning(const char *msg);
	
	/*
	 * Safe malloc().
	 */
	extern void *smalloc(size_t size);

#endif /* UTIL_H_ */
