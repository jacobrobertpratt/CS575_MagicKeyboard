

#pragma once
#define _extern extern "C"


extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
}

/**
 * Error message definitions specific to the ULID library.
 */
#define ERROR_NULL      103     // "structure or data was null"
#define ERROR_PARAM     104     // "incorrect input parameters"
#define ERROR_CAPVID    105     // "failed to capture video frame"
#define ERROR_NOTIMP    106     // "not yet implemented"
#define	ERROR_FUNC		107		// "function failed"
#define ERROR_INIT		108		// "required sturcutres, vectors, or variables were not initialized"

#define ERROR_COUNT     ERROR_INIT  // Largest ERROR message

 /* Returns the number of #define states listed above */
_extern int mk_error_getArrSize();

/**
 * Gives the pointer to a already defined string value representing the error.
 * All error numbers are defined at the top of error.h (above)
 * @param errnum is a global error number (int) that is defined in error.h
 * @return (char *) of the string, or NULL on failure.
 */
_extern char* mk_error_getString(int errnum);

/**
 * Function will print the provided global integer message to the terminal. If a message
 * is added to the input paramters, then the messaged is tacked on to the end of the input
 * message. Else, if the message paramters is NULL, the message ignores it.
 * @param errnum global error number
 * @param message to add to the standard error message
 * @return error number
 */
_extern int mk_error_print(int errnum, const char* file, const char* func, int line, const char* format, ...);


/**
 * Macro that prints the error message to the terminal.
 *
 * The general format will be:
 *      [<file name>:<function name> @ <line number> ] Error: <error message here>.
 *
 * If verbose is specified then the above format iss the same save the ending which is verbose.
 *      [<file name>:<function name> @ <line number> ] Error: <error message here> - <verbose string here>.
 */
#define mk_verror(a,b,...) mk_error_print(a, __FILE__,__FUNCTION__,__LINE__, b, __VA_ARGS__)

 // non-verbose := same as verbose with null format and __VA_ARGS__
#define mk_error(a) mk_verror(a, 0, 0)
