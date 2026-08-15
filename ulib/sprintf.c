
/*
 *  sprintf.c - concatenate strings into 1 string.
 */

#include "stdarg.h"         // Our own implementation

#include "types.h"
#include "stat.h"
#include "user.h"

void
sprintint(char * buf, int xx, int base, int sgn)
{
    char digits[] = "0123456789ABCDEF";
    char buf2 [16];
    int i, neg;
    uint x;

    memset (buf2, 0, sizeof (buf2));

    neg = 0;
    if (sgn && xx < 0) {
        neg = 1;
        x = -xx;
    }  else {
        x = xx;
    }

    i = 0;
    do {
        buf2[i++] = digits[x % base];
    } while((x /= base) != 0);

    if(neg)
        buf2[i++] = '-';

    int j = 0;
    while(--i >= 0)
        buf [j++] = buf2 [i];
//     putc(fd, buf[i]);
}

/* 
 *  Only understands %d, %x, %p, %s.
 *
 *  Return the number of character in the str.
 */
int
sprintf (char *str, char *fmt, ...)
{
    va_list ap;
    char *s;
    int c, i, state;

    int index = 0;
    char return_str [256];

    memset (return_str, 0, sizeof (return_str));

    va_start(ap, fmt);

    state = 0;
    for(i = 0; fmt[i]; i++) {

        c = fmt[i] & 0xff;
        if(state == 0) {
            if(c == '%') {
                state = '%';
            } else {
                return_str [index++] = c;
                //putc(fd, c);
            }
        } 
        else if(state == '%') 
        {
            if(c == 'd') 
            {
                char buf [16];
                memset (buf, 0, sizeof (buf));
                sprintint(buf, va_arg(ap, int), 10, 1);
                int i = 0;
                while(buf [i] != 0) 
                {
                    return_str [index++] = buf [i];
                    i++;
                 }

            } else if(c == 'x' || c == 'p') 
            {
                //printint(fd, va_arg(ap, int), 16, 0);
            } 
            else if(c == 's') 
            {
                s = va_arg(ap, char*);
                if(s == 0)
                    s = "(null)";
                
                while(*s != 0) {
                //     putc(fd, *s);
                    return_str [index++] = *s;
                     s++;
                 }
            } else if(c == 'c') {
                //putc(fd, va_arg(ap, uint));
            } else if(c == '%') {
                //putc(fd, c);
            } else {
                // Unknown % sequence.  Print it to draw attention.
                 
                //putc(fd, '%');
                //putc(fd, c);
            }
            state = 0;
        }
    }

    return_str [index] = 0;

    strcpy (str, return_str);

    return strlen (str);
}
