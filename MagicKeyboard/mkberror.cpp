#include "mkberror.hpp"

static const char* errorArray[] = {
    "structure or data was null",
    "incorrect input parameters",
    "failed to capture video frame",
    "not yet implemented",
    "function failed",
    "required sturcutres, vectors, or variables were not initialized"
};

_extern int mk_error_getArrSize() {
    return ERROR_COUNT - 102;
}

_extern char* mk_error_getString(int errnum) {
    char* retStr = NULL;
    // Gets the maximum value of the possible errornum
    int max_errnum = ERROR_COUNT;
    if (errnum > max_errnum)
        return NULL;
    // checks if standard errno number or local error number
	if (errnum < 103)
		//retStr = strerror(errnum); // Unsafe, deprecation error
		return NULL; // <-- What is this
    else {
        errnum = errnum - 103;
        retStr = (char*)errorArray[errnum];
    }
    return retStr;
}

_extern int mk_error_print(int errnum, const char* file, const char* func, int line, const char* format, ...) {
    char* err_str = NULL;
    err_str = mk_error_getString(errnum);

    if (!err_str) {
        printf("[%s:%s@%d] Error[%d]: invalide error code \n", func, file, line, errnum);
        goto end;
    }

    if (!format) {
        printf("[%s:%s@%d] Error[%d]: %s\n", func, file, line, errnum, err_str);
        goto end;
    }

    printf("[%s:%s@%d] Error %d: %s - ", func, file, line, errnum, err_str);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

end:
    return errnum;
}