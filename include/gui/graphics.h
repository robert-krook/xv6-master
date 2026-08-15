#ifndef GUI_H
#define GUI_H

#ifndef __ASSEMBLER__

enum vga {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_LIGHT_BROWN = 14,
    VGA_WHITE = 15,
};

//extern int SCREEN_WIDTH;
//extern int SCREEN_HEIGHT;
//extern int screen_size;

// 24 bit RGB. used in GUI Utility
typedef struct RGB {
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGB;

// 32 bit RGBA. used above GUI Utility
typedef struct RGBA {
    unsigned char A;
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGBA;

#endif
#endif
