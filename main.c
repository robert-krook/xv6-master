#include <stdio.h>
#include <math.h>

#include <string.h>

#include <stdarg.h>



// char offscreen_buffer[4][640*480/8]; //initialize offscreen buffer

// int draw_pixel
// (int x, int y, int color, int show) 
// {
	
// 	//if invalid color handle****
// 	if(color < 0 || color > 15)
// 		return -1;
	
// 	int temp_index = ((y * 640) + x);
// 	int bit_position = 7 - (temp_index % 8);
// 	int index = temp_index / 8;
// 	int bit_value;
// 	int mask;
// 	int color_position = 0x1;

// 	int i;
// 	for(i = 0; i < 4; i++) 
// 	{
// 		bit_value = color & color_position;
// 		if(bit_value == 0)
// 		{
// 			mask = color_position << bit_position;
// 			offscreen_buffer[i][index] = (offscreen_buffer[i][index] & mask);
// 		} else {		
// 			mask = bit_value << bit_position;
// 			offscreen_buffer[i][index] = (offscreen_buffer[i][index] | mask);
// 		}
// 		color = color >> 1;
// 	}

// 	return 0;
// }

// void plot_pixel_16color(int x, int y, int color) {
//     // 1. Calculate offset and bit mask
//     unsigned int base = (y * 640) + x;
//     unsigned int offset = base >> 3;       // divide by 8 for byte offset
//     unsigned char bitmask = 0x80 >> (x & 0x7); // 1 shifted by remainder
    
//     // 2. Plot color plane by plane
//     for (int plane = 0; plane < 4; plane++) {
//         // Select the VGA sequencer plane register
//         //outpw(0x03C4, ( (1 << plane) << 8 ) | 0x02);
        
//         // Write the bit to video memory (using dummy read-before-write or direct map)
//         unsigned char far *vidmem = (unsigned char far *)0xA0000000L;
        
//         if ((color >> plane) & 0x01) {

// offscreen_buffer[plane][offset] = |= bitmask; //(offscreen_buffer[plane][index] | mask);

//        //     vidmem[offset] |= bitmask;
//         } else {
//             offscreen_buffer[plane][offset] &= ~bitmask; 
//             //vidmem[offset] &= ~bitmask;
//         }
//     }
// }

// int clear_screen(void) {
// 	//call draw pixel on offscreen_buffer
//  	int x;
// 	int y;
// 	for(x = 0; x < 640; x++) 
// 	{	
// 		for( y = 0; y < 480; y++) 
// 		{
// 			draw_pixel(x, y, 1, 1);
// 		}
// 	}

// 	//Call blit to copy over buffer
// 	//blit();
// 	return 0;
// }


/*
 *  sprintf.c - concatenate strings into 1 string.
 */

//#include "stdarg.h"         // Our own implementation

// #include "types.h"
// #include "stat.h"
// #include "user.h"

void
sprintint(char * buf, int xx, int base, int sgn)
{
    char digits[] = "0123456789ABCDEF";
    char buf2 [16];
    int i, neg;
    int x;

memset (buf2, 0, sizeof (buf2));

    neg = 0;
    if(sgn && xx < 0)
    {
        neg = 1;
        x = -xx;
    } 
    else 
    {
        x = xx;
    }

    i = 0;
    do 
    {
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
wsprintf (char *str, char *fmt, ...)
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
                //putc(fd, c);
            }
        } else if(state == '%') {
            if(c == 'd') {
                char buf [16];
                memset (buf, 0, sizeof (buf));
                sprintint(buf, va_arg(ap, int), 10, 1);
 
int i = 0;

                while(buf [i] != 0) {
                //     putc(fd, *s);
                    return_str [index++] = buf [i];
                    i++;
                 }


            } else if(c == 'x' || c == 'p') {
                //printint(fd, va_arg(ap, int), 16, 0);
            } else if(c == 's') {
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




int main()
{
//    printf("Welcome\n");
//    clear_screen();
//draw_pixel(10, 10, 0, 1);

//float alpha;

 //alpha = (float) 250 / 255;

char buf [40];

  wsprintf (buf, "%d%d%d%d%d",  6, 25, 14, 18, 30);

    printf ("%s\n", buf);

    return (0);
}
