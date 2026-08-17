
/*
 *  printf.c -- print string in console.
 *
 * To Do
 * 
 * [ ] %-50s    Left aligned string of width 50
 * [ ] %.10d    Preleading zeros of total 10 positions
 * [ ] %.7s     Maximum of 7 positions
 * 
 */

#include "stdarg.h"         // Our own implementation

#include "types.h"
#include "stat.h"
#include "user.h"

#include "ctype.h"

static void
putc (int fd, char c)
{
    write(fd, &c, 1);
}

static void
printint (int fd, int xx, int base, int sgn)
{
    static char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    
    if (sgn && xx < 0) {
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;

    do {
        buf[i++] = digits[x % base];
    } while((x /= base) != 0);
    
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
        putc(fd, buf[i]);
}

/*
 *
 * Print to the given fd. Only understands %d, %x, %p, %s.
 * 
 * Examples:
 * 
 * printf (0, "%10d", 10) => output 
 * printf (0, "%5s", "Test")
 * 
 */
void
printf (int fd, char *fmt, ...)
{
    va_list ap;
    char *s;
    int c, i, state;

    char prefix [10];       // Prefix for string or integer
    char precision [10];    // Precision (behind the comma)

    va_start (ap, fmt);

    state = 0;

    for (i = 0; fmt[i]; i++) {

        c = fmt[i] & 0xff;

        if(state == 0) {

            if(c == '%') {
                state = '%';
                memset (prefix, 0, sizeof (prefix));
            } else {
                putc(fd, c);
            }

        } else if(state == '%') {
            
            if(c == 'd') {
                printint (fd, va_arg(ap, int), 10, 1);

            } else if(c == 'x' || c == 'p') {
                printint(fd, va_arg(ap, int), 16, 0);

            } else if(c == 's') {

                s = va_arg(ap, char*);

                if (s == 0)
                    s = "(null)";

                int fill_out = atoi (prefix) - strlen (s);

                if (strlen (prefix)>0) {
                    for (int k=0; k<fill_out; k++)
                        putc (fd, ' ');
                }

                while (*s != 0) {
                    putc(fd, *s);
                    s++;
                }

                state = 0;

            } else if(c == 'c') {
                putc(fd, va_arg(ap, uint));

            } else if(c == 'f') {
                
                putc (fd, 'h');
                putc (fd, 'i');

                double g = va_arg (ap, double);
                int k = (int) g;
                printint(fd, k, 10, 1);

                return;

            } else if(c == '%') {
                putc(fd, c);

            } else {
                if (!isdigit (c)) {
                    // Unknown % sequence.  Print it to draw attention. It could be
                    // that a number follows for insert spaces.
                    //putc(fd, '%');
                    putc(fd, c);
                    state = 0;
                } else {
                    // We got the format of %2.2f for example.
                    int j = 0;
                    while (isdigit (c)) {
                        prefix [j] = c;
                        j++;
                        i++;
                        if (i>strlen (fmt))
                            return;
                        c = fmt[i] & 0xff;
                    }
                    if (c=='.')
                    {
                        // We have to go for the Precision 
                    }
                    i--;
                }
            }
        }
    }
}
