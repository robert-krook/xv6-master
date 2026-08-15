
/*
 *  ctype.c -- provides many functions for classifying and modifying characters.
 *
 *  To Do -- next functions
 * 
 *  isalnum ()
 *  isalpha ()
 *  isbllank ()
 *  iscntrl ()
 *  isgraph ()
 *  islower ()
 *  isprint ()
 *  ispunct ()
 *  isspace ()
 *  isupper ()
 *  isxdigit ()
 *  tolower ()
 *  toupper ()
 * 
 */

#include "types.h"

#include "ctype.h"

/*
 *  isdigit -- a digit (0 to 9)
 */
bool
isdigit (char c)
{
    if (c >= 0x30 && c<=0x39)
        return true;
    else
        return false;
}


/* 
 * isspace -- a whitespace character (space, tab, carriage return, new line, vertical tab, or formfeed)
 */
int isspace (int c)
{
    switch(c)
    {
        case TAB:
        case LF:
        case VT:
        case FF:
        case CR:
        case SPACE:
            return true;
        default:
            return false;
    }
}

/* 
 * isblank -- returns true for tab and space only 
 */
int isblank (int c)
{
    if( c==TAB || c==SPACE )
        return true;
    else
        return false;
}
