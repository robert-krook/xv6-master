/*  
    ulib.c -- user lib functions.

    strcpy  -- copy character string from source to destination.
    strcmp  -- compare two strings.

*/

#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

// char *
// strncpy (char *s, const char *t, int n)
// {
//   char *os;
  
//   os = s;
//   while(n-- > 0 && (*s++ = *t++) != 0)
//     ;
//   while(n-- > 0)
//     *s++ = 0;
//   return os;
// }


// char *
// strcpy (char *dst, char *src)
// {
//   char *os;

//   os = dst;
//   while((*dst++ = *src++) != 0)
//     ;
//   return os;
// }

// int
// strcmp(const char *p, const char *q)
// {
//   while(*p && *p == *q)
//     p++, q++;
//   return (uchar)*p - (uchar)*q;
// }

// uint
// strlen(char *s)
// {
//   int n;

//   for(n = 0; s[n]; n++)
//     ;
//   return n;
// }

// void *
// memset (void *dst, int c, uint n)
// {
//   stosb (dst, c, n);
//   return dst;
// }

// char *
// strchr(const char *s, char c)
// {
//   for(; *s; s++)
//     if(*s == c)
//       return (char*)s;
//   return 0;
// }

// int
// getchar (void)
// {
//     int cc;
//     char c;

// while (1) {
//     cc = read (0, &c, 1);
//     if (cc < 1)
//         return 0;
//     if (cc >= 1)
//     break;
// }

//     return c;
// }

// char *
// gets (char *buf, int max)
// {
//     int i, cc;
//     char c;

//     for(i=0; i+1 < max; ) {

//         cc = read(0, &c, 1);
//         if(cc < 1)
//             break;

//         buf[i++] = c;

//         if(c == '\n' || c == '\r')
//             break;

//     }
//     buf[i] = '\0';
//     return buf;
// }

// int
// stat(char *n, struct stat *st)
// {
//   int fd;
//   int r;

//   fd = open(n, O_RDONLY);
//   if(fd < 0)
//     return -1;
//   r = fstat(fd, st);
//   close(fd);
//   return r;
// }

// int
// atoi(const char *s)
// {
//   int n;

//   n = 0;
//   while('0' <= *s && *s <= '9')
//     n = n*10 + *s++ - '0';
//   return n;
// }

// void *
// memmove(void *vdst, void *vsrc, int n)
// {
//   char *dst, *src;
  
//   dst = vdst;
//   src = vsrc;
//   while(n-- > 0)
//     *dst++ = *src++;
//   return vdst;
// }

// int
// strncmp (const char *p, const char *q, uint n)
// {
//   while(n > 0 && *p && *p == *q)
//     n--, p++, q++;
//   if(n == 0)
//     return 0;
//   return (uchar)*p - (uchar)*q;
// }

// /* returns true for all whitespace chars */
// int isspace(int c)
// {
//     switch(c)
//     {
//         case TAB:
//         case LF:
//         case VT:
//         case FF:
//         case CR:
//         case SPACE:
//             return true;
//         default:
//             return false;
//     }
// }

// /* returns true for tab and space only */
// int isblank(int c)
// {
//     if( c==TAB || c==SPACE )
//         return true;
//     else
//         return false;
// }


