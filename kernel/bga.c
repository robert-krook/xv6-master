
/*
 *  bga.c --
 */

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "msg.h"
#include "spinlock.h"
#include "traps.h"

#include "memlayout.h"
#include "bga.h"

#include "gui/bga_character.h"
#include "gui/bga_icons.h"
#include "gui/bga_mouse_shape.h"

#define VBE_DISPI_IOPORT_INDEX              0x01CE
#define VBE_DISPI_IOPORT_DATA               0x01CF

//#define VBE_DISPI_INDEX_ID                  0

#define VBE_DISPI_ID0                      0xB0C0
#define VBE_DISPI_ID1                      0xB0C1
#define VBE_DISPI_ID2                      0xB0C2
#define VBE_DISPI_ID3                      0xB0C3
#define VBE_DISPI_ID4                      0xB0C4


 #define VBE_DISPI_INDEX_ID                 0x0
 #define VBE_DISPI_INDEX_XRES               0x1
 #define VBE_DISPI_INDEX_YRES               0x2
 #define VBE_DISPI_INDEX_BPP                0x3
 #define VBE_DISPI_INDEX_ENABLE             0x4
 #define VBE_DISPI_INDEX_BANK               0x5
 #define VBE_DISPI_INDEX_VIRT_WIDTH         0x6
 #define VBE_DISPI_INDEX_VIRT_HEIGHT        0x7
 #define VBE_DISPI_INDEX_X_OFFSET           0x8
 #define VBE_DISPI_INDEX_Y_OFFSET           0x9
 #define VBE_DISPI_INDEX_VIDEO_MEMORY_64K   0xa

#define VBE_DISPI_LFB_ENABLED               0x40
#define VBE_DISPI_NOCLEARMEM                0x80

#define VBE_DISPI_DISABLED                  0x00
//#define VBE_DISPI_INDEX_ENABLE              0x04
#define VBE_DISPI_ENABLED                   0x01

// Pointer to banked BGA buffer
unsigned char * bga_lfb = (unsigned char *) P2V (0xa0000);

#define BGA_SCREEN_WIDTH                    1024
#define BGA_SCREEN_HEIGHT                   768

short bga_nr_banks = BGA_SCREEN_HEIGHT / 64;

#define BGA_FB_VADDR 0xFFFFFFFFD0000000 

unsigned char * bga_offscreen_buffer; //initialize offscreen buffer

unsigned char * bga_offscreen_buffer_24; //initialize offscreen buffer


struct spinlock bga_sl;

// Define off screen buffer (refresh vi blitter)
//char   bga_offscreen_buffer    [12][1024*768]; //initialize bga offscreen buffer
//char   bga_offscreen_buffer    [12][1024*768]; //initialize bga offscreen buffer

// unsigned char   bga_offscreen_buffer    [bga_nr_banks] 
//                                         [BGA_SCREEN_WIDTH * BGA_SCREEN_HEIGHT]; //initialize bga offscreen buffer

void
bga_write_register (unsigned short IndexValue, unsigned short DataValue)
{
    outw(VBE_DISPI_IOPORT_INDEX, IndexValue);
    outw(VBE_DISPI_IOPORT_DATA, DataValue);
}

unsigned short 
bga_read_register (unsigned short IndexValue)
{
    outw(VBE_DISPI_IOPORT_INDEX, IndexValue);
    return inw(VBE_DISPI_IOPORT_DATA);
}

int 
bga_is_available (void)
{
    return (bga_read_register (VBE_DISPI_INDEX_ID) == VBE_DISPI_ID4);
}

void 
bga_set_video_mode (unsigned int Width, unsigned int Height, unsigned int BitDepth, 
                    int UseLinearFrameBuffer, int ClearVideoMemory)
{
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_XRES, Width);
    bga_write_register(VBE_DISPI_INDEX_YRES, Height);
    bga_write_register(VBE_DISPI_INDEX_BPP, BitDepth);
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED |
        (UseLinearFrameBuffer ? VBE_DISPI_LFB_ENABLED : 0) |
        (ClearVideoMemory ? 0 : VBE_DISPI_NOCLEARMEM));
}

void 
bga_set_bank (unsigned short BankNumber)
{
    bga_write_register(VBE_DISPI_INDEX_BANK, BankNumber);
}

// void
// bga_color_demo ()
// {
//     int offset = (1 * BGA_SCREEN_WIDTH + 1);
    
//     for (int b=0; b<(bga_nr_banks + 1); b++) {
//         for (int y=0; y<64; y++)
//             for (int x=0; x<(BGA_SCREEN_WIDTH); x++) {
//                 offset = (b * 64 * 1024) + (y * BGA_SCREEN_WIDTH + x);
//                 *(bga_offscreen_buffer + offset) = b*5;
//         }
//     }

// }

void bga_init ()
{
    cprintf ("bga_init\n");

    bga_mouse_color [0] = 0;
    bga_mouse_color [1] = 2;

    bga_offscreen_buffer = (unsigned char *) 0xFFFFFFFF90000000;

    // Switch to graphics mode.
    bga_set_video_mode (1024, 768, 8, 0, 1);
    bga_clear_screen ();
    bga_blit ();

	//Initialize the spinlock
	initlock (&bga_sl, "spin");
}

void bga_init_24 ()
{
    cprintf ("bga_init\n");

    bga_mouse_color_24 [0].R = 255;
    bga_mouse_color_24 [0].G = 0;
    bga_mouse_color_24 [0].B = 0;
    bga_mouse_color_24 [0].A = 255;

    bga_mouse_color_24 [1].R = 255;
    bga_mouse_color_24 [1].G = 255;
    bga_mouse_color_24 [1].B = 255;
    bga_mouse_color_24 [1].A = 255;

    bga_offscreen_buffer_24 = (unsigned char *) 0xFFFFFFFF90000000;

    bga_nr_banks = 3 * (BGA_SCREEN_HEIGHT / 64);

    // Switch to graphics mode.
    bga_set_video_mode (1024, 768, 24, 0, 1);
    bga_clear_screen_24 ();

// RGBA rgba;
// rgba.R = 255;
// rgba.G = 0;
// rgba.B = 0;
// rgba.A = 255;

//     bga_draw_pixel_24 (1020, 760, rgba);


//     bga_draw_line_24 (1, 1, 1023, 767, rgba);

//     bga_draw_character_24 (500, 500, 'A', rgba);

// bga_draw_rect_border_24 (rgba, 30, 39, 300, 100);

// bga_draw_rect_by_coord_24 (10, 10, 200, 200, rgba);

// bga_draw_icon_24 (300, 300, 4, rgba);

    bga_blit_24 ();

	//Initialize the spinlock
	initlock (&bga_sl, "spin");
}

int
bga_blit ()
{
    //int offset = 0;

    for (int b=0; b<bga_nr_banks; b++) {
        bga_set_bank (b);
        memmove (bga_lfb, bga_offscreen_buffer+(b * 64 * 1024), 64 * 1024 );
        //  for (int y=0; y<64; y++)
        //      for (int x=0; x<(BGA_SCREEN_WIDTH); x++) {
        //          offset = (b * 64 * 1024) + (y * BGA_SCREEN_WIDTH + x);
        //              *(bga_lfb+(y * 1024 + x)) = *(bga_offscreen_buffer + offset); // b*5;
        //  }
    }
    return 0;
}


int
bga_blit_24 ()
{
    //int offset = 0;

    for (int b=0; b<bga_nr_banks; b++) {

        bga_set_bank (b);
        memmove (bga_lfb, bga_offscreen_buffer_24+(b * 64 * 1024), 64 * 1024);
        //  for (int y=0; y<64; y++)
        //      for (int x=0; x<(BGA_SCREEN_WIDTH); x++) {
        //          offset = (b * 64 * 1024) + (y * BGA_SCREEN_WIDTH + x);
        //              *(bga_lfb+(y * 1024 + x)) = *(bga_offscreen_buffer + offset); // b*5;
        //  }
    }
    return 0;
}

void
bga_clear_screen ()
{
    memset (bga_offscreen_buffer, 0, (1024*768));
}

void
bga_clear_screen_24 ()
{
    memset (bga_offscreen_buffer_24, 0, 768 * 1024 * 3);
}

int
bga_draw_pixel (int x, int y, int color)
{
    *(bga_offscreen_buffer + (y * BGA_SCREEN_WIDTH + x)) = color;
    return 0;
}

int
bga_draw_pixel_24 (int x, int y, RGBA color)
{
    RGB *t;

    t = (RGB *)(bga_offscreen_buffer_24 + ((y * 3) * BGA_SCREEN_WIDTH + (x * 3)));

    t->R = color.R;
    t->G = color.G;
    t->B = color.B;

    return 0;
}

//Bresenham's algorithm Taken and adjusted from http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
int 
bga_draw_line (int x1, int y1, int x2, int y2, int color) 
{
	int xdiff = x2 - x1;
	int ydiff = y2 - y1;
	
	int dx = xdiff * ( (xdiff < 0) * (-1) + (xdiff > 0)), sx = x1 < x2 ? 1 : -1;
	int dy = ydiff * ( (ydiff < 0) * (-1) + (ydiff > 0)), sy = y1 < y2 ? 1 : -1;
	int err = (dx > dy ? dx : -dy)/2, e2;

	for(;;)
    {
		bga_draw_pixel(x1, y1, color);
		if(x1 == x2 && y1 == y2) break;
		e2 = err;
		if(e2 > -dx) { err -= dy; x1 += sx; }
		if(e2 < dy) { err += dx; y1 += sy; }
	}

	return 0;
}

int 
bga_draw_line_24 (int x1, int y1, int x2, int y2, RGBA color) 
{
	int xdiff = x2 - x1;
	int ydiff = y2 - y1;
	
	int dx = xdiff * ( (xdiff < 0) * (-1) + (xdiff > 0)), sx = x1 < x2 ? 1 : -1;
	int dy = ydiff * ( (ydiff < 0) * (-1) + (ydiff > 0)), sy = y1 < y2 ? 1 : -1;
	int err = (dx > dy ? dx : -dy)/2, e2;

	for(;;)
    {
		bga_draw_pixel_24(x1, y1, color);
		if(x1 == x2 && y1 == y2) break;
		e2 = err;
		if(e2 > -dx) { err -= dy; x1 += sx; }
		if(e2 < dy) { err += dx; y1 += sy; }
	}

	return 0;
}

/*
 *  Draw a character at the screen.
 */
int 
bga_draw_character (int x, int y, char ch, int color)
{
    int i, j;
    int ord = ch - 0x20;
    if (ord < 0 || ord >= (BGA_CHARACTER_NUMBER - 1)) {
        return -1;
    }
//    acquireGUILock(buf);
    for (i = 0; i < BGA_CHARACTER_HEIGHT; i++) 
    {
        if (y + i > BGA_SCREEN_HEIGHT) {
            break;
        }

        if (y + i < 0) {
            continue;
        }

        for (j = 0; j < BGA_CHARACTER_WIDTH; j++) 
        {
            if (bga_character[ord][i][j] == 1) 
            {
                if (x + j > BGA_SCREEN_WIDTH) 
                {
                    break;
                }
                if (x + j < 0) 
                {
                    continue;
                }
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPointAlpha(t, color);
                bga_draw_pixel(x + j, y + i, color);
            }
        }
    }
    //releaseGUILock(buf);
    return BGA_CHARACTER_WIDTH;
}

int 
bga_draw_character_24 (int x, int y, char ch, RGBA color)
{
    int i, j;
    int ord = ch - 0x20;
    if (ord < 0 || ord >= (BGA_CHARACTER_NUMBER - 1)) {
        return -1;
    }
//    acquireGUILock(buf);
    for (i = 0; i < BGA_CHARACTER_HEIGHT; i++) 
    {
        if (y + i > BGA_SCREEN_HEIGHT) {
            break;
        }

        if (y + i < 0) {
            continue;
        }

        for (j = 0; j < BGA_CHARACTER_WIDTH; j++) 
        {
            if (bga_character[ord][i][j] == 1) 
            {
                if (x + j > BGA_SCREEN_WIDTH) 
                {
                    break;
                }
                if (x + j < 0) 
                {
                    continue;
                }
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPointAlpha(t, color);
                bga_draw_pixel_24(x + j, y + i, color);
            }
        }
    }
    //releaseGUILock(buf);
    return BGA_CHARACTER_WIDTH;
}

/*
 *  Draw a string at the screen.
 */
void 
bga_draw_string (int x, int y, char *str, int color)
{
    int offset_x = 0;

    while (*str != '\0')
    {
        offset_x += bga_draw_character(x + offset_x, y, *str, color);
        str++;
    }
}

void 
bga_draw_string_24 (int x, int y, char *str, RGBA color)
{
    int offset_x = 0;

    while (*str != '\0')
    {
        offset_x += bga_draw_character_24(x + offset_x, y, *str, color);
        str++;
    }
}

void 
bga_draw_string_with_max_width (int x, int y, int width, char *str, int color)
{
    int offset_x = 0;

    while (*str != '\0' && offset_x + BGA_CHARACTER_WIDTH <= width)
    {
        offset_x += bga_draw_character(x + offset_x, y, *str, color);
        str++;
    }
}

void 
bga_draw_string_with_max_width_24 (int x, int y, int width, char *str, RGBA color)
{
    int offset_x = 0;

    while (*str != '\0' && offset_x + BGA_CHARACTER_WIDTH <= width)
    {
        offset_x += bga_draw_character_24(x + offset_x, y, *str, color);
        str++;
    }
}

int 
bga_draw_icon (int x, int y, int icon, int color)
{
    int i, j;
//    int color2;

//    color2 = color.R;

    if (icon < 0 || icon > (BGA_ICON_NUMBER - 1))
    {
        return -1;
    }

    for (i = 0; i < BGA_ICON_SIZE; i++)
    {
        if (y + i > BGA_SCREEN_HEIGHT)
        {
            break;
        }
        if (y + i < 0)
        {
            continue;
        }
        for (j = 0; j < BGA_ICON_SIZE; j++)
        {
            if (bga_icons [icon] [i] [j] == 1)
            {
                if (x + j > BGA_SCREEN_WIDTH)
                {
                    break;
                }
                if (x + j < 0)
                {
                    continue;
                }
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPointAlpha(t, color);
                bga_draw_pixel(x + j, y + i, color);
            }
        }
    }
    return BGA_CHARACTER_WIDTH;
}

int 
bga_draw_icon_24 (int x, int y, int icon, RGBA color)
{
    int i, j;
//    int color2;

//    color2 = color.R;

    if (icon < 0 || icon > (BGA_ICON_NUMBER - 1))
    {
        return -1;
    }

    for (i = 0; i < BGA_ICON_SIZE; i++)
    {
        if (y + i > BGA_SCREEN_HEIGHT)
        {
            break;
        }
        if (y + i < 0)
        {
            continue;
        }
        for (j = 0; j < BGA_ICON_SIZE; j++)
        {
            if (bga_icons [icon] [i] [j] == 1)
            {
                if (x + j > BGA_SCREEN_WIDTH)
                {
                    break;
                }
                if (x + j < 0)
                {
                    continue;
                }
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPointAlpha(t, color);
                bga_draw_pixel_24(x + j, y + i, color);
            }
        }
    }
    return BGA_CHARACTER_WIDTH;
}

void 
bga_draw_mouse(int mode, int x, int y)
{
    int i, j;

    for (i = 0; i < BGA_MOUSE_HEIGHT; i++)
    {
        if (y + i >= BGA_SCREEN_HEIGHT)
        {
            break;
        }
        if (y + i < 0)
        {
            continue;
        }
        for (j = 0; j < BGA_MOUSE_WIDTH; j++)
        {
            if (x + j >= BGA_SCREEN_WIDTH)
            {
                break;
            }
            if (x + j < 0)
            {
                continue;
            }
            int temp = bga_mouse_pointer [mode] [i] [j];
            if (temp)
            {
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPoint(t, mouse_color[temp - 1]);
                bga_draw_pixel(x + j, (y + i), bga_mouse_color [temp - 1]);
            }
        }
    }
}

void 
bga_draw_mouse_24(int mode, int x, int y)
{
    int i, j;

    for (i = 0; i < BGA_MOUSE_HEIGHT; i++)
    {
        if (y + i >= BGA_SCREEN_HEIGHT)
        {
            break;
        }
        if (y + i < 0)
        {
            continue;
        }
        for (j = 0; j < BGA_MOUSE_WIDTH; j++)
        {
            if (x + j >= BGA_SCREEN_WIDTH)
            {
                break;
            }
            if (x + j < 0)
            {
                continue;
            }
            int temp = bga_mouse_pointer [mode] [i] [j];
            if (temp)
            {
                //t = buf + (y + i) * SCREEN_WIDTH + x + j;
                //drawPoint(t, mouse_color[temp - 1]);
                bga_draw_pixel_24(x + j, (y + i), bga_mouse_color_24 [temp - 1]);
            }
        }
    }
}


void 
bga_draw_rect_bound (int x, int y, int width, int height, int fill, int max_x, int max_y)
{
    int i, j;

    for (i = 0; i < height - 1; i++)
    {
     //   memset (bga_offscreen_buffer+((y + i) * 1024 + x), fill, width);
        if (y + i < 0)
            continue;
        if (y + i >= max_y)
            break;
        for (j = 0; j < width - 1; j++)
        {
            if (x + j < 0)
                continue;
            if (x + j >= max_x)
                break;
//            t = buf + (y + i) * SCREEN_WIDTH + x + j;
//            drawPointAlpha(t, fill);
             bga_draw_pixel(x + j, (y + i), fill);
        }
    }
}

void 
bga_draw_rect_bound_24 (int x, int y, int width, int height, RGBA fill, int max_x, int max_y)
{
    int i, j;
   // int color2 = fill.R; // get_nearest_color (fill.R, fill.G, fill.B);

    for (i = 0; i < height - 1; i++)
    {
        if (y + i < 0)
            continue;
        if (y + i >= max_y)
            break;
        for (j = 0; j < width - 1; j++)
        {
            if (x + j < 0)
                continue;
            if (x + j >= max_x)
                break;
//            t = buf + (y + i) * SCREEN_WIDTH + x + j;
//            drawPointAlpha(t, fill);
             bga_draw_pixel_24 (x + j, (y + i), fill);
        }
    }
}

void 
bga_draw_rect (int x, int y, int width, int height, int fill)
{
    bga_draw_rect_bound(x, y, width, height, fill, BGA_SCREEN_WIDTH, BGA_SCREEN_HEIGHT);
}

void 
bga_draw_rect_24(int x, int y, int width, int height, RGBA fill)
{
    bga_draw_rect_bound_24(x, y, width, height, fill, BGA_SCREEN_WIDTH, BGA_SCREEN_HEIGHT);
}

void 
bga_draw_rect_by_coord(int xmin, int ymin, int xmax, int ymax, int fill)
{
    bga_draw_rect (xmin, ymin, xmax - xmin, ymax - ymin, fill);
}

void 
bga_draw_rect_by_coord_24(int xmin, int ymin, int xmax, int ymax, RGBA fill)
{
    bga_draw_rect_24 (xmin, ymin, xmax - xmin, ymax - ymin, fill);
}

// void 
// clearRect (RGB *buf, RGB *temp_buf, int x, int y, int width, int height)
// {
//     RGB *t;
//     RGB *o;
//     int i;
//     int max_line = (SCREEN_WIDTH - x) < width ? (SCREEN_WIDTH - x) : width;
//     for (i = 0; i < height; i++)
//     {
//         if (y + i >= SCREEN_HEIGHT)
//         {
//             break;
//         }
//         if (y + i < 0)
//         {
//             continue;
//         }
//         // t = buf + (y + i) * SCREEN_WIDTH + x;
//         // o = temp_buf + (y + i) * SCREEN_WIDTH + x;
//         // memmove(t, o, max_line * 3);
//     }
// }

void 
draw24Image_24 (RGB *buf, RGB *img, int x, int y, int width, int height, int max_x, int max_y)
{
    int i;
    RGB *t;
    RGB *o;
    int max_line = (max_x - x) < width ? (max_x - x) : width;
    for (i = 0; i < height; i++)
    {
        if (y + i >= max_y)
        {
            break;
        }
        if (y + i < 0)
        {
            continue;
        }
        t = buf + (y + i) * BGA_SCREEN_WIDTH + x;
        o = img + (height - i) * width;
        memmove (t, o, max_line * 3);
    }
}


/*
 *  Draw inside of a window.
 */
//bga_draw_24_image_part (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, 
void 
bga_draw_24_image_part (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, 
    int subh, char *title)
{
    if (x >= BGA_SCREEN_WIDTH || y >= BGA_SCREEN_HEIGHT) {
        return;
    }

    int minj = x < 0 ? -x : 0;
    int maxj = x + subw > BGA_SCREEN_WIDTH ? BGA_SCREEN_WIDTH - x : subw;

    if (minj >= maxj) {
        cprintf("Wrong dimensions\n");
        return;
    }

    int i;
    
    for (i = 0; i < subh; i++)
    {
        if (y + i < 0) {
            continue;
        }
        if (y + i >= BGA_SCREEN_HEIGHT) {
            break;
        }

        for (int j=0; j < subw; j++)
        {
            //int color2 = img [j + (i * width)].R;
            int color2 = img [j + (i * width)].R;
            bga_draw_pixel (j + x, i + y, color2);
        }

//memmove (bga_offscreen_buffer+(y * 1024 + x), &img[0 + (i * width)].R ,  64 * 1024 );

        // t = buf + (y + i) * SCREEN_WIDTH + minj + x;
        // o = img + (i + suby) * width + subx + minj;
        // memmove(t, o, (maxj - minj) * 3);
    }
}

void 
bga_draw_24_image_part_24 (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, 
    int subh, char *title)
{
    if (x >= BGA_SCREEN_WIDTH || y >= BGA_SCREEN_HEIGHT) {
        return;
    }

    int minj = x < 0 ? -x : 0;
    int maxj = x + subw > BGA_SCREEN_WIDTH ? BGA_SCREEN_WIDTH - x : subw;

    if (minj >= maxj) {
        cprintf("Wrong dimensions\n");
        return;
    }

    int i;
    
    for (i = 0; i < subh; i++)
    {
        if (y + i < 0) {
            continue;
        }
        if (y + i >= BGA_SCREEN_HEIGHT) {
            break;
        }

        RGB *t = (RGB *)(bga_offscreen_buffer_24 + (((y + i) * 3) * BGA_SCREEN_WIDTH + (x * 3)));
        //RGB *o = img + ((y + i) * subw);
        RGB *o = img + (i + suby) * width + subx + minj;
        memmove (t, o, (maxj - minj) * 3);
    }
}

void 
bga_draw_rect_border (int color, int x, int y, int width, int height)
{
    if (x >= BGA_SCREEN_WIDTH || x + width < 0 || y >= BGA_SCREEN_HEIGHT || y + height < 0 || width < 0 || height < 0)
    {
        return;
    }

    int i;

    //int max_line = (SCREEN_WIDTH - x) < width ? (SCREEN_WIDTH - x) : width;
    //RGB *t = buf + y * SCREEN_WIDTH + x;

    if (y > 0)
    {
        //bga_draw_line (x, y, x + width, y, color);
        for (i = 0; i < width; i++)
        {
            if (x + i > 0 && x + i < BGA_SCREEN_WIDTH)
            {
                bga_draw_pixel(x + i, y, color);
                //*(t + i) = color;
            }
        }
    }

    if (y + height < BGA_SCREEN_HEIGHT)
    {
//        bga_draw_line (x, y + height, x + width, y + height, color);

        //RGB *o = t + height * SCREEN_WIDTH;
        for (i = 0; i < width; i++)
        {
            if (y > 0 && x + i > 0 && x + i < BGA_SCREEN_WIDTH)
            {
                bga_draw_pixel(x + i, y + height, color);
                //*(o + i) = color;
            }
        }
    }

    if (x > 0)
    {
        for (i = 0; i < height; i++)
        {
            if (y + i > 0 && y + i < BGA_SCREEN_HEIGHT)
            {
                //*(t + i * SCREEN_WIDTH) = color;
                bga_draw_pixel(x, y + i, color);
            }
        }
    }

    if (x + width < BGA_SCREEN_WIDTH)
    {
        //RGB *o = t + width;
        for (i = 0; i < height; i++)
        {
            if (y + i > 0 && y + i < BGA_SCREEN_HEIGHT)
            {
                bga_draw_pixel(x + width, y + i, color);
                //*(o + i * SCREEN_WIDTH) = color;
            }
        }
    }
}


void 
bga_draw_rect_border_24 (RGBA color, int x, int y, int width, int height)
{
    if (x >= BGA_SCREEN_WIDTH || x + width < 0 || y >= BGA_SCREEN_HEIGHT || y + height < 0 || width < 0 || height < 0)
    {
        return;
    }

    int i;

    //int max_line = (SCREEN_WIDTH - x) < width ? (SCREEN_WIDTH - x) : width;
    //RGB *t = buf + y * SCREEN_WIDTH + x;

    if (y > 0)
    {
        //bga_draw_line (x, y, x + width, y, color);
        for (i = 0; i < width; i++)
        {
            if (x + i > 0 && x + i < BGA_SCREEN_WIDTH)
            {
                bga_draw_pixel_24(x + i, y, color);
                //*(t + i) = color;
            }
        }
    }

    if (y + height < BGA_SCREEN_HEIGHT)
    {
//        bga_draw_line (x, y + height, x + width, y + height, color);

        //RGB *o = t + height * SCREEN_WIDTH;
        for (i = 0; i < width; i++)
        {
            if (y > 0 && x + i > 0 && x + i < BGA_SCREEN_WIDTH)
            {
                bga_draw_pixel_24(x + i, y + height, color);
                //*(o + i) = color;
            }
        }
    }

    if (x > 0)
    {
        for (i = 0; i < height; i++)
        {
            if (y + i > 0 && y + i < BGA_SCREEN_HEIGHT)
            {
                //*(t + i * SCREEN_WIDTH) = color;
                bga_draw_pixel_24(x, y + i, color);
            }
        }
    }

    if (x + width < BGA_SCREEN_WIDTH)
    {
        //RGB *o = t + width;
        for (i = 0; i < height; i++)
        {
            if (y + i > 0 && y + i < BGA_SCREEN_HEIGHT)
            {
                bga_draw_pixel_24(x + width, y + i, color);
                //*(o + i * SCREEN_WIDTH) = color;
            }
        }
    }
}

#define BGA_INPUT_BUF 128
struct {
	char buf[BGA_INPUT_BUF];
	uint r;	//Read index
	uint w;	//Write index
	uint e;	//Edit index
} bga_input;

int 
bga_getkey (void) 
{
	int c;
	if(bga_input.r == bga_input.e)
		return -1;
    c = bga_input.buf [bga_input.r++ % BGA_INPUT_BUF];
    
    return c;
}

void 
bga_graphicsintr (int (*getc) (void)) 
{
	int c;
	
	acquire (&bga_sl);
	
	while ((c = getc ()) >= 0) {
		if(c != 0 && bga_input.e-bga_input.r < BGA_INPUT_BUF) {
			bga_input.buf[bga_input.e++ % BGA_INPUT_BUF] = c;
		}
	}
	
	release (&bga_sl);

}
