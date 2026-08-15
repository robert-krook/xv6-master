
/*
 *  stdio.c -- standard I/O
 *
 *  To Do
 * 
 *  [ ] fclose ()
 *  [ ] feof ()
 *  [ ] ferror ()
 *  [ ] printf ()
 *  [ ] sprintf ()
 * 
 */

#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"

int
getchar (void)
{
    int cc;
    char c;

    while (1) {
        cc = read (0, &c, 1);
        if (cc < 1)
            return 0;
        if (cc >= 1)
        break;
    }

    return c;
}

char *
gets (char *buf, int max)
{
    int i, cc;
    char c;

    for(i=0; i+1 < max; ) {

        cc = read(0, &c, 1);
        if(cc < 1)
            break;

        buf[i++] = c;

        if(c == '\n' || c == '\r')
            break;

    }
    buf[i] = '\0';
    return buf;
}

