#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "memlayout.h"
#include "gui/user_window.h"
#include "gui/graphics.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

window startWindow;

char *GUI_programs[] = {"Amiga", "Alpha", "DECWindows", "NeXTSTEP", "TRON"};

void startProgramHandler(Widget *widget, message *msg)
{
    //if (msg->msg_type == M_MOUSE_DBCLICK)
    if (msg->msg_type == M_MOUSE_LEFT_CLICK) {

        printf (0, "Call select style %s\n", widget->name);

        if (!strncmp (widget->name, "Amiga", 5)) {
            GUI_select_style (2);
        } else {
            GUI_select_style (1);
        }

        closePopupWindow (&startWindow);

    }
}

int 
main (int argc, char *argv [])
{
    RGBA border_color;
    RGBA window_color;

    int caller = (int)argv[1];
    
    startWindow.width = 2 * START_ICON_WIDTH;
    startWindow.initialPosition.xmin = 25;
    startWindow.initialPosition.xmax = startWindow.width + 25;

    if (GUI_get_style()==2) {
        startWindow.height = 200 - DOCK_HEIGHT;
        startWindow.initialPosition.ymin = DOCK_HEIGHT;
        startWindow.initialPosition.ymax = 200;
    } else {
        startWindow.height = SCREEN_HEIGHT / 2;
        startWindow.initialPosition.ymin = SCREEN_HEIGHT - DOCK_HEIGHT - startWindow.height;
        startWindow.initialPosition.ymax = SCREEN_HEIGHT - DOCK_HEIGHT;
    }

    startWindow.hasTitleBar = 0;

    createPopupWindow (&startWindow, caller);

    if (GUI_get_style()==2) {
        window_color.R = 255;
        window_color.G = 255;
        window_color.B = 255;
        window_color.A = 255;
        addColorFillWidget (&startWindow, window_color, 0, 0, startWindow.width, startWindow.height, 0, emptyHandler, "");
    } else {
    }

    struct RGBA buttonColor;
    struct RGBA textColor;

if (GUI_get_style()==2) {
    buttonColor.R = 255;
    buttonColor.G = 255;
    buttonColor.B = 255;
    buttonColor.A = 255;
} else {
    buttonColor.R = 244;
    buttonColor.G = 180;
    buttonColor.B = 0;
    buttonColor.A = 255;
}
    //buttonColor.R = get_nearest_color (buttonColor.R, buttonColor.G, buttonColor.B);

if (GUI_get_style()==2) {
    textColor.R = 89;
    textColor.G = 101;
    textColor.B = 186;
    textColor.A = 255;
} else {
    textColor.R = 0;
    textColor.G = 0;
    textColor.B = 0;
    textColor.A = 255;
}
    //textColor.R = get_nearest_color (textColor.R, textColor.G, textColor.B);

if (GUI_get_style()==2) {
    border_color.R = 255;
    border_color.G = 255;
    border_color.B = 255;
    border_color.A = 255;

}
else {
    border_color.R = 0;
    border_color.G = 0;
    border_color.B = 0;
    border_color.A = 255;
}
    //border_color.R = get_nearest_color (border_color.R, border_color.G, border_color.B);

    for (int i = 0; i < 5; i++) {
        addButtonWidget (&startWindow, textColor, buttonColor, border_color, 
            GUI_programs [i], 5, 5 + 30 * i, 100, 30, 0, startProgramHandler);
    }

    while (1) {
        updatePopupWindow (&startWindow);
    }

}