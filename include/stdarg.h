
/*
 *  stdarg.h - our implementation of stdargs.h
 *
 *  Refactor:
 * 
 *      [ ] We make use of __builtin.... of the compiler. We need our own implementation
 *          for the future.
 */

typedef __builtin_va_list   va_list;

#define va_start(v,l)	    __builtin_va_start(v,l)
#define va_end(v)	        __builtin_va_end(v)
#define va_arg(v,l)	        __builtin_va_arg(v,l)
