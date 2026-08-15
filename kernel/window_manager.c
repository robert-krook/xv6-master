/*
 *  window_manager.c -- handles all GUI windows.
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

#include "gui/amiga/amiga.h"

int min(int x, int y) { return x < y ? x : y; }
int max(int x, int y) { return x > y ? x : y; }

int clamp(int x, int l, int r) { return min(r, max(l, x)); }

struct RGBA iconColor;
struct RGBA titleBarColor;
struct RGBA dockColor;
struct RGBA closeColor;
struct RGBA txtColor;
struct RGBA minimizeColor;

#define MAX_WINDOW_CNT 50

//linked-list of windows
static struct {
	struct proc *proc;
	kernel_window wnd;
	int next, prev;
} windowlist [MAX_WINDOW_CNT];

static int windowlisthead, windowlisttail;
static int desktopId = -1;

static struct {
	struct proc *proc;
	kernel_window wnd;
	int caller;
} popupwindow;

static int mouseShape;
static int gui_style;

static int clickedOnTitle, clickedOnContent, clickedOnPopup;

struct spinlock wmlock;

static struct {
	int x, y;
} wm_mouse_pos, wm_last_mouse_pos;

#define MOUSE_SPEED_X 1
#define MOUSE_SPEED_Y -1

int 
isInRect (int xmin, int ymin, int xmax, int ymax, int x, int y)
{
	return (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
}

void 
create_rect_by_coord (win_rect *rect, int xmin, int ymin, int xmax, int ymax)
{
	rect->xmin = xmin;
	rect->xmax = xmax;
	rect->ymin = ymin;
	rect->ymax = ymax;
}

void create_rect_by_size (win_rect *rect, int xmin, int ymin, int width, int height)
{
	rect->xmin = xmin;
	rect->xmax = xmin + width;
	rect->ymin = ymin;
	rect->ymax = ymin + height;
}

void 
moveRect (win_rect *rect, int dx, int dy)
{
	rect->xmin += dx;
	rect->xmax += dx;
	rect->ymin += dy;
	rect->ymax += dy;
}

int 
findNextAvailableWindowId ()
{
	for (int i = 0; i < MAX_WINDOW_CNT; i++)
	{
		if (windowlist[i].prev == i && windowlist[i].next == i)
		{
			return i;
		}
	}
	return -1;
}

void 
addToWindowList (int idx)
{
	windowlist[idx].prev = windowlisttail;
	windowlist[idx].next = -1;
	if (windowlisttail != -1)
		windowlist[windowlisttail].next = idx;
	windowlisttail = idx;
}

void 
removeFromWindowList (int idx)
{
	if (windowlisttail == idx)
		windowlisttail = windowlist[windowlisttail].prev;
	if (windowlist[idx].prev != -1)
		windowlist[windowlist[idx].prev].next = windowlist[idx].next;
	if (windowlist[idx].next != -1)
		windowlist[windowlist[idx].next].prev = windowlist[idx].prev;
}

void 
initMessageQueue (msg_buf *buf)
{
	buf->front = buf->rear = buf->cnt = 0;
}

int 
dispatchMessage (msg_buf *buf, message *msg)
{
	if (buf->cnt >= MSG_BUF_SIZE)
		return 1;
	++buf->cnt;
	buf->data[buf->rear] = *msg;
	if ((++buf->rear) >= MSG_BUF_SIZE)
		buf->rear = 0;
	return 0;
}

//return non-zero if buf is empty
int 
getMessage (msg_buf *buf, message *result)
{
	if (buf->cnt == 0)
		return 1;

	acquire (&wmlock);

	--buf->cnt;

	*result = buf->data [buf->front];

	if ((++buf->front) >= MSG_BUF_SIZE)
		buf->front = 0;

	release (&wmlock);

	return 0;
}

/*
 * wmInit -- initialize the window manager.
 */
void 
wmInit ()
{
//    amiga_init ();

	titleBarColor.R = 255;
	titleBarColor.G = 255;
	titleBarColor.B = 255;
	titleBarColor.A = 255;
#if defined (__VGA__)  || defined (__BGA__)
    titleBarColor.R = get_nearest_color (titleBarColor.R, titleBarColor.G, titleBarColor.B);
#endif

	dockColor.R = 255; //15;
	dockColor.G = 255; //157;
	dockColor.B = 255; //88;
	dockColor.A = 255;
#if defined (__VGA__)  || defined (__BGA__)
    dockColor.R = get_nearest_color (dockColor.R, dockColor.G, dockColor.B);
#endif

	closeColor.R = 255;
	closeColor.G = 255;
	closeColor.B = 255;
	closeColor.A = 255;
#if defined (__VGA__)  || defined (__BGA__)
    closeColor.R = get_nearest_color (closeColor.R, closeColor.G, closeColor.B);
#endif

	iconColor.R = 85;
	iconColor.G = 85;
	iconColor.B = 255;
	iconColor.A = 255;
#if defined (__VGA__) || defined (__BGA__)
    iconColor.R = get_nearest_color (iconColor.R, iconColor.G, iconColor.B);
#endif

	txtColor.R = 0;
    txtColor.G = 0;
    txtColor.B = 0;
    txtColor.A = 255;
#if defined (__VGA__)  || defined (__BGA__)
    txtColor.R = get_nearest_color (txtColor.R, txtColor.G, txtColor.B);
#endif

	minimizeColor.R = 255;
    minimizeColor.G = 255;
    minimizeColor.B = 255;
	minimizeColor.A = 255;
#if defined (__VGA__)  && defined (__BGA__)
    minimizeColor.R = get_nearest_color (minimizeColor.R, minimizeColor.G, minimizeColor.B);
#endif

	mouseShape = 0;
    gui_style = 1;  // default alpha style

	wm_mouse_pos.x = SCREEN_WIDTH / 2;
	wm_mouse_pos.y = SCREEN_HEIGHT / 2;
    
	wm_last_mouse_pos = wm_mouse_pos;

	windowlisthead = -1;
	windowlisttail = -1;

	int i;

	for (i = 0; i < MAX_WINDOW_CNT; ++i) {
		windowlist[i].next = i;
		windowlist[i].prev = i;
	}

	popupwindow.caller = -1;

	clickedOnTitle = clickedOnContent = clickedOnPopup = 0;

	initlock (&wmlock, "wmlock");
}

void 
debugPrintWindowList ()
{
	cprintf("############################\n");
	cprintf("current Proc at %d\n", myproc());
	cprintf("current Head at %d\n", windowlisthead);
	cprintf("current Tail at %d\n", windowlisttail);
	cprintf("\n");

	int p;
	for (p = windowlisthead; p != -1; p = windowlist[p].next) {

		cprintf("current Window at %d\n", p);
		cprintf("current Window proc %d\n", windowlist[p].proc);
		cprintf("prev Window at %d\n", windowlist[p].prev);
		cprintf("next Window at %d\n", windowlist[p].next);
		cprintf("\n");
	}
}

void 
focusWindow(int winId)
{
	if (winId == -1 || winId == windowlisttail)
		return;

	if (winId == windowlisthead) {
		int newhead = windowlist[winId].next;
		windowlist[newhead].prev = -1;
		windowlisthead = newhead;
	}
	else
	{
		int prevWin = windowlist[winId].prev;
		int nextWin = windowlist[winId].next;
		windowlist[prevWin].next = nextWin;
		windowlist[nextWin].prev = prevWin;
	}
	addToWindowList(winId);
}

void 
moveFocusWindow(int dx, int dy)
{
	if (windowlist[windowlisttail].wnd.hasTitleBar)
	{
		moveRect(&windowlist[windowlisttail].wnd.position, dx, dy);
	}
}

int 
getWindowCount ()
{
	int windowCount = 0;

	for (int p = windowlisthead; p != -1; p = windowlist[p].next)
	{
		if (p != desktopId)
		{
			windowCount++;
		}
	}
	return windowCount;
}

void 
handleDesktopDockClick (int mouse_x, message *msg)
{
	int p;
	int windowCount = getWindowCount();

	if (windowCount > 0) { //handle each program dock bar click

		int xStart = START_ICON_WIDTH + 5;
		int barWidth = min((SCREEN_WIDTH - START_ICON_WIDTH - SHOW_DESKTOP_ICON_WIDTH) / (windowCount + 1), DOCK_PROGRAM_NORMAL_WIDTH);

		for (p = windowlisthead; p != -1; p = windowlist[p].next) {
			if (p != desktopId)
			{
				if (mouse_x > xStart + 1 && mouse_x <= xStart + barWidth - 1)
				{
					if (windowlist[p].wnd.minimized)
					{
						message newmsg;
						newmsg.msg_type = WM_WINDOW_MAXIMIZE;
						dispatchMessage(&windowlist[p].wnd.msg_buf, &newmsg);
					}
					else
					{
						focusWindow(p);
					}
					break;
				}
				xStart += barWidth;
			}
		}
	}

	if (mouse_x >= SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH) //handle show desktop icon click
	{
		for (p = windowlisthead; p != -1; p = windowlist[p].next)
		{
			if (p != desktopId)
			{
				if (windowlist[p].wnd.minimized == 0)
				{
					message newmsg;
					newmsg.msg_type = WM_WINDOW_MINIMIZE;
					dispatchMessage(&windowlist[p].wnd.msg_buf, &newmsg);
				}
			}
		}
	}
	if (mouse_x <= START_ICON_WIDTH && msg->msg_type == M_MOUSE_LEFT_CLICK) //handle start window icon click
	{
		if (popupwindow.caller != desktopId)
		{
			message newmsg;
			newmsg.msg_type = msg->msg_type;
			newmsg.params[0] = wm_mouse_pos.x;
			newmsg.params[1] = wm_mouse_pos.y;
			dispatchMessage(&windowlist[desktopId].wnd.msg_buf, &newmsg);
		}
	}
}


//handle messages from mouse and keyboard
void 
wmHandleMessage (message *msg)
{
    acquire(&wmlock);

	message newmsg;

	switch (msg->msg_type)
	{
	    case M_MOUSE_MOVE:

		    wm_last_mouse_pos = wm_mouse_pos;

    		if (msg->params[0] > -SCREEN_WIDTH && msg->params[0] < SCREEN_WIDTH)
	    	{
		    	if (msg->params[1] > -SCREEN_HEIGHT && msg->params[0] < SCREEN_HEIGHT)
			    {
				    wm_mouse_pos.x += msg->params[0] * MOUSE_SPEED_X;
				    wm_mouse_pos.y += msg->params[1] * MOUSE_SPEED_Y;
			    }
		    }

		    if (wm_mouse_pos.x > SCREEN_WIDTH)
			    wm_mouse_pos.x = SCREEN_WIDTH;
		    if (wm_mouse_pos.y > SCREEN_HEIGHT)
			    wm_mouse_pos.y = SCREEN_HEIGHT;
		    if (wm_mouse_pos.x < 0)
			    wm_mouse_pos.x = 0;
		    if (wm_mouse_pos.y < 0)
			    wm_mouse_pos.y = 0;

		    if (clickedOnTitle) //dragging the title bar
		    {
			    mouseShape = 1;
			    moveFocusWindow(wm_mouse_pos.x - wm_last_mouse_pos.x, wm_mouse_pos.y - wm_last_mouse_pos.y);
		    }
		    else if (clickedOnContent) //dragging inside the window
		    {
			    newmsg.msg_type = msg->msg_type;
			    newmsg.params[0] = wm_mouse_pos.x - windowlist[windowlisttail].wnd.position.xmin;
			    newmsg.params[1] = wm_mouse_pos.y - windowlist[windowlisttail].wnd.position.ymin;
			    dispatchMessage(&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
		    }
		break;

	case M_MOUSE_DOWN:
		//check if clicked on the popup first
		if (popupwindow.caller != -1 && isInRect(popupwindow.wnd.position.xmin, popupwindow.wnd.position.ymin, popupwindow.wnd.position.xmax, popupwindow.wnd.position.ymax, wm_mouse_pos.x, wm_mouse_pos.y))
		{
			clickedOnPopup = 1;
		}
		else
		{
			//close the popup first
			message closePopup;
			closePopup.msg_type = WM_WINDOW_CLOSE;
			dispatchMessage (&popupwindow.wnd.msg_buf, &closePopup);
			//find the focus window
			int p = windowlisttail;
			for (; p != -1; p = windowlist[p].prev)
			{
				kernel_window *win = &windowlist[p].wnd;
				if (p != desktopId && win->minimized!=1 && isInRect(win->position.xmin, win->position.ymin - TITLE_HEIGHT, win->position.xmax, win->position.ymax, wm_mouse_pos.x, wm_mouse_pos.y))
				{
					focusWindow(p);
					break;
				}
			}
			if (p == -1)
				focusWindow(desktopId);

			//check if clicked on content or the title bar
			kernel_window *win = &windowlist[windowlisttail].wnd;
			if (isInRect(win->position.xmin, win->position.ymin - TITLE_HEIGHT, win->position.xmax, win->position.ymin, wm_mouse_pos.x, wm_mouse_pos.y))
			{
				if (wm_mouse_pos.x + 30 > win->position.xmax)
				{
					newmsg.msg_type = WM_WINDOW_CLOSE;
					dispatchMessage (&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
				}
				else if (wm_mouse_pos.x + 60 > win->position.xmax && wm_mouse_pos.x + 30 <= win->position.xmax)
				{
					//not nessasary for current design, we could just minimize the window here
					//but we may want minimized window to halt updating so passing this message to the window first
					newmsg.msg_type = WM_WINDOW_MINIMIZE;
					dispatchMessage (&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
				}
				else
				{
					clickedOnTitle = 1;
				}
			}
			//dispatch the message
			if (isInRect(win->position.xmin, win->position.ymin, win->position.xmax, win->position.ymax, wm_mouse_pos.x, wm_mouse_pos.y))
			{
				clickedOnContent = 1;
				newmsg.msg_type = msg->msg_type;
				newmsg.params[0] = wm_mouse_pos.x - popupwindow.wnd.position.xmin;
				newmsg.params[1] = wm_mouse_pos.y - popupwindow.wnd.position.ymin;
				dispatchMessage(&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
			}
		}

		break;

	case M_MOUSE_LEFT_CLICK:
	case M_MOUSE_RIGHT_CLICK:
	case M_MOUSE_DBCLICK:
		if (clickedOnPopup)
		{
			clickedOnPopup = 0;
			newmsg.msg_type = msg->msg_type;
			newmsg.params[0] = wm_mouse_pos.x - popupwindow.wnd.position.xmin;
			newmsg.params[1] = wm_mouse_pos.y - popupwindow.wnd.position.ymin;
			dispatchMessage(&popupwindow.wnd.msg_buf, &newmsg);
		}
		else if (clickedOnContent)
		{
			clickedOnContent = 0;
			//check if clicked on window dock
			//because we have a simple dock, updating it is entirely in the kernel.
			//if we want, we can let the desktop program handle it with the help of some new system calls related to the information of opened windows.
			if (windowlisttail == desktopId && wm_mouse_pos.y >= SCREEN_HEIGHT - DOCK_HEIGHT && wm_mouse_pos.y <= SCREEN_HEIGHT)
			{
				handleDesktopDockClick(wm_mouse_pos.x, msg);
			}
			else
			{
				newmsg.msg_type = msg->msg_type;
				newmsg.params[0] = wm_mouse_pos.x - windowlist[windowlisttail].wnd.position.xmin;
				newmsg.params[1] = wm_mouse_pos.y - windowlist[windowlisttail].wnd.position.ymin;
				dispatchMessage(&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
			}
		}

	case M_MOUSE_UP:

		if (clickedOnPopup)
		{
			clickedOnPopup = 0;
		}
		if (clickedOnContent)
		{
			clickedOnContent = 0;
			newmsg.msg_type = msg->msg_type;
			newmsg.params[0] = wm_mouse_pos.x - windowlist[windowlisttail].wnd.position.xmin;
			newmsg.params[1] = wm_mouse_pos.y - windowlist[windowlisttail].wnd.position.ymin;
			dispatchMessage (&windowlist[windowlisttail].wnd.msg_buf, &newmsg);
		}
		if (clickedOnTitle)
		{
			clickedOnTitle = 0;
		}
		mouseShape = 0;
		break;
	case M_KEY_DOWN:
	case M_KEY_UP:
		dispatchMessage(&windowlist[windowlisttail].wnd.msg_buf, msg);
		break;

	default:
		break;
	}
	release(&wmlock);
}

void 
draw_window_bar (kernel_window *win, RGBA barcolor)
{
	int xmin = win->position.xmin;
	int xmax = win->position.xmax + 1;
	int ymin = win->position.ymin - TITLE_HEIGHT;
	int ymax = win->position.ymin;

    #ifdef __VGA__
	vga_draw_rect_by_coord(xmin, ymin, xmax - 2 * TITLE_HEIGHT, ymax, barcolor);
	vga_draw_rect_by_coord(xmax - 2 * TITLE_HEIGHT, ymin, xmax - TITLE_HEIGHT, ymax, minimizeColor);
	vga_draw_rect_by_coord(xmax - TITLE_HEIGHT, ymin, xmax, ymax, closeColor);
	vga_draw_string(xmin + 5, ymin + 3, win->title, txtColor);
	vga_draw_icon(xmax - 2 * TITLE_HEIGHT - 1, ymin - 1, 1, iconColor);
	vga_draw_icon(xmax - TITLE_HEIGHT - 1, ymin - 1, 0, iconColor);
    #elifdef __BGA__
	bga_draw_rect_by_coord(xmin, ymin, xmax - 2 * TITLE_HEIGHT, ymax, barcolor.R);
	bga_draw_rect_by_coord(xmax - 2 * TITLE_HEIGHT, ymin, xmax - TITLE_HEIGHT, ymax, minimizeColor.R);
	bga_draw_rect_by_coord(xmax - TITLE_HEIGHT, ymin, xmax, ymax, closeColor.R);
	bga_draw_string(xmin + 5, ymin + 3, win->title, txtColor.R);
	bga_draw_icon(xmax - 2 * TITLE_HEIGHT - 1, ymin - 1, 1, iconColor.R);
	bga_draw_icon(xmax - TITLE_HEIGHT - 1, ymin - 1, 0, iconColor.R);
    #else
    bga_draw_rect_by_coord_24 (xmin, ymin, xmax - 2 * TITLE_HEIGHT, ymax, barcolor);
	bga_draw_rect_by_coord_24 (xmax - 2 * TITLE_HEIGHT, ymin, xmax - TITLE_HEIGHT, ymax, minimizeColor);
	bga_draw_rect_by_coord_24 (xmax - TITLE_HEIGHT, ymin, xmax, ymax, closeColor);
	bga_draw_string_24 (xmin + 5, ymin + 3, win->title, txtColor);
	bga_draw_icon_24 (xmax - 2 * TITLE_HEIGHT - 1, ymin - 1, 1, iconColor);
	bga_draw_icon_24 (xmax - TITLE_HEIGHT - 1, ymin - 1, 0, iconColor);
    #endif
}

void 
drawWindow (kernel_window *win)
{
	int width = win->position.xmax - win->position.xmin;
	int height = win->position.ymax - win->position.ymin;

#ifdef __VGA__
	// directly flush the RGB array of window onto screen_buf
	vga_draw_24_image_part(win->window_buf, win->position.xmin, win->position.ymin,
					width, height, 0, 0, width, height, win->title);

	vga_draw_rect_border(0, win->position.xmin, win->position.ymin,
				   width, height);

#elifdef __BGA__
	// directly flush the RGB array of window onto screen_buf
	bga_draw_24_image_part(win->window_buf, win->position.xmin, win->position.ymin,
					width, height, 0, 0, width, height, win->title);
                    
	bga_draw_rect_border(0, win->position.xmin, win->position.ymin,
				   width, height);
#else
	// directly flush the RGB array of window onto screen_buf

	    bga_draw_24_image_part_24 (win->window_buf, win->position.xmin, win->position.ymin,
					width, height, 0, 0, width, height, win->title);

    RGBA rgba;

    rgba.R = 255;
    rgba.G = 255;
    rgba.B = 255;
    rgba.A = 255;

	bga_draw_rect_border_24 (rgba, win->position.xmin, win->position.ymin,
				   width, height);

#endif

	if (win->hasTitleBar) {
		draw_window_bar (win, titleBarColor);
	}
}

void 
drawDesktopDock ()
{

    if (gui_style==1) {
#ifdef __VGA__
	vga_draw_rect_by_coord (0, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, dockColor);
#elifdef __BGA__
	bga_draw_rect_by_coord (0, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, dockColor.R);
#else
	bga_draw_rect_by_coord_24 (0, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, dockColor);
#endif
    } else {
        amiga_desktop_draw_dock ();
    }

	int p;
	int windowCount = getWindowCount ();

    if (gui_style==1) {
#ifdef __VGA__
	vga_draw_rect_by_coord(0, SCREEN_HEIGHT - DOCK_HEIGHT, START_ICON_WIDTH, SCREEN_HEIGHT, titleBarColor);
#elifdef __BGA__
    bga_draw_rect_by_coord(0, SCREEN_HEIGHT - DOCK_HEIGHT, START_ICON_WIDTH, SCREEN_HEIGHT, titleBarColor.R);
#else
    bga_draw_rect_by_coord_24(0, SCREEN_HEIGHT - DOCK_HEIGHT, START_ICON_WIDTH, SCREEN_HEIGHT, titleBarColor);
#endif
    } else {
        amiga_desktop_draw_title_bar ();
    }

	if (windowCount > 0) {

		int xStart = START_ICON_WIDTH + 5;

		int barWidth = min((SCREEN_WIDTH - START_ICON_WIDTH - SHOW_DESKTOP_ICON_WIDTH) / (windowCount + 1), 
            DOCK_PROGRAM_NORMAL_WIDTH);

		for (p = windowlisthead; p != -1; p = windowlist[p].next) {

			if (p != desktopId) {

                if (gui_style==1) {
//double w = (DOCK_HEIGHT * (double) 0.5);

//cprintf("(d7-1)\n");

//				draw_string_with_max_width(xStart + 5, SCREEN_HEIGHT - DOCK_HEIGHT * 0.7, 
//                    barWidth - 2, windowlist[p].wnd.title, txtColor);
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
                bga_draw_string_with_max_width_24 (xStart + 5, SCREEN_HEIGHT - DOCK_HEIGHT, 
                    barWidth - 2, windowlist[p].wnd.title, txtColor);
          
				bga_draw_rect_by_coord_24 (xStart + barWidth - 2, SCREEN_HEIGHT - DOCK_HEIGHT, 
                   xStart + barWidth, SCREEN_HEIGHT, txtColor);
				xStart += barWidth;
#endif
			} else {
                amiga_desktop_draw_active_applications (windowlist[p].wnd.title, barWidth, xStart);
            }
            }
		}
	}

    if (gui_style==1) {
        #ifdef __VGA__
            vga_draw_rect_by_coord (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, titleBarColor);
            vga_draw_icon (START_ICON_WIDTH / 2 - 15, SCREEN_HEIGHT - DOCK_HEIGHT + 3, 2, iconColor);
        #elifdef __BGA__
            bga_draw_rect_by_coord (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, titleBarColor.R);
            bga_draw_icon (START_ICON_WIDTH / 2 - 15, SCREEN_HEIGHT - DOCK_HEIGHT + 3, 2, iconColor.R);
        #else
            bga_draw_rect_by_coord_24 (SCREEN_WIDTH - SHOW_DESKTOP_ICON_WIDTH, SCREEN_HEIGHT - DOCK_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, titleBarColor);
            bga_draw_icon_24 (START_ICON_WIDTH / 2 - 15, SCREEN_HEIGHT - DOCK_HEIGHT + 3, 2, iconColor);
        #endif
    } else {
        amiga_desktop_draw_start_icon ();
    }
}

//the only place that actually updates the screen
void 
update_screen()
{
	acquire (&wmlock);

	if (myproc ()!=windowlist[desktopId].proc) {
		panic ("Update screen called by non desktop process");
		release (&wmlock);
		return;
	}

	//draw desktop first
	drawWindow (&windowlist[desktopId].wnd);

	drawDesktopDock ();

	// draw other windows in order
	// switch uvm to allow access to the RGB array of those windows
	int p;
	for (p = windowlisthead; p != -1; p = windowlist[p].next) {
		if (p != desktopId && windowlist[p].wnd.minimized == 0) {
			    switchuvm (windowlist[p].proc);
			    drawWindow (&windowlist[p].wnd);
		}
	}

	//draw popup window if there is one
	if (popupwindow.caller != -1)
	{
		switchuvm (popupwindow.proc);
		drawWindow (&popupwindow.wnd);
	}

	//switch back to the desktop process (because it is the only process that calls this function)
	if (myproc() == 0)
		switchkvm();
	else
		switchuvm(myproc());

	//draw the mouse

#ifdef __VGA__
    vga_draw_mouse (mouseShape, wm_mouse_pos.x, wm_mouse_pos.y);
	//memmove(screen, screen_buf, screen_size);
    vga_blit ();
#elifdef __BGA__
    bga_draw_mouse (mouseShape, wm_mouse_pos.x, wm_mouse_pos.y);
	//memmove(screen, screen_buf, screen_size);
    bga_blit ();
#else
    // Draw the mouse at the canvas
    bga_draw_mouse_24 (mouseShape, wm_mouse_pos.x, wm_mouse_pos.y);

    // Draw the screen
    bga_blit_24 ();
#endif

	release (&wmlock);
}

void
clear_window (window *win)
{
    for (int i=0; i<(win->width*win->height); i++) {
        RGB *rgb = win->window_buf +i;
        rgb->R = 89;
        rgb->G = 101;
        rgb->B = 186;
    }
}

/*
 *  Create a new window.
 *
 *  return window handler on succuss, -1 if unsuccessful
 */
int 
create_window (window_p window, char *title)
{
	acquire (&wmlock);

    clear_window (window);

	int winId = findNextAvailableWindowId();
	if (winId == -1) {
		release (&wmlock);
		return 1;
	}

	if (windowlisthead == -1) {
		windowlisthead = winId;
	}

	addToWindowList (winId);

	//the first window is always the desktop
	if (desktopId == -1) {
		desktopId = winId;
	}

	int xmin = window->initialPosition.xmin;
	int xmax = window->initialPosition.xmax;
	int ymin = window->initialPosition.ymin;
	int ymax = window->initialPosition.ymax;

	if (xmin >= 0 && xmax <= SCREEN_WIDTH &&
		ymin >= 0 && ymax <= SCREEN_HEIGHT &&
		xmax - xmin == window->width &&
		ymax - ymin == window->height)
	{
		create_rect_by_coord (&windowlist[winId].wnd.position, xmin, ymin, xmax, ymax);
	} else {
		create_rect_by_coord (&windowlist[winId].wnd.position, 
             SCREEN_WIDTH / 2 - window->width / 2, SCREEN_HEIGHT / 2 - window->height / 2, 
             SCREEN_WIDTH / 2 + window->width / 2, SCREEN_HEIGHT / 2 + window->height / 2);
	}

	windowlist[winId].wnd.window_buf = window->window_buf;
    
	window->handler = winId;
	windowlist[winId].proc = myproc();
	windowlist[winId].wnd.minimized = 0;
	windowlist[winId].wnd.hasTitleBar = window->hasTitleBar;

	initMessageQueue (&windowlist[winId].wnd.msg_buf);

	uint len = strlen(title);
	if (len >= MAX_TITLE_LEN)
	{
		len = MAX_TITLE_LEN;
	}

	memmove (windowlist[winId].wnd.title, title, len);

	release (&wmlock);

	return 0;
}

/*
 *  createPopupWindow -- create a new popup window
 */
int 
createPopupWindow (window_p window, int caller)
{
	acquire(&wmlock);

    clear_window (window);

	if (popupwindow.caller != -1) {
		release(&wmlock);
		return 1;
	}

	int xmin = window->initialPosition.xmin;
	int xmax = window->initialPosition.xmax;
	int ymin = window->initialPosition.ymin;
	int ymax = window->initialPosition.ymax;

cprintf ("create popup %d %d\n", ymin, ymax);

	if (xmin >= 0 && xmax <= SCREEN_WIDTH &&
		ymin >= 0 && ymax <= SCREEN_HEIGHT &&
		xmax - xmin == window->width &&
		ymax - ymin == window->height) {

cprintf ("create popup - 2 %d %d\n", ymin, ymax);

		create_rect_by_coord (&popupwindow.wnd.position, xmin, ymin, xmax, ymax);
	}

	popupwindow.wnd.window_buf = window->window_buf;
	popupwindow.caller = caller;
	window->handler = caller;
	popupwindow.proc = myproc();
	popupwindow.wnd.minimized = 0;
	popupwindow.wnd.hasTitleBar = window->hasTitleBar;

	initMessageQueue(&popupwindow.wnd.msg_buf);

	release(&wmlock);

	return 0;
}

int 
closePopupWindow (window_p window)
{
	acquire(&wmlock);

	popupwindow.caller = -1;
	initMessageQueue(&popupwindow.wnd.msg_buf);
	window->handler = -1;
	memset(popupwindow.wnd.title, 0, MAX_TITLE_LEN);

	release(&wmlock);

	return 0;
}

int 
close_window(window_p window)
{

	acquire(&wmlock);

	int winId = window->handler;
	removeFromWindowList(winId);
	windowlist[winId].prev = winId;
	windowlist[winId].next = winId;

	initMessageQueue(&windowlist[winId].wnd.msg_buf);
	memset(windowlist[winId].wnd.title, 0, MAX_TITLE_LEN);

	if (winId == windowlisttail)
	{
		focusWindow(windowlist[winId].prev);
	}

	window->handler = -1;

	release(&wmlock);

	return 0;
}

int 
minimizeWindow(window_p window)
{

	acquire(&wmlock);

	int winId = window->handler;

	windowlist[winId].wnd.minimized = 1;
	if (winId == windowlisttail)
	{
		focusWindow(windowlist[winId].prev);
	}

	release(&wmlock);

	return 0;
}

int 
maximizeWindow(window_p window)
{

	acquire(&wmlock);

	int winId = window->handler;

	windowlist[winId].wnd.minimized = 0;
	focusWindow(winId);

	release(&wmlock);

	return 0;
}

//doesn't seem to work...
int 
turnoffScreen()
{
	acquire(&wmlock);

	for (int p = windowlisthead; p != -1; p = windowlist[p].next)
	{
		message newmsg;
		newmsg.msg_type = WM_WINDOW_CLOSE;
		dispatchMessage(&windowlist[p].wnd.msg_buf, &newmsg);
	}
	//memset(screen_buf, 255, screen_size);
	//memmove(screen, screen_buf, screen_size);

	release(&wmlock);

	return 0;
}

//system calls 
int 
sys_GUI_createPopupWindow()
{
	window *wnd;
	int caller;
	argptr(0, (char **)&wnd, sizeof(window));
	argint(1, &caller);
	return createPopupWindow(wnd, caller);
}

int 
sys_GUI_closePopupWindow()
{
	window *wnd;
	argptr(0, (char **)&wnd, sizeof(window));
	return closePopupWindow(wnd);
}

int 
sys_gui_createWindow ()
{
	window *wnd;
	char *title;
	argptr(0, (char **)&wnd, sizeof(window));
	argstr(1, &title);
	return create_window(wnd, title);
}

int 
sys_GUI_closeWindow()
{
	window *wnd;
	argptr(0, (char **)&wnd, sizeof(window));
	return close_window(wnd);
}

int 
sys_GUI_minimizeWindow()
{
	window *wnd;
	argptr(0, (char **)&wnd, sizeof(window));
	return minimizeWindow(wnd);
}

int 
sys_GUI_maximizeWindow()
{
	window *wnd;
	argptr(0, (char **)&wnd, sizeof(window));
	return maximizeWindow(wnd);
}

int 
sys_GUI_getMessage ()
{
	int h;
	message *res;

	argint (0, &h);

	argptr(1, (char **)(&res), sizeof(message));

	if (myproc() != windowlist[h].proc) {
		return 1;
	}

	return getMessage (&windowlist[h].wnd.msg_buf, res);
}

int 
sys_GUI_getPopupMessage()
{
	message *res;
	argptr(0, (char **)(&res), sizeof(message));
	if (popupwindow.caller == -1)
	{
		return 1;
	}
	return getMessage(&popupwindow.wnd.msg_buf, res);
}

int 
sys_GUI_updateScreen()
{
	update_screen ();
	return 0;
}

int 
sys_GUI_turnoffScreen()
{
	turnoffScreen();
	return 0;
}

int
sys_GUI_select_style ()
{
    int n;
  
    if(argint(0, &n) < 0)
        return -1;

    cprintf ("select style choosen %d\n", n);


    gui_style = n;



    message window_invalidate;
	window_invalidate.msg_type = WM_WINDOW_INVALIDATE;
	dispatchMessage (&windowlist[desktopId].wnd.msg_buf, &window_invalidate);




}

int
sys_GUI_get_style ()
{
    cprintf ("get selected style choosen\n");
    return gui_style;
}
