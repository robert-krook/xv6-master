
/*
 *  amiga.c -- amiga gui version.
 *
 *  Desktop GUI styling.
 *     
 */

#include "types.h"
#include "x86.h"
#include "param.h"
#include "defs.h"
#include "msg.h"
#include "spinlock.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#include "gui/graphics.h"
#include "bga.h"

#include "gui/gui.h"

#include "gui/user_window.h"
#include "gui/window_manager.h"

#include "stdio.h"
#include "fcntl.h"
#include "libc/stdlib.h"


struct RGBA amiga_iconColor;
struct RGBA amiga_titleBarColor;
struct RGBA amiga_dockColor;
struct RGBA amiga_closeColor;
struct RGBA amiga_txtColor;
struct RGBA amiga_minimizeColor;

struct subsection_display {
    int depth;
    uchar modes     [16];
    uchar virtual   [16];
} sub_display;

struct screen {
    int default_depth;
    struct subsection_display subsection_display;
} screen;

struct device {
    uchar driver        [24];
    uchar vendor_name   [64];
} device;

struct desktop {
    uchar name [16];
    uchar dock_position [8];
    uchar bg_color      [16];
    uchar text_color    [16];
} desktop;

struct _window {
    uchar color         [16];
    uchar text_color    [16];
} _window;

struct screen screen;
struct device device;
struct desktop desktop;
struct _window _window;

int
config_split (char *line, int n, char *key, char *value)
{
    int part = 0;   // We start with the key
    int j = 0, i = 0;

    while (i<n && line [i]==' ') {
        i++;
        continue;
    }

    for (i; i<n; i++) {
        if (line [i]==' ') {
            if (part==0) {
                part = 1;
                key [j] = 0;
                j = 0;
                continue;
            } else {
                value [j] = 0;
                return 0;
            }
        }
        if (part==0)
            key [j++] = line [i];
        else
            value [j++] = line [i];
    }
    return -1;
}

/*
 *  amiga_read_config -- read the amiga configuration file.
 */
int
amiga_read_config ()
{
    char buffer [128];
    int n = 0;
    int section = 0;
    
return 1;

    int fd = fopen ("/etc/amiga.cfg", O_RDONLY);
    if (fd<0) {
        cprintf ("Unable to open file\n");
    }

    cprintf ("read the config file of the amiga\n");

    while (n=fgetline (fd, buffer, sizeof (buffer))>=0) {

        if (n>0) {

            uchar key [128];
            uchar value [128];

            config_split (buffer, sizeof (buffer), key, value);
            cprintf ("data = %s, key = %s, value = %s\n", buffer, key, value);

            if (!strncmp (key, "Section", 7) && !strncmp (value, "\"Screen\"", 8)) {
                section = 1;
                cprintf ("%s\n", "Section Screen");
                continue;
            }

            if (!strncmp (key, "Section", 7) && !strncmp (value, "\"Device\"", 8)) {
                section = 2;
                cprintf ("%s\n", "Section Device");
                continue;    
            }

            if (!strncmp (key, "Section", 7) && !strncmp (value, "\"Desktop\"", 9)) {
                section = 3;
                cprintf ("%s\n", "Section Desktop");
                continue;
            }

            if (!strncmp (key, "Section", 7) && !strncmp (value, "\"Window\"", 8)) {
                section = 4;
                cprintf ("%s\n", "Section Window");
                continue;
            }

            if (section == 1) {
                if (!strncmp (key, "DefaultDepth", sizeof ("DefaultDepth")))
                    screen.default_depth = atoi (value);

                if (!strncmp (key, "SubSection", sizeof ("SubSection")))
                    continue;   // Skip for now

                if (!strncmp (key, "Depth", sizeof ("Depth")))
                    screen.subsection_display.depth = atoi (value);

                if (!strncmp (key, "Depth", sizeof ("Depth")))
                    screen.subsection_display.depth = atoi (value);

                if (!strncmp (key, "Modes", sizeof ("Modes")))
                    strcpy (screen.subsection_display.modes, value);

                if (!strncmp (key, "Virtual", sizeof ("Virtual")))
                    strcpy (screen.subsection_display.virtual, value);
            }

            if (section == 2) {
                if (!strncmp (key, "Driver", sizeof ("Driver")))
                     strcpy (device.driver, value);

                if (!strncmp (key, "VendorName", sizeof ("VendorName")))
                     strcpy (device.vendor_name, value);
            }

            if (section == 3) {
                if (!strncmp (key, "Name", sizeof ("Name")))
                     strcpy (desktop.name, value);

                if (!strncmp (key, "DockPosition", sizeof ("DockPosition")))
                     strcpy (desktop.dock_position, value);

                if (!strncmp (key, "BgColor", sizeof ("BgColor")))
                     strcpy (desktop.bg_color, value);

                if (!strncmp (key, "TextColor", sizeof ("TextColor")))
                     strcpy (desktop.text_color, value);
            }

            if (section == 4) {
                if (!strncmp (key, "Color", sizeof ("Color")))
                     strcpy (_window.color, value);

                if (!strncmp (key, "TextColor", sizeof ("TextColor")))
                     strcpy (_window.text_color, value);
            }

        }
    }

    fclose (fd);

}

void
show_config ()
{
    cprintf ("Default screen depth %d\n", screen.default_depth);
}

int
amiga_init ()
{
    amiga_read_config ();

    show_config ();

    amiga_dockColor.R = 255;
    amiga_dockColor.G = 255;
    amiga_dockColor.B = 255;
    amiga_dockColor.A = 255;

	amiga_titleBarColor.R = 255;
	amiga_titleBarColor.G = 255;
	amiga_titleBarColor.B = 255;
	amiga_titleBarColor.A = 255;

	amiga_closeColor.R = 255;
	amiga_closeColor.G = 255;
	amiga_closeColor.B = 255;
	amiga_closeColor.A = 255;

	amiga_iconColor.R = 85;
	amiga_iconColor.G = 85;
	amiga_iconColor.B = 255;
	amiga_iconColor.A = 255;

	amiga_txtColor.R = 0;
    amiga_txtColor.G = 0;
    amiga_txtColor.B = 0;
    amiga_txtColor.A = 255;

	amiga_minimizeColor.R = 255;
    amiga_minimizeColor.G = 255;
    amiga_minimizeColor.B = 255;
	amiga_minimizeColor.A = 255;

    return 0;
}

int
amiga_desktop_draw_dock ()
{
#ifdef __VGA__
	vga_draw_rect_by_coord (0, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, amiga_dockColor);
#elifdef __BGA__
	bga_draw_rect_by_coord (0, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, amiga_dockColor.R);
#else
	bga_draw_rect_by_coord_24 (0, 0, SCREEN_WIDTH, DOCK_HEIGHT, amiga_dockColor);
#endif
    return 0;
}

int
amiga_desktop_draw_title_bar ()
{
#ifdef __VGA__
	vga_draw_rect_by_coord(0, SCREEN_HEIGHT - DOCK_HEIGHT, START_ICON_WIDTH, SCREEN_HEIGHT, amiga_titleBarColor);
#elifdef __BGA__
    bga_draw_rect_by_coord(0, SCREEN_HEIGHT - DOCK_HEIGHT, START_ICON_WIDTH, SCREEN_HEIGHT, amiga_titleBarColor.R);
#else
    bga_draw_rect_by_coord_24(0, 0, START_ICON_WIDTH, DOCK_HEIGHT, amiga_titleBarColor);
#endif
    return 0;
}

int 
amiga_desktop_draw_active_applications (uchar *title, int barWidth, int xStart)
{
#ifdef __VGA__
    vga_draw_string_with_max_width(xStart + 5, SCREEN_HEIGHT - DOCK_HEIGHT, 
        barWidth - 2, windowlist[p].wnd.title, txtColor);
    vga_draw_rect_by_coord(xStart + barWidth - 2, SCREEN_HEIGHT - DOCK_HEIGHT, 
        xStart + barWidth, SCREEN_HEIGHT, txtColor);
	    xStart += barWidth;
#elifdef __BGA__
	bga_draw_string_with_max_width(xStart + 5, SCREEN_HEIGHT - DOCK_HEIGHT, 
        barWidth - 2, windowlist[p].wnd.title, txtColor.R);
	bga_draw_rect_by_coord(xStart + barWidth - 2, SCREEN_HEIGHT - DOCK_HEIGHT, 
        xStart + barWidth, SCREEN_HEIGHT, txtColor.R);
    	xStart += barWidth;
#else
    bga_draw_string_with_max_width_24 (xStart + 5, 0, barWidth - 2, title, amiga_txtColor);
	bga_draw_rect_by_coord_24 (xStart + barWidth - 2, 0, xStart + barWidth, DOCK_HEIGHT, amiga_txtColor);
	xStart += barWidth;
#endif
}

int
amiga_desktop_draw_start_icon ()
{
#ifdef __VGA__
    vga_draw_rect_by_coord (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, titleBarColor);
	vga_draw_icon (START_ICON_WIDTH / 2 - 15, SCREEN_HEIGHT - DOCK_HEIGHT + 3, 2, iconColor);
#elifdef __BGA__
    bga_draw_rect_by_coord (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, titleBarColor.R);
	bga_draw_icon (START_ICON_WIDTH / 2 - 15, SCREEN_HEIGHT - DOCK_HEIGHT + 3, 2, iconColor.R);
#else
    bga_draw_rect_by_coord_24 (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, 0, SCREEN_WIDTH, DOCK_HEIGHT, amiga_titleBarColor);
	bga_draw_icon_24 (START_ICON_WIDTH / 2 - 15, 0 + 3, 2, amiga_iconColor);
#endif
    return 0;
}

// int
// amiga_draw_desktop_dock (int windowCount)
// {

// 	int p;
// //	int windowCount = getWindowCount ();



// 	if (windowCount > 0)
// 	{
// 		int xStart = START_ICON_WIDTH + 5;

// 		int barWidth = min((SCREEN_WIDTH - START_ICON_WIDTH - SHOW_DESKTOP_ICON_WIDTH) / (windowCount + 1), 
//             DOCK_PROGRAM_NORMAL_WIDTH);

// 		for (p = windowlisthead; p != -1; p = windowlist[p].next)
// 		{
// 			if (p != desktopId)
// 			{


// 			}
// 		}
// 	}



//     return 0;
// }

int amiga_draw_window ()
{
    return 1;
}

int amiga_draw_mouse ()
{
    return 0;
}
