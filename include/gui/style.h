
/*
 *  style.h -- GUI defined style. Read from theme style files.
 */

#include "graphics.h"

struct style {

//  Default window styles
    RGBA *color_window;             // Color of the window
    RGBA *color_window_text;        // Color of a text     
    RGBA *color_window_border;      // Color of the window border

//  Default button styles
    RGBA *color_button;             // Color of a button
    RGBA *color_button_text;        // Color of the text in the button
    RGBA *color_button_border;      // Color of the border of the button

//  Desktop menu styles
    int start_button;               // Is there a start button
    

//  Custom styles

} style;
