/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <utility.h> - Utility library.
 */

#ifndef UTIL_H_
#define UTIL_H_

	#include <stdlib.h>

	/* Forward definitions. */
	extern void error(const char *);
	extern void warning(const char *);
	extern void *smalloc(size_t);
	extern void *srealloc(void *, size_t);

#endif /* UTIL_H_ */
