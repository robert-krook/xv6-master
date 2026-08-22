
/*
 *  kbc.c --    keyboard driver.
 */

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "kbd.h"

#include "msg.h"

#include "gui/gui.h"
#include "bga.h"

int
kbdgetc (void)
{
    static uint shift;

    static uchar *charcode[4] = {
        normalmap, shiftmap, ctlmap, ctlmap
    };

    uint st, data, c;

    st = inb(KBSTATP);

    if((st & KBS_DIB) == 0)
        return -1;

    data = inb(KBDATAP);

    message msg;

    if(data == 0xE0) {

        shift |= E0ESC;
        return 0;

    }  else if(data & 0x80) {

        // Key released
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shiftcode[data] | E0ESC);

        c = normalmap[data];
        msg.msg_type = M_KEY_UP;
        msg.params[0] = c;
        msg.params[1] = shift;
        handleMessage(&msg);

        return 0;

    } else if(shift & E0ESC) {
        // Last character was an E0 escape; or with 0x80
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shiftcode[data];
    shift ^= togglecode[data];

    // Send keystroke to window manager and applications
    msg.msg_type = M_KEY_DOWN;

    if (shift==1)
        msg.params[0] = shiftmap [data];
    else
        msg.params[0] = normalmap [data];

    msg.params[1] = shift;
    
    handleMessage (&msg);

    c = charcode[shift & (CTL | SHIFT)][data];
    if (shift & CAPSLOCK) {
        if('a' <= c && c <= 'z')
            c += 'A' - 'a';
        else if('A' <= c && c <= 'Z')
            c += 'a' - 'A';
    }

    return c;
}

void
kbdintr (void)
{
    //consoleintr(kbdgetc);
    if (is_graphics() == 1)
#ifdef __VGA__    
		vga_graphicsintr (kbdgetc);
#else
        bga_graphicsintr (kbdgetc);
#endif
	else
  		consoleintr(kbdgetc);
}
