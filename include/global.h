/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <global.h> - Global variables and functions.
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
	
	#include <stdio.h>
	
	/*
	 * Compress/decompress a file using LZW algorithm.
	 */
	extern void lzw(FILE *input, FILE *output, int compress);

#endif /* GLOBAL_H_ */
