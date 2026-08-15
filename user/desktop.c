
#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "memlayout.h"

#include "gui/user_window.h"
#include "gui/graphics.h"
#include "gui/bitmap.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

window desktop;

struct RGBA desktopColor;
struct RGBA buttonColor;
struct RGBA textColor;
struct RGBA border_color;

char * GUI_programs[] = {"shell", "editor", "explorer", "clock"};

window programWindow;

int start_button = -1;

void 
startProgramHandler (Widget *widget, message *msg)
{
    printf (0, "msg\n");
    if (msg->msg_type == M_MOUSE_DBCLICK) {

        int pid = fork ();
printf (0, "my pid = %d\n", pid);

        if (pid == 0) {
            char *argv2[] = {widget->context.button->text, 0};
            exec (argv2[0], argv2);
            exit();
        }
    }
}

void 
startWindowHandler (Widget *widget, message *msg)
{
    int mouse_x = msg->params[0];
    int mouse_y = msg->params[1];

    printf (0, "%d\n", msg->msg_type);

if (GUI_get_style()==2) {
    if (msg->msg_type == M_MOUSE_LEFT_CLICK && mouse_x < START_ICON_WIDTH && mouse_y < DOCK_HEIGHT) {
        if (fork() == 0) {
            printf (0, "Start window\n");
            char *argv2[] = {"/bin/startWindow", (char *) desktop.handler, 0};
            exec (argv2[0], argv2);
            exit ();
        }
    }
} else {

    if (msg->msg_type == M_MOUSE_LEFT_CLICK && mouse_x < START_ICON_WIDTH && mouse_y > SCREEN_HEIGHT - DOCK_HEIGHT) {
        if (fork() == 0) {
            printf (0, "Start window\n");
            char *argv2[] = {"/bin/startWindow", (char *) desktop.handler, 0};
            exec (argv2[0], argv2);
            exit ();
        }
    }
}

    if (msg->msg_type == M_MOUSE_LEFT_CLICK) {

        if (widget->type==IMAGE) {
            if (!strcmp (widget->name, "imgClock")) {
                int pid = fork ();
                printf (0, "my pid = %d\n", pid);
                if (pid == 0) {

chpr (pid, 1);

                    char *argv2[] = {"/bin/clock", (char *) desktop.handler, 0};
                    exec (argv2[0], argv2);
                    exit ();
                }
            }
        }

        if (widget->type==IMAGE) {
            if (!strcmp (widget->name, "imgCli")) {
                if (fork() == 0) {
                    char *argv2[] = {"/bin/shell", (char *) desktop.handler, 0};
                    exec (argv2[0], argv2);
                    exit ();
                }
            }
        }
    }

    if (msg->msg_type == M_MOUSE_LEFT_CLICK) {

        if (widget->type==IMAGE) {
            if (!strcmp (widget->name, "imgExplorer") || !strcmp (widget->name, "imgClick")) {
                if (fork() == 0) {
                    char *argv2[] = {"/bin/explorer", (char *) desktop.handler, 0};
                    exec (argv2[0], argv2);
                    exit ();
                }
            }
        }
    }

}

void 
desktopWindowHandler (Widget *widget, message *msg)
{
    printf (0, "desktop = %d\n", msg->msg_type);

    if (msg->msg_type == WM_WINDOW_INVALIDATE) {

        if (GUI_get_style()==2) {
            desktop.widgets [start_button].position.ymin = 0;
            desktop.widgets [start_button].position.ymax = DOCK_HEIGHT - 2;
        } else {
            desktop.widgets [start_button].position.ymin = SCREEN_HEIGHT - 36;
            desktop.widgets [start_button].position.ymax =  SCREEN_HEIGHT;    
        }

    }

}

int main (int argc, char *argv[])
{

    // Start VGA graphics mode
    init_graphics ();

    // Buildup the desktop screen
    desktop.width = SCREEN_WIDTH;
    desktop.height = SCREEN_HEIGHT;

    desktop.initialPosition.xmin = 0;
    desktop.initialPosition.xmax = SCREEN_WIDTH;
    desktop.initialPosition.ymin = 0;
    desktop.initialPosition.ymax = SCREEN_HEIGHT;
    desktop.hasTitleBar = 0;

    desktop.wnd_handler = desktopWindowHandler;

    create_window (&desktop, "desktop");

    desktopColor.R = 89;
    desktopColor.G = 101;
    desktopColor.B = 186;
    desktopColor.A = 255;
#ifdef __VGA__  && __BGA__    
    desktopColor.R = get_nearest_color (desktopColor.R, desktopColor.G, desktopColor.B);
#endif

    addColorFillWidget (&desktop, desktopColor, 0, 0, desktop.width, desktop.height, 
        0, emptyHandler, "desktop");

    buttonColor.R = 85;
    buttonColor.G = 85;
    buttonColor.B = 255;
    buttonColor.A = 255;
#ifdef __VGA__  && __BGA__    
    buttonColor.R = get_nearest_color (buttonColor.R, buttonColor.G, buttonColor.B);
#endif

    textColor.R = 255;
    textColor.G = 255;
    textColor.B = 255;
    textColor.A = 255;
#ifdef __VGA__  && __BGA__    
    textColor.R = get_nearest_color (textColor.R, textColor.G, textColor.B);
#endif

    border_color.R = 255;
    border_color.G = 255;
    border_color.B = 255;
    border_color.A = 255;
#ifdef __VGA__  && __BGA__    
    border_color.R = get_nearest_color (border_color.R, border_color.G, border_color.B);
#endif

    // for(int i=0; i<4; i++) 
    // {
    //    addButtonWidget (&desktop, textColor, buttonColor, border_color, GUI_programs[i], 
    //     5, 20 + 50*i, 80, 30, 0, startProgramHandler); 
    // }

    start_button = addButtonWidget (&desktop, textColor, buttonColor, border_color, "start", 5, 
        SCREEN_HEIGHT - 36, 72, 36, 0, startWindowHandler);

    RGB * background = malloc(32 * 32 * 3);
    int h, w;
    char filename[] = "/usr/share/icons/test.bmp";

    read24BitmapFile (filename, background, &h, &w);

    RGB * clock = malloc(41 * 36 * 3);
    int h2, w2;
    char filename_clock[] = "/usr/share/icons/clock.bmp";

    read24BitmapFile (filename_clock, clock, &h2, &w2);

    RGB * cli = malloc(52 * 34 * 3);
    int h3, w3;
    char filename_cli[] = "/usr/share/icons/cli.bmp";

    read24BitmapFile (filename_cli, cli, &h3, &w3);

    int lastTime = 0;

    addImageWidget (&desktop, background, SCREEN_WIDTH - 60, 50, 32, 32, startWindowHandler, "imgClick");

    addImageWidget (&desktop, clock, SCREEN_WIDTH - 60, 100, 41, 36, startWindowHandler, "imgClock");

    addImageWidget (&desktop, cli, SCREEN_WIDTH - 80, 150, 52, 34, startWindowHandler, "imgCli");

    add_image (&desktop, "/usr/share/icons/explorer.bmp", "imgExplorer", SCREEN_WIDTH - 80, 200, 
        startProgramHandler, &w, &h);

// int xx = 10;
// int yy = 10;

    while (1) {

    //    draw24Image (&desktop, cli, xx, yy, 52, 34);

        updateWindow (&desktop);

        int currentTime = uptime ();

        if (currentTime - lastTime >= 1)
        {
            // if (xx > 1000) 
            // xx = 10;
            // else
            // xx+=10;

            // if (yy > 700) 
            // yy= 10;
            // else
            // yy+=10;

            GUI_updateScreen ();
            
            lastTime = currentTime;
        }
    }

}
