/*
 *  gui.c --    our gui parent library.
 */

#include "types.h"

#include "gui/graphics.h"
#include "bga.h"

#include "gui/amiga/amiga.h"

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

/*
 *  gui_init_graphics -- initialize the graphics mode.
 */
void 
gui_init_graphics ()
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
