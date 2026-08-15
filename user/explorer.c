
/*
 *  explorer -- for starting files etc.
 */

#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "memlayout.h"
#include "gui/user_window.h"
#include "user.h"
#include "msg.h"
#include "fs.h"
#include "stat.h"

window desktop;
char current_path[MAX_LONG_STRLEN];
int current_path_widget;
char newDir[MAX_SHORT_STRLEN];
char buf[MAX_LONG_STRLEN];
struct RGBA textColor;
struct RGBA dirColor;

int statusBarHeight = 50;

char *GUI_programs[] = {"shell", "editor", "explorer", "demo", "clock"};

void gui_ls(char *path);

char * 
getFileExtension (char *filename)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last .
    for (p = filename + strlen(filename); p >= filename && *p != '.'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), '\0', 1);
    //memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

int 
isOpenable(char *filename) 
{
    int isOpenable=0;
    for(int i=0; i<5; i++) {
       if(strcmp(filename, GUI_programs[i])==0) isOpenable=1;
    }
    return isOpenable;
}

char *
getparentpath(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    //p++;

    memmove(buf, path, p - path);
    buf[p - path] = '\0';
    return buf;
}

void 
mkdirHandler (Widget *widget, message *msg)
{
    if (msg->msg_type == M_MOUSE_DBCLICK)
    {
        memset(newDir, 0, MAX_SHORT_STRLEN);
        strcpy(newDir, current_path);
        memset(newDir + strlen(current_path), '/', 1);
        strcpy(newDir + strlen(current_path) + 1, "temp");


        if (fork() == 0)
        {
            //even if I put "temp" here, the error persists
            char *argv2[] = {"mkdir", newDir, 0};
            exec(argv2[0], argv2);
            exit();
        }
        wait();
        gui_ls(current_path);
    }
}

void 
backHandler(Widget *widget, message *msg)
{
    if (msg->msg_type == M_MOUSE_DBCLICK)
    {
        strcpy(current_path, getparentpath(current_path));
        gui_ls(current_path);
    }
}

void 
cdHandler (Widget *widget, message *msg)
{
    if (msg->msg_type == M_MOUSE_LEFT_CLICK)
    {
        if (fork() == 0) {
            int current_path_length = strlen(current_path);
            current_path[current_path_length] = '/';
        //strcpy(current_path + current_path_length + 1, widget->context.text->text);
            strcpy(current_path + current_path_length + 1, widget->name);

            printf (0, "%s", current_path);

        //gui_ls (current_path);
            //char *argv2[] = {"/bin/explorer", current_path, 0};
            char *argv2 [] = {"/bin/explorer", current_path, 0};
            exec (argv2[0], argv2);
            exit ();
        }

    }
}

void 
buttonHandler (Widget *widget, message *msg)
{
    if (msg->msg_type == M_MOUSE_DBCLICK) {

        if (fork() == 0) {

            char *fileName = widget->context.text->text;
            //printf(1, "extension is: ");
            //printf(1, getFileExtension(fileName));

            if (strcmp(getFileExtension(fileName), "txt") == 0) {
                //printf(1, "\nopen editor\n");
                //char *argv2[] = {"editor", widget->context.text->text};
                char *argv2[] = {"/bin/editor", widget->context.text->text, 0};
                exec (argv2[0], argv2);
                exit ();
            } else {                
               
                if (widget->type==IMAGE) {
                     char *argv2[] = {widget->name, 0};
                     exec(argv2[0], argv2);
                }

                if (widget->type==TEXT) {
                     char *argv2[] = {widget->context.text->text, 0};
                     exec(argv2[0], argv2);
                }
                
                exit();
            }
        }
    }
}

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), '\0', 1);
    //memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void 
gui_ls (char *path)
{
    int w, h;


   // strcpy (desktop.widgets[current_path_widget].context.text->text, path);

    //printf(1, desktop.widgets[current_path_widget].context.text->text);

    while (1) {

        int p;
        for (p = desktop.widgetlisthead; p != -1; p = desktop.widgets[p].next) {

            if (desktop.widgets[p].type == TEXT && current_path_widget!=p) {
                removeWidget(&desktop, p);
                break;
            }
        }
        
        if (p == -1) {
            break;
        }
    }

    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    int lineCount = 0;

    if ((fd = open(path, 0)) < 0)
    {
        printf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:

        addTextWidget(&desktop, textColor, fmtname(path), 10, statusBarHeight + lineCount * CHARACTER_HEIGHT, 200, CHARACTER_HEIGHT, 1, emptyHandler);
        lineCount++;
        //printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
        break;

    case T_DIR:

        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf(1, "ls: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {


            if (de.inum == 0)
                continue;

            memmove(p, de.name, DIRSIZ);

            p[DIRSIZ] = 0;

            if (stat(buf, &st) < 0)
            {
                printf(1, "ls: cannot stat %s\n", buf);
                continue;
            }

            //printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            char formatName[MAX_SHORT_STRLEN];

            strcpy (formatName, fmtname(buf));

            if (st.type == T_FILE && (isOpenable(formatName) || strcmp(getFileExtension(formatName), "txt") == 0)) {
//printf (0, "file = %s\n", buf);



                add_image (&desktop, "/usr/share/icons/app.bmp", buf, (lineCount + 1) * 64, 48, buttonHandler, &w, &h);
                //addTextWidget (&desktop, textColor, formatName, 10, statusBarHeight + lineCount * CHARACTER_HEIGHT, 200, CHARACTER_HEIGHT, 1, buttonHandler);
                addTextWidget (&desktop, textColor, formatName, (lineCount + 1) * 64, 48 + 52, 64, CHARACTER_HEIGHT, 1, buttonHandler);
                lineCount++;

            }

            if (st.type == T_DIR && strcmp(formatName, ".") != 0 && strcmp(formatName, "..") != 0) {

                printf (0, "%s\n", formatName);
                add_image (&desktop, "/usr/share/icons/explorer.bmp", formatName, (lineCount + 1) * 64, 48, cdHandler, &w, &h);
               // addTextWidget (&desktop, dirColor, formatName, 10, statusBarHeight + lineCount * CHARACTER_HEIGHT, 200, CHARACTER_HEIGHT, 1, cdHandler);
                addTextWidget (&desktop, dirColor, formatName, (lineCount + 1) * 64, 48 + 52, 64, CHARACTER_HEIGHT, 1, emptyHandler);
                lineCount++;
            }
        }
        break;
    }

    close(fd);
}

int 
main (int argc, char *argv [])
{
    struct RGBA bgColor;
    struct RGBA border_color;

    desktop.width = 600;
    desktop.height = 300;

    desktop.hasTitleBar = 1;

//if (argc>1)
//    create_window (&desktop, "-explorer-");
//    else
    create_window (&desktop, "explorer");

    desktop.needsRepaint = 0;

    bgColor.R = 89;
    bgColor.G = 101;
    bgColor.B = 186;
    bgColor.A = 255;

    textColor.R = 255;
    textColor.G = 255;
    textColor.B = 255;
    textColor.A = 255;

    dirColor.R = 255;
    dirColor.G = 255;
    dirColor.B = 255;
    dirColor.A = 255;

    addColorFillWidget (&desktop, bgColor, 0, 0, desktop.width, desktop.height, 0, emptyHandler, "explorer");

    if (argc>1) {
        printf (0, "path = %s\n", argv [1]);
        strcpy (current_path, argv [1]);
    } else
        strcpy (current_path, "");

   // strcpy (current_path, "bin");

chpr (getpid(), 3);

    gui_ls (current_path);

chpr (getpid(), 10);

    struct RGBA buttonColor;
    buttonColor.R = 244;
    buttonColor.G = 180;
    buttonColor.B = 0;
    buttonColor.A = 255;

    border_color.R = 255;
    border_color.G = 255;
    border_color.B = 255;
    border_color.A = 255;

    //addButtonWidget(&desktop, textColor, buttonColor, border_color, "mkdir", 80, 10, 50, 30, 0, mkdirHandler);
    //addButtonWidget(&desktop, textColor, buttonColor, border_color, "back", 10, 10, 50, 30, 0, backHandler);

    // current_path_widget = addTextWidget(&desktop, textColor, current_path, 
    // 160, 10+6, 200, CHARACTER_HEIGHT, 0, emptyHandler);

    desktop.needsRepaint = 1;

//printf (0, "e\n");

    while (1) {
        updateWindow (&desktop);
    }
}
