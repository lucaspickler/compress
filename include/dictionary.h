/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <dictionary.h> - Dictionary library.
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

/*============================================================================*
 *                            Private Interface                               *
 *============================================================================*/

	/*
	 * Code.
	 */
	typedef unsigned code_t;

	/*
	 * Dictionary entry.
	 */
	struct entry
	{
		char ch;     /* Character.    */
		code_t code; /* Code.         */
		int parent;  /* Parent node.  */
		int next;    /* Next sibling. */
		int child;   /* First child.  */
	};

	/*
	 * Dictionary.
	 */
	struct dictionary
	{
		int max_entries;       /* Maximum number of entries. */
		int nentries;          /* Number of entries.         */
		struct entry *entries; /* Entries.                   */
	};
	
	/*
	 * Opaque pointer to a dictionary.
	 */
	typedef struct dictionary * dictionary_t;
	
/*============================================================================*
 *                             Public Interface                               *
 *============================================================================*/
 
	/*
	 * Adds a character to a dictionary entry.
	 */
	extern int dictionary_add(dictionary_t dict, int i, char ch, code_t code);
	
	/*
	 * Creates a dictionary.
	 */
	extern dictionary_t dictionary_create(int max_entries);
	
	/*
	 * Destroys a dictionary.
	 */
	extern void dictionary_destroy(dictionary_t dict);
	
	/*
	 * Searches for a character in a dictionary entry.
	 */
	extern int dictionary_find(dictionary_t dict, int i, char ch);
	
	/*
	 * Resets a dictionary.
	 */
	extern void dictionary_reset(struct dictionary *dict);

#endif
