

#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "memlayout.h"
#include "gui/user_window.h"
#include "user.h"
#include "msg.h"

#include "date.h"

window programWindow;

char filename[40];

int file = -1;

int lastMaximumOffset = 0;

void 
buttonHandler (Widget *widget, message *msg)
{
    if (msg->msg_type == M_MOUSE_LEFT_CLICK || msg->msg_type == M_MOUSE_DBCLICK)
    {
        // if (file != -1)
        // {
        //     file = open(filename, O_RDWR);
        // }
        // else
        // {
        //     file = open("/new.txt", O_CREATE);
        // }

        // int i;
        // for (i = programWindow.widgetlisthead; i != -1; i = programWindow.widgets[i].next)
        // {
        //     if (programWindow.widgets[i].type == INPUTFIELD)
        //     {
        //         break;
        //     }
        // }
        // write(file, programWindow.widgets[i].context.inputfield->text, 512);
        // close(file);
    }
}

// void 
// inputHandler(Widget *w, message *msg)
// {

//     int width = w->position.xmax - w->position.xmin;
//     int height = w->position.ymax - w->position.ymin;

//     if (msg->msg_type == M_MOUSE_LEFT_CLICK)
//     {
//         inputMouseLeftClickHandler(w, msg);
//     }
//     else if (msg->msg_type == M_KEY_DOWN)
//     {
//         inputFieldKeyHandler(w, msg);

//         //MODIFY THE HEIGHT OF THE INPUTFIELD
//         int newHeight = CHARACTER_HEIGHT * (getMouseYFromOffset(w->context.inputfield->text, width, strlen(w->context.inputfield->text)) + 1);
//         if (newHeight > height)
//         {
//             w->position.ymax = w->position.ymin + newHeight;
//         }

//         //MODIFY THE SCROLL OFFSET IF IT CHANGES
//         int maximumOffset = getScrollableTotalHeight(&programWindow) - programWindow.height;
//         if (maximumOffset > 0 && lastMaximumOffset != maximumOffset)
//         {
//             programWindow.scrollOffsetY = maximumOffset;
//             lastMaximumOffset = maximumOffset;
//         }

//         int currentHeight = getMouseYFromOffset(w->context.inputfield->text, width, w->context.inputfield->current_pos) * CHARACTER_HEIGHT+ w->position.ymin - programWindow.scrollOffsetY;

//         if (currentHeight <= w->position.ymin && programWindow.scrollOffsetY>0)
//             programWindow.scrollOffsetY -= CHARACTER_HEIGHT;
//         if (currentHeight >= programWindow.height-1 && programWindow.scrollOffsetY<=maximumOffset)
//             programWindow.scrollOffsetY += CHARACTER_HEIGHT;
            
//     }
// }

int 
main (int argc, char *argv[])
{

    struct RGBA bgColor;
//    struct RGBA textColor;

    programWindow.width = 200;
    programWindow.height = 100;
    programWindow.hasTitleBar = 1;

    create_window (&programWindow, "clock");

    bgColor.R = 89;
    bgColor.G = 101;
    bgColor.B = 186;
    bgColor.A = 255;
#if defined (__VGA__) || defined (__BGA__)
    bgColor.R = get_nearest_color (bgColor.R, bgColor.G, bgColor.B);
#endif

    addColorFillWidget(&programWindow, bgColor, 0, 0, programWindow.width, 

    programWindow.height, 0, buttonHandler, "editor");

//     textColor.R = 255;
//     textColor.G = 255;
//     textColor.B = 255;
//     textColor.A = 255;
// #ifdef __VGA__ && __BGA__        
//     textColor.R = get_nearest_color (textColor.R, textColor.G, textColor.B);
// #endif

    struct RGBA buttonColor;
    buttonColor.R = 255;
    buttonColor.G = 255;
    buttonColor.B = 255;
    buttonColor.A = 255;
#if defined (__VGA__) || defined (__BGA__)
    buttonColor.R = get_nearest_color (buttonColor.R, buttonColor.G, buttonColor.B);
#endif
    
    int i = 0;

    int widget_id = addTextWidget (&programWindow, buttonColor, "", 20, 40, 190, 100, 0, emptyHandler);


    while (1) {
        char buf [40];

        memset (buf, 0, sizeof (buf));

        struct cmos_time_struct r;
        date (&r);

        sprintf (buf, "%d-%d-%d %d:%d:%d", r.year, r.month, r.day, r.hour, r.min, r.sec);

        strcpy ((char *) programWindow.widgets [widget_id].context.text->text, buf);

        sleep (50);

        i++;

        updateWindow (&programWindow);

        programWindow.needsRepaint = 1;
    }
}
