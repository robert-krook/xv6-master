
#include "types.h"
#include "libc/string.h"
#include "math.h"
#include "libc/stdlib.h"

int 
setenv (const char *name, const char *value, int overwrite)
{
    return 0;
}

int
atoi(const char *s)
{
    int n;

    n = 0;
    while('0' <= *s && *s <= '9')
        n = n*10 + *s++ - '0';

    return n;
}

void 
reverse (char *str, int len)
{
    int i = 0, j = len - 1, temp;

    while (i < j) {
        temp = str [i];
        str [i] = str [j];
        str [j] = temp;
        i++;
        j--;
    }

}

int 
int2str (int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str [i++] = (x % 10) + '0';
        x = x / 10;
    }

    while (i < d) {
        str [i++] = '0';
    }

    reverse (str, i);
    str [i] = '\0';
    return i;
}

void
ftoa (float n, char *result, int precision)
{
    int ipart = (int) n;

    float fpart = n  - (float) ipart;

    int i = int2str (ipart, result, 0);

    if (precision != 0) {
        result [i] = '.';

        fpart = fpart * pow(10, precision);

        int2str ((int) fpart, result + i + 1, precision);
    }

}

void
itoa (int xx, char *str, int base, int sgn)
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
    } while ((x /= base) != 0);
    
    if (neg)
        buf[i++] = '-';


int j = 0;

    while(--i >= 0)
         str [j++] = buf[i];

str [j++] = '\0';

    //strncpy (str, buf, strlen (buf));
}

