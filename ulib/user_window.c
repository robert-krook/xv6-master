/*
 *  user_window.c -- user mode window library.
 *
 *  We need the functions for graphical applications.
 */

#include "types.h"
#include "stat.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#include "user.h"
#include "gui/user_window.h"
#include "gui/graphics.h"
#include "gui/character.h"

#include "gui/bitmap.h"

void drawColorFillWidget (window *win, Widget *w);
void drawButtonWidget (window *win, Widget *w);
void drawTextWidget (window *win, Widget *w);
void drawInputFieldWidget (window *win, Widget *w);
void drawShapeWidget (window *win, Widget *w);
void drawImageWidget (window *win, Widget *w);

int freeWidget (window *win, int index);

// Define the standard 16-color VGA/CGA palette
const int palette[16][3] = {
    {0, 0, 0},       // 0: Black
    {0, 0, 170},     // 1: Blue
    {0, 170, 0},     // 2: Green
    {0, 170, 170},   // 3: Cyan
    {170, 0, 0},     // 4: Red
    {170, 0, 170},   // 5: Magenta
    {170, 85, 0},    // 6: Brown
    {170, 170, 170}, // 7: Light Gray
    {85, 85, 85},    // 8: Dark Gray
    {85, 85, 255},   // 9: Light Blue
    {85, 255, 85},   // 10: Light Green
    {85, 255, 255},  // 11: Light Cyan
    {255, 85, 85},   // 12: Light Red
    {255, 85, 255},  // 13: Light Magenta
    {255, 255, 85},  // 14: Yellow
    {255, 255, 255}  // 15: White
};

/*
 * get_nearest_color -- Function to find the nearest 16-color palette index from 256 colors.
 */
int 
get_nearest_color (int r, int g, int b) 
{
    int best_index = 1.0;

    long min_distance =  __LONG_MAX__;

    for (int i = 0; i < 16; i++) {

        // Calculate squared Euclidean distance to avoid slow sqrt() calls
        long dr = r - palette[i][0];
        long dg = g - palette[i][1];
        long db = b - palette[i][2];

        long distance = (dr * dr) + (dg * dg) + (db * db);

        if (distance < min_distance) {
            min_distance = distance;
            best_index = i;
        }

        if (min_distance == 0)
            break;

    }

    return best_index;
}

/*
 *  drawPointAlpha -- draw a point with a color at a position.
 */
void 
drawPointAlpha (RGB *color, RGBA origin)
{
    float alpha;

    if (origin.A == 255) {
        color->R = origin.R;
        color->G = origin.G;
        color->B = origin.B;
        return;
    }

    if (origin.A == 0) {
        return;
    }

    alpha = (float) (origin.A / 255);
    color->R = color->R * (1 - alpha) + origin.R * alpha;
    color->G = color->G * (1 - alpha) + origin.G * alpha;
    color->B = color->B * (1 - alpha) + origin.B * alpha;
}

void 
drawFillRect (window *win, RGBA color, int x, int y, int width, int height)
{
    int screen_width = win->width;
    int screen_height = win->height;

    if (x >= screen_width || x + width < 0 || y >= screen_height || y + height < 0 || width < 0 || height < 0) {
        return;
    }

    if (x < 0) {
        width = width + x;
        x = 0;
    }

    if (y < 0) {
        height = height + y;
        y = 0;
    }

    if (x + width > screen_width) {
        width = screen_width - x;
    }

    if (y + height > screen_height) {
        height = screen_height - y;
    }

    int i, j;
    RGB *t;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (x + j > 0 && x + j < screen_width && y + i > 0 && y + i < screen_height) {
                t = win->window_buf + (y + i) * win->width + (x + j);
                drawPointAlpha(t, color);
            }
        }
    }

}

void 
debugPrintWidgetList (window *win)
{
    printf(1, "############################\n");
    printf(1, "current Head at %d\n", win->widgetlisthead);
    printf(1, "current Tail at %d\n", win->widgetlisttail);
    printf(1, "current scrollOffset is %d\n", win->scrollOffsetY);
    printf(1, "\n");

    int p;
    for (p = win->widgetlisthead; p != -1; p = win->widgets[p].next) {
        printf(1, "current Widget at %d, type: %d, scrollable: %d\n", p, win->widgets[p].type, win->widgets[p].scrollable);
        printf(1, "current Widget position %d, %d, %d, %d\n", win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax);
        printf(1, "prev Window at %d\n", win->widgets[p].prev);
        printf(1, "next Window at %d\n", win->widgets[p].next);
        //cprintf("current Window width %d\n", windowlist[p].wnd->position.xmax - windowlist[p].wnd->position.xmin);
        printf(1, "\n");
    }
}

void 
addToWidgetListTail (window *win, int idx)
{
    win->widgets[idx].prev = win->widgetlisttail;
    win->widgets[idx].next = -1;
    if (win->widgetlisttail != -1)
        win->widgets[win->widgetlisttail].next = idx;
    win->widgetlisttail = idx;
}

int 
findNextAvailable (window *win)
{

    for (int i = 0; i < MAX_WIDGET_SIZE; i++)
    {
        if (win->widgets[i].prev == i && win->widgets[i].next == i)
        {
            return i;
        }
    }
    return -1;
}

int 
isInRect (int xmin, int ymin, int xmax, int ymax, int x, int y)
{
    return (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
}

void 
closePopupWindow (window *win)
{
    free(win->window_buf);
    for (int p = win->widgetlisthead; p != -1; p = win->widgets[p].next)
    {
        freeWidget(win, p);
    }
    GUI_closePopupWindow(win);
    exit();
}

void 
createPopupWindow (window *win, int caller)
{

    int width = win->width;
    int height = win->height;

    win->window_buf = malloc(width * height * 3);
    if (!win->window_buf)
    {
        return;
    }

    //memset(win->window_buf, 255, height * width * 3);
    
    win->widgetlisthead = -1;
    win->widgetlisttail = -1;
    int i;
    for (i = 0; i < MAX_WIDGET_SIZE; ++i)
    {
        win->widgets[i].next = i;
        win->widgets[i].prev = i;
    }
    win->needsRepaint = 1;
    win->hasTitleBar = 0;
    win->scrollOffsetX = 0;
    win->scrollOffsetY = 0;
    GUI_createPopupWindow(win, caller);
}

void 
create_window (window *win, const char *title)
{
    int width = win->width;
    int height = win->height;

    win->window_buf = malloc(width * height * 3);

    if (!win->window_buf) {
        printf (0, "No memory left\n");
        return;
    }

    win->keyfocus = -1;
    win->scrollOffsetX = 0;
    win->scrollOffsetY = 0;
    win->widgetlisthead = -1;
    win->widgetlisttail = -1;

    int i;
    for (i = 0; i < MAX_WIDGET_SIZE; ++i) {
        win->widgets[i].next = i;
        win->widgets[i].prev = i;        
    }

    win->needsRepaint = 1;

    if (win->hasTitleBar != 0) {
        win->hasTitleBar = 1;
    }

    GUI_createWindow (win, title);
}

void 
repaintWindow (window *win)
{
    if (win->needsRepaint) {

        //memset(win->window_buf, 255, win->height * win->width * 3);

        for (int p = win->widgetlisthead; p != -1; p = win->widgets[p].next) {

            //don't draw widget that is invisible
            if ((!win->widgets[p].scrollable && (win->widgets[p].position.xmin > win->width ||
                                                 win->widgets[p].position.xmax < 0 ||
                                                 win->widgets[p].position.ymin > win->height ||
                                                 win->widgets[p].position.ymax < 0)) ||
                (win->widgets[p].scrollable && (win->widgets[p].position.xmin - win->scrollOffsetX > win->width ||
                                                win->widgets[p].position.xmax - win->scrollOffsetX < 0 ||
                                                win->widgets[p].position.ymin - win->scrollOffsetY > win->height ||
                                                win->widgets[p].position.ymax - win->scrollOffsetY < 0))) {
                continue;
            }

            switch (win->widgets[p].type) {
            case COLORFILL:
                drawColorFillWidget(win, &win->widgets[p]);
                break;
            case BUTTON:
                drawButtonWidget(win, &win->widgets[p]);
                break;
            case TEXT:
                drawTextWidget(win, &win->widgets[p]);
                break;
            case INPUTFIELD:
                drawInputFieldWidget(win, &win->widgets[p]);
                break;
            case SHAPE:
                //drawShapeWidget(win, &win->widgets[p]);
                break;
            case IMAGE:
                drawImageWidget(win, &win->widgets[p]);
                break;
            default:
                break;
            }
        }

    }
}

void 
closeWindow(window *win)
{
    free(win->window_buf);
    
    for (int p = win->widgetlisthead; p != -1; p = win->widgets[p].next) {
        freeWidget(win, p);
    }

    GUI_closeWindow(win);

    exit();
}

void 
updatePopupWindow (window *win)
{
    repaintWindow (win);

    message msg;

    if (GUI_getPopupMessage (&msg) == 0) {

        win->needsRepaint = 1;

        //deleting this printing seems to make popup window unable to open other programs
        //printf(2, "message is %d\n", msg.msg_type);

        if (msg.msg_type == WM_WINDOW_CLOSE) {
            closePopupWindow(win);
        }
        else {

            if (msg.msg_type == M_KEY_DOWN || msg.msg_type == M_KEY_UP) {

                win->widgets[win->keyfocus].handler(&win->widgets[win->keyfocus], &msg);

            } else {

                int mouse_x = msg.params[0];
                int mouse_y = msg.params[1];

                for (int p = win->widgetlisttail; p != -1; p = win->widgets[p].prev) {

                    if (isInRect(win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax, mouse_x, mouse_y)) {

                        win->widgets[p].handler(&win->widgets[p], &msg);

                        if (win->widgets[p].type == INPUTFIELD) {
                            win->keyfocus = p;
                        }

                        break;
                    }
                }
            }
        }
    } else {
        win->needsRepaint = 0;
    }
    
    return;
}

void 
updateWindow (window *win)
{
    repaintWindow (win);

    message msg;

    if (GUI_getMessage (win->handler, &msg) == 0) {

        win->needsRepaint = 1;

 if (msg.msg_type == WM_WINDOW_INVALIDATE) {
    printf (0, "Invalidated %s\n", "");


 message newmsg;

                        newmsg.msg_type = msg.msg_type;
                        //newmsg.params[0] = mouse_x + win->scrollOffsetX;
                        //newmsg.params[1] = mouse_y + win->scrollOffsetY;

                        //win->widgets[0].handler (&win->widgets[0], &newmsg);
                        win->wnd_handler(0, &newmsg);

 }


        if (msg.msg_type == WM_WINDOW_CLOSE) {
            closeWindow (win);

        } else if (msg.msg_type == WM_WINDOW_MINIMIZE) {
            GUI_minimizeWindow (win);

        } else if (msg.msg_type == WM_WINDOW_MAXIMIZE) {
            GUI_maximizeWindow (win);

        } else if (win->keyfocus != -1 && (msg.msg_type == M_KEY_DOWN || msg.msg_type == M_KEY_UP)) {
            win->widgets[win->keyfocus].handler(&win->widgets[win->keyfocus], &msg);

        } else {

            int mouse_x = msg.params[0];
            int mouse_y = msg.params[1];

            for (int p = win->widgetlisttail; p != -1; p = win->widgets[p].prev) {

                if ((!win->widgets[p].scrollable && isInRect(win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax, mouse_x, mouse_y)) ||
                    (win->widgets[p].scrollable && isInRect(win->widgets[p].position.xmin - win->scrollOffsetX, win->widgets[p].position.ymin - win->scrollOffsetY, win->widgets[p].position.xmax - win->scrollOffsetX, win->widgets[p].position.ymax - win->scrollOffsetY, mouse_x, mouse_y))) {

                    if (!win->widgets[p].scrollable) {
                        win->widgets[p].handler (&win->widgets[p], &msg);
                    } else {
                        message newmsg;

                        newmsg.msg_type = msg.msg_type;
                        newmsg.params[0] = mouse_x + win->scrollOffsetX;
                        newmsg.params[1] = mouse_y + win->scrollOffsetY;

                        win->widgets[p].handler (&win->widgets[p], &newmsg);
                    }

                    if (win->widgets[p].type == INPUTFIELD) {
                        win->keyfocus = p;
                    }

                    break;
                }
            }
        }
    } else {
        win->needsRepaint = 0;
    }
    return;
}

int 
drawCharacter (window *win, int x, int y, char ch, RGBA color)
{
    int i, j;
    RGB *t;
    int ord = ch - 0x20;
    
    if (ord < 0 || ord >= (CHARACTER_NUMBER - 1)) {
        return -1;
    }

    for (i = 0; i < CHARACTER_HEIGHT; i++) {

        if (y + i > win->height) {
            break;
        }

        if (y + i < 0) {
            continue;
        }

        for (j = 0; j < CHARACTER_WIDTH; j++) {

            if (character[ord][i][j] == 1) 
            {
                if (x + j > win->width) {
                    break;
                }

                if (x + j < 0) {
                    continue;
                }

                t = win->window_buf + (y + i) * win->width + x + j;
                
                drawPointAlpha(t, color);
            }
        }
    }
    return CHARACTER_WIDTH;
}

void 
drawString (window *win, char *str, RGBA color, int x, int y, int width, int height)
{
    int offset_x = 0;
    int offset_y = 0;

    while (*str != '\0') {

        if (offset_y > height)
            break;
        
        if (*str != '\n') {

            if (x + offset_x >= 0 && x + offset_x <= win->width && y + offset_y >= 0 && y + offset_y <= win->height) {
                drawCharacter(win, x + offset_x, y + offset_y, *str, color);
            }

            offset_x += CHARACTER_WIDTH;

            if (offset_x > width) {
                offset_x = 0;
                offset_y += CHARACTER_HEIGHT;
            }
        }
        else
        {
            offset_x = 0;
            offset_y += CHARACTER_HEIGHT;
        }

        str++;
    }
}

void 
removeFromWidgetList(window *win, int idx)
{
    if (win->widgetlisthead == idx)
        win->widgetlisthead = win->widgets[win->widgetlisttail].next;

    if (win->widgetlisttail == idx)
        win->widgetlisttail = win->widgets[win->widgetlisttail].prev;

    if (win->widgets[idx].prev != -1)
        win->widgets[win->widgets[idx].prev].next = win->widgets[idx].next;

    if (win->widgets[idx].next != -1)
        win->widgets[win->widgets[idx].next].prev = win->widgets[idx].prev;
    
    win->widgets[idx].prev = idx;
    win->widgets[idx].next = idx;
}

/*
 * Add a new widget to the window.
 */
int 
addWidget (window *win)
{
    int widgetId = findNextAvailable(win);
    if (widgetId == -1)
        return -1;

    if (win->widgetlisthead == -1)
    {
        win->widgetlisthead = widgetId;
    }

    addToWidgetListTail(win, widgetId);

    return widgetId;
}

void 
drawRect (window *win, RGB color, int x, int y, int width, int height)
{
    int screen_width = win->width;
    int screen_height = win->height;

    if (x >= screen_width || x + width < 0 || y >= screen_height || y + height < 0 || width < 0 || height < 0) {
        return;
    }

    int i;

    //int max_line = (SCREEN_WIDTH - x) < width ? (SCREEN_WIDTH - x) : width;
    RGB *t = win->window_buf + y * screen_width + x;

    if (y >= 0) {
        for (i = 0; i < width; i++) {
            if (x + i >= 0 && x + i < screen_width) {
                *(t + i) = color;
            }
        }
    }

    if (y + height <= screen_height) {
        RGB *o = t + height * screen_width;
        for (i = 0; i < width; i++) {
            if (y >= 0 && x + i > 0 && x + i < screen_width) {
                *(o + i) = color;
            }
        }
    }

    if (x >= 0) {
        for (i = 0; i < height; i++) {
            if (y + i >= 0 && y + i < screen_height) {
                *(t + i * screen_width) = color;
            }
        }
    }

    if (x + width <= screen_width) {
        RGB *o = t + width;
        for (i = 0; i < height; i++) {
            if (y + i >= 0 && y + i < screen_height) {
                *(o + i * screen_width) = color;
            }
        }
    }

}

int 
freeWidget (window *win, int index)
{
    switch (win->widgets[index].type) {
    case COLORFILL:
        free (win->widgets[index].context.colorfill);
        break;
    case BUTTON:
        free (win->widgets[index].context.button);
        break;
    case TEXT:
        free (win->widgets[index].context.text);
        break;
    case INPUTFIELD:
        free (win->widgets[index].context.inputfield);
        break;
    case IMAGE:
        free (win->widgets[index].context.image);
        break;
    default:
        break;
    }
    return 0;
}

/*
 *  removeWidget -- remove widget from the window.
 */
int 
removeWidget (window *win, int index)
{
    if (win->widgets[index].prev == index && win->widgets[index].next == index) {
        return -1;
    }

    freeWidget(win, index);
    
    removeFromWidgetList(win, index);
    
    return 0;
}

void 
setWidgetSize (Widget *widget, int x, int y, int w, int h)
{
    widget->position.xmin = x;
    widget->position.ymin = y;
    widget->position.xmax = x + w;
    widget->position.ymax = y + h;
}

/*
 *  addTextWidget -- add a text (label) to the window.
 */
int 
addTextWidget (window *win, RGBA c, char *text, int x, int y, int w, int h, int scrollable, Handler handler)
{

    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;

    Text *t = malloc (sizeof(Text));
    t->color = c;
    strcpy (t->text, text);

    Widget *widget = &win->widgets[widgetId];
    widget->context.text = t;
    widget->type = TEXT;
    widget->handler = handler;
    widget->scrollable = scrollable;
    setWidgetSize(widget, x, y, w, h);

    return widgetId;
}

/*
 *  addInputFieldWidget -- add an input field to the window.
 */
int 
addInputFieldWidget (window *win, RGBA c, char *text, int x, int y, int w, int h, int scrollable, Handler handler)
{
    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;

    InputField *t = malloc(sizeof(InputField));
    t->color = c;
    strcpy (t->text, text);
    t->current_pos = strlen (text);

    Widget *widget = &win->widgets [widgetId];
    widget->context.inputfield = t;
    widget->type = INPUTFIELD;
    widget->handler = handler;
    widget->scrollable = scrollable;
    setWidgetSize(widget, x, y, w, h);

    win->keyfocus = widgetId;

    return widgetId;
}

/*
 *  fillRect -- fill a rectangle with a color.
 */
void 
fillRect (RGB *buf, int x, int y, int width, int height, int max_x, int max_y, RGBA fill)
{
    int i, j;
    RGB *t;

    for (i = 0; i < height; i++) {

        if (y + i < 0)
            continue;

        if (y + i >= max_y)
            break;

        for (j = 0; j < width; j++) {
            if (x + j < 0)
                continue;

            if (x + j >= max_x)
                break;

            t = buf + (y + i) * max_x + x + j;

            drawPointAlpha(t, fill);
        }
    }
}

/*
 *  Fill the widget with a color.
 */
int 
addColorFillWidget (window *win, RGBA c, int x, int y, int w, int h, 
    int scrollable, Handler handler, char * title)
{
    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;

    ColorFill *b = malloc (sizeof (ColorFill));

    b->buf = malloc(w * h * 3);
        
    fillRect (b->buf, 0, 0, w, h, w, h, c);

    Widget *widget = &win->widgets [widgetId];
    widget->context.colorfill = b;
    widget->type = COLORFILL;
    widget->handler = handler;
    widget->scrollable = scrollable;

    setWidgetSize(widget, x, y, w, h);

    return widgetId;
}

/*
 *  draw24Image -- draw a raw image on the canvas.
 */
void 
draw24Image (window *win, RGB *img, int x, int y, int width, int height)
{
    int i;
    RGB *t;
    RGB *o;

    int max_line = (win->width - x) < width ? (win->width - x) : width;

    for (i = 0; i < height; i++) {

        if (y + i >= win->height) {
            break;
        }

        if (y + i < 0) {
            continue;
        }

        t = win->window_buf + (y + i) * win->width + x;
        o = img + (height - i - 1) * width;
        memmove(t, o, max_line * 3);
    }
}

/*
 *  drawColorFillWidget -- draw the background of the widget.
 */
void 
drawColorFillWidget (window *win, Widget *w)
{
    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;
    int xmin = w->position.xmin, ymin = w->position.ymin;

    if (w->scrollable) {
        xmin = w->position.xmin - win->scrollOffsetX;
        ymin = w->position.ymin - win->scrollOffsetY;
    }

    draw24Image (win, w->context.colorfill->buf, xmin, ymin, width, height);
}

/*
 *  addButtonWidget -- add a button to the window.
 */
int 
addButtonWidget (window *win, RGBA c, RGBA bc, RGBA border_color, char *text, 
    int x, int y, int w, int h, int scrollable, Handler handler)
{
    int widgetId = addWidget (win);
    if (widgetId == -1)
        return -1;

    Button *b = malloc (sizeof(Button));

    b->bg_color = bc;
    b->color = c;
    b->text_align = 0;                          // Default left alignment

    strcpy (b->text, text);

    Widget *widget = &win->widgets[widgetId];
    widget->context.button = b;
    widget->type = BUTTON;
    widget->handler = handler;
    widget->scrollable = scrollable;
    widget->border = border_color;
    strcpy (widget->name, text);

    setWidgetSize (widget, x, y, w, h);

    return widgetId;
}

int 
addImageWidget (window *win, RGB *image, int x, int y, int w, int h, Handler handler, unsigned char *name)
{
    int widgetId = addWidget (win);
    if (widgetId == -1)
        return -1;

    Image * b = malloc (sizeof(Image));

    b->image = image;

    RGBA border_color;

    Widget *widget = &win->widgets[widgetId];
    widget->context.image = b;
    widget->type = IMAGE;
    widget->handler = handler;
    widget->scrollable = 0;
    widget->border = border_color;

    strcpy (widget->name, name);

    setWidgetSize (widget, x, y, w, h);

    return widgetId;
}

/*
 *  drawInputFieldWidget -- draw an input field at the window.
 */
void 
drawInputFieldWidget (window *win, Widget *w)
{
    int xmin = w->position.xmin, ymin = w->position.ymin;
    if (w->scrollable) {
        xmin = w->position.xmin - win->scrollOffsetX;
        ymin = w->position.ymin - win->scrollOffsetY;
    }

    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;

    drawString (win, w->context.inputfield->text, w->context.inputfield->color, xmin, ymin, width, height);

    // draw the text cursor
    int offset_x = 0;
    int offset_y = 0;
    int iter = 0;

    while (iter < w->context.inputfield->current_pos) {

        if (offset_y > height)
            break;
        
        if (w->context.inputfield->text[iter] != '\n') {
            offset_x += CHARACTER_WIDTH;
            if (offset_x > width)
            {
                offset_x = 0;
                offset_y += CHARACTER_HEIGHT;
            }
        } else {
            offset_x = 0;
            offset_y += CHARACTER_HEIGHT;
        }
        iter++;
    }

    RGBA black;
    black.R = 255;
    black.G = 255;
    black.B = 255;
    black.A = 255;

    if (offset_y < height) {
        drawFillRect(win, black, xmin + offset_x, ymin + offset_y + 1, 1, CHARACTER_HEIGHT - 4);
    }
}

/*
 *  drawButtonWidget -- draw a button at the window.
 */
void 
drawButtonWidget (window *win, Widget *w)
{
    RGB border_color;

    border_color.R = w->border.R;
    border_color.G = w->border.G;
    border_color.B = w->border.B;

    int xmin = w->position.xmin, ymin = w->position.ymin;

    if (w->scrollable) {
        xmin = w->position.xmin - win->scrollOffsetX;
        ymin = w->position.ymin - win->scrollOffsetY;
    }

    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;

    int textYOffset = (height - CHARACTER_HEIGHT) / 2;
    int textXOffset = 2;

    if (w->context.button->text_align==1) {
        if (width > strlen(w->context.button->text) * CHARACTER_WIDTH) {
            textXOffset = (width - strlen(w->context.button->text) * CHARACTER_WIDTH) / 2;
        }
    }
    
    // Fill the widget with background color
    drawFillRect (win, w->context.button->bg_color, xmin, ymin, width, height);

    // Draw border
    drawRect (win, border_color, xmin, ymin, width, height);

    // Draw text in widget
    drawString (win, w->context.button->text, w->context.button->color, xmin + textXOffset, ymin + textYOffset, width, height);
}

/*
 * drawTextWidget -- draw text widget at the window canvas.
 */
void 
drawTextWidget (window *win, Widget *w)
{
    int xmin = w->position.xmin, ymin = w->position.ymin;

    if (w->scrollable) {
        xmin = w->position.xmin - win->scrollOffsetX;
        ymin = w->position.ymin - win->scrollOffsetY;
    }

    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;

    drawString (win, w->context.text->text, w->context.text->color, 
        xmin, ymin, width, height);

}

void 
drawImageWidget (window *win, Widget *w)
{
    int xmin = w->position.xmin, ymin = w->position.ymin;

    if (w->scrollable) {
        xmin = w->position.xmin - win->scrollOffsetX;
        ymin = w->position.ymin - win->scrollOffsetY;
    }

    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;

    draw24Image (win, w->context.image->image, xmin, ymin, width, height);

}

int 
add_image (window *win, unsigned char *filename, unsigned char *name, int x, int y, Handler handler, int *w, int *h)
{
    int bmpFile = open(filename, 0);
    if (bmpFile < 0) {
        return 1;
    }

    BITMAP_FILE_HEADER bmpFileHeader;
    BITMAP_INFO_HEADER bmpInfoHeader;

    readBitmapHeader (bmpFile, &bmpFileHeader, &bmpInfoHeader);

    close (bmpFile);

    RGB * image = malloc (bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 3);
 
    read24BitmapFile (filename, image, h, w);

    addImageWidget (win, image, x, y, *w, *h, handler, name);

    return 0;
}

int
select_style (int style)
{
        GUI_select_style (style);
}

int
get_style (void)
{
        return GUI_get_style ();
}
