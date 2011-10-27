/* A header file containing various useful functions, etc. */

#ifndef _UTILS_H
#define _UTILS_H

	/* Standard Headers */
		#ifndef _STDIO_H
			#include <stdio.h>
		#endif
		#ifndef _STDLIB_H
			#include <stdlib.h>
		#endif
		#include <time.h>

	/* DEBUG MACROS */
		#define DEBUG fprintf(stderr,"Reached line %d in %s\n",__LINE__,__FILE__); //prints "Reached line <current line> at <current file>" to standard error
		#define DEBUGNUM(var) fprintf(stderr,"At line %d in %s: %s = %d\n",__LINE__,__FILE__, #var , var ); //prints line num, file, number variable name and value
		#define DEBUGSTRING(var) fprintf(stderr,"At line %d in %s: %s = %s\n",__LINE__,__FILE__, #var , var ); //prints line num, file, string variable name and value
		#define DEBUGMSG(msg) fprintf(stderr,"At line %d in %s: %s\n",__LINE__,__FILE__, msg ); //prints line num, file and custom message
		#define DEBUGVAR(fmt,var) fprintf(stderr,"At line %d in %s: %s = ",__LINE__,__FILE__,#var);fprintf(stderr,fmt,var);fprintf(stderr,"\n"); // prints line num, file, variable name and value as according to format string
		#define DEBUGARRAY(fmt, ptr, size, iter) fprintf(stderr,"At line %d in %s: %s[0:%s-1] = {",__LINE__,__FILE__,#ptr,#size); for((iter)=0;(iter)<(size);(iter)++) fprintf(stderr, fmt ", ", (ptr)[(iter)]); fputs("\b\b}\n", stderr); // prints line num, file, array name and range, and "{a, b, c, ...}" where a, b, c, etc (up to size) are elements of array, with fmt as their format string. fmt must be literal. iter must be a valid, usable iteration variable.
		#define LOOP_START(limit) int loop_debug_count = 0; int loop_debug_limit = (limit); // Put before a loop to use LOOP_CHECK to abort if it goes too long
		#define LOOP_CHECK if (loop_debug_count++ >= loop_debug_limit) {break;}; // Put in a loop (after LOOP_START) to abort loop if it loops (limit) times.

	/* UTIL MACROS */
		#define STRING_LENGTH 256 //max length of strings, strings will use this much memory regardless of actual used size
		#define FOR_ZERO_TO_N(var, max) for ((var)=0;(var)<(max);(var)++)
		#define SQUARE_DIST(x,y) (x)*(x)+(y)*(y)
		#define MAX(x,y) ((x)>(y))?(x):(y)
		#define MIN(x,y) ((x)<(y))?(x):(y)
		#define INRANGE(x,y,a) ((a)>=(x) != ((a)>=(y))) // returns true if a is in [x,y)
		#define TRUE 1
		#define FALSE 0
		#define STR_EQUAL(a,b) (strcmp((a),(b)) == 0)
		#define STR_CLONE(dest, source) (dest) = malloc(strlen((source)+1)); strcpy((dest),(source)) // make new memory and copy string into it
		#define FAILED(val) ((val) != EXIT_SUCCESS)

	/* Primitives */

	/* Methods */

#endif // end of multiple inclusion protection
