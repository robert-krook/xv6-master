
#include "types.h"

#include "gui/graphics.h"
#include "bga.h"

#include "gui/amiga/amiga.h"

// int
// sys_init_graphics (void)
// {

//     mouse_color[0] = 0;
//     mouse_color[1] = 15;

// 	write_regs(g_640x480x16);
//         graphics_mode = 1;
        
// 	//draw a rectangle of black pixels
// 	if(vga_clear_screen() != 0)
//         return -1;

// 	//Initialize the spinlock
// 	initlock(&sl, "spin");

// 	return 0; 
// }

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

void gui_init_graphics ()
{
    amiga_init ();
#ifdef __VGA__
        vga_init_graphics ();
        SCREEN_WIDTH = 640;
        SCREEN_HEIGHT = 480;
#elifdef __BGA__
        SCREEN_WIDTH = 1024;
        SCREEN_HEIGHT = 768;
        bga_init ();
#else
        SCREEN_WIDTH = 1024;
        SCREEN_HEIGHT = 768;
        bga_init_24 ();

#endif
}
