

#include "gui/graphics.h"
#include "gui/bitmap.h"
#include "gui/user_window.h"


struct stat;
struct cmos_time_struct;
struct RGBA;
struct RGB;
struct message;
struct Widget;
struct window;


typedef void (*Handler)(struct Widget *, struct message *);

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read (int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod (char *, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

int ps (void);
int getcwd (void *, int);
int df (void);

// ulib.c
int     stat(char*, struct stat*);
char *  strncpy (char *s, const char *t, int n);
char *  strcpy (char *dst, char *src);              // String copy
void *  memmove(void*, void*, int);                 // Memory move
char *  strchr(const char*, char c);                 
int     strcmp(const char*, const char*);           // String compare
void    printf(int, char *, ...);
int     sprintf(char *, char*, ...);
char *  gets(char*, int max);
int     getchar (void);

uint    strlen(char*);
void *  memset(void*, int, uint);
void *  malloc(uint);
void    free(void*);
int     atoi(const char*);
int     strncmp(const char *p, const char *q, uint n);

//custom system calls
int init_graphics(void);
int draw_pixel(int, int, int);
int blit(void);
int clear_screen(void);
int exit_graphics(void);
int draw_line(int, int, int, int, int);
int getkey(void);


// window_manager.c
int GUI_createPopupWindow(struct window *, int);
int GUI_closePopupWindow(struct window *);
int GUI_createWindow(struct window *, const char *);
int GUI_closeWindow(struct window *);
int GUI_maximizeWindow(struct window *);
int GUI_minimizeWindow(struct window *);
int GUI_getMessage (int, struct message *);
int GUI_getPopupMessage(struct message *);
void GUI_updateScreen();
void GUI_turnoffScreen();
int GUI_select_style (int style);
int GUI_get_style (void);

// user_window.c
void debugPrintWidgetList(struct window *win);
void createPopupWindow(struct window *, int);
void closePopupWindow(struct window *);
void create_window (struct window *, const char *);
void closeWindow(struct window *);
void updateWindow(struct window *);
void updatePopupWindow(struct window *);
    
int addButtonWidget(struct window *win, struct RGBA c, struct RGBA bc, struct RGBA border_color, 
    char *text, int x, int y, int w, int h, int, Handler handler);
int addImageWidget (struct window *win, struct RGB *image, int x, int y, int w, int h, Handler handler, unsigned char *name);

    int addTextWidget(struct window *win, struct RGBA c, char *text, int x, int y, int w, int h, int, Handler handler);
    int addInputFieldWidget(struct window *win, struct RGBA c, char *text, int x, int y, int w, int h, int, Handler handler);
    int addColorFillWidget(struct window *win, struct RGBA c, int x, int y, int w, int h, int, Handler handler, char *title);
    int addRectangleWidget(struct window *win, struct RGBA c, struct RGBA filledColor, int filled, int x, int y, int w, int h, int scrollable, Handler handler);
    int removeWidget(struct window *win, int index);
    int setWidgetHandler(struct window *win, int index, Handler handler);

// user_handler.c
void emptyHandler (struct Widget *w, struct message *msg);
int getInputOffsetFromMousePosition(char *str, int width, int mouse_x, int mouse_y);
int getMouseXFromOffset(char *str, int width, int offset);
int getMouseYFromOffset(char *str, int width, int offset);
void inputMouseLeftClickHandler(struct Widget *w, struct message *msg);
void inputFieldKeyHandler(struct Widget *w, struct message *msg);

int getScrollableTotalHeight(struct window *win);
int addScrollBarWidget(struct window *window, struct RGBA color, Handler handler);


void draw24Image(struct window *win, struct RGB *img, int x, int y, int width, int height);

 int date(struct cmos_time_struct *r);

void 
drawString (struct window *win, char *str, struct RGBA color, int x, int y, int width, int height);

// bitmap.c
int readBitmapFile(char *, struct RGBA *, int *, int *);
int read24BitmapFile(char *fileName, RGB *result, int *height, int *width);
void readBitmapHeader(int bmpFile, BITMAP_FILE_HEADER *bmpFileHeader, BITMAP_INFO_HEADER *bmpInfoHeader);
int add_image (window *win, unsigned char *filename, unsigned char *name, int x, int y, Handler handler, int *w, int *h);

int chpr (int pid, int priority);

