
#include "types.h"
#include "x86.h"

extern char * strcpy (char *dst, char *src);

/*
 *  String copy from src to dst.
 */
char *
strcpy (char *dst, char *src)
{
    char *os;

    os = dst;
    while((*dst++ = *src++) != 0)
        ;

    return os;
}

/*
 *  String compare.
 */
int
strcmp (const char *p, const char *q)
{
    while(*p && *p == *q)
        p++, q++;
    return (uchar)*p - (uchar)*q;
}

/*
 *  Get the string length.
 */
uint
strlen(char *s)
{
    int n;

    for(n = 0; s[n]; n++)
        ;

    return n;
}

char *
strncpy (char *s, const char *t, int n)
{
    char *os;
  
    os = s;

    while(n-- > 0 && (*s++ = *t++) != 0)
        ;
    while(n-- > 0)
        *s++ = 0;

    return os;
}

void *
memset (void *dst, int c, uint n)
{
    stosb (dst, c, n);
    return dst;
}

char *
strchr (const char *s, char c)
{
    for (; *s; s++)
        if (*s == c)
            return (char *)s;
    return 0;
}

int
strncmp (const char *p, const char *q, uint n)
{
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uchar)*p - (uchar)*q;
}

void *
memmove (void *vdst, void *vsrc, int n)
{
    char *dst, *src;
  
    dst = vdst;
    src = vsrc;

    while(n-- > 0)
        *dst++ = *src++;

    return vdst;
}

char *
strcat (char *dest, const char *src)
{
    int i,j;

    for (i = 0; dest[i] != '\0'; i++)
        ;

    for (j = 0; src[j] != '\0'; j++)
        dest [i+j] = src [j];

    dest [i+j] = '\0';

    return dest;
}
