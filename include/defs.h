
/*
 *  defs.h -- function prototypes.
 */

//#ifndef __ASSEMBLER__

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct stat;
struct superblock;
struct cmos_time_struct;

struct message;

// msg.c
int             handleMessage (struct message * msg);

// window_manager.c
void            wmInit(void);
void            wmHandleMessage(struct message *);

// mouse.c
void            mouseinit(void);
void            mouseintr(uint);

// bio.c
void            binit(void);
struct buf*     bread(uint dev, uint sector);
void            brelse(struct buf*);
void            bwrite(struct buf*);

// console.c
void            consoleinit(void);
void            kprintf(char*, ...);
void            cprintf(char*, ...);
void            consoleintr(int(*)(void));
void            panic(char*) __attribute__((noreturn));

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);

// fs.c
void            readsb(int dev, struct superblock *sb);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit(void);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint, uint);
void            show_disk_free (void);
int             df (void);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);
extern uchar    ioapicid;
void            ioapicinit(void);

// kalloc.c
char*           kalloc(void);
void            kfree(char*);
void            kinit1(void*, void*);
void            kinit2(void*, void*);

// kbd.c
void            kbdintr(void);

// lapic.c
int             cpunum(void);
extern volatile uint*    lapic;
void            lapiceoi(void);
void            lapicinit(void);
void            lapicstartap(uchar, uint);
void            microdelay(int);
int             lapicid(void);

// log.c
void            initlog(void);
void            log_write(struct buf*);
void            begin_trans();
void            commit_trans();

// mp.c
extern int      ismp;
int             mpbcpu(void);
void            mpinit(void);
void            mpstartthem(void);

// apic.c
int             acpiinit(void);

// picirq.c
void            picenable(int);
void            picinit(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

// proc.c
struct proc*    copyproc(struct proc*);
void            exit(void);
int             fork(void);
int             growproc(int);
int             kill(int);
void            pinit(void);
void            procdump(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(void);
void            wakeup(void*);
void            yield(void);
struct cpu      *mycpu(void);
struct proc     *myproc();
int             ps (void);

// swtch.S
void            swtch(struct context**, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
void            getcallerpcs(void*, uintp*);
void            getstackpcs(uintp*, uintp*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            pushcli(void);
void            popcli(void);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);
char *          strcpy (char *dst, char *src);

// syscall.c
int             argint(int, int*);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             arguintp(int, uintp*);
int             fetchuintp(uintp, uintp*);
int             fetchstr(uintp, char**);
void            syscall(void);

// timer.c
void            timerinit(void);

// trap.c
void            idtinit(void);
extern uint     ticks;
void            tvinit(void);
extern struct spinlock tickslock;

// uart.c
void		uartearlyinit(void);
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);

// vm.c and vm64.c
void            seginit(void);
void            kvmalloc(void);
void            vmenable(void);
pde_t*          setupkvm(void);
char*           uva2ka(pde_t*, char*);
int             allocuvm(pde_t*, uint, uint);
int             deallocuvm(pde_t*, uintp, uintp);
void            freevm(pde_t*);
void            inituvm(pde_t*, char*, uint);
int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
pde_t*          copyuvm(pde_t*, uint);
void            switchuvm(struct proc*);
void            switchkvm(void);
int             copyout(pde_t*, uint, void*, uint);
void            clearpteu(pde_t *pgdir, char *uva);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

//      graphics.c -- GUI VGA funtions

//      32 bit RGBA. used above GUI Utility
typedef struct RGBA {
    // Standard RGBA color (SVGA and higher for example)
    unsigned char A;
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGBA;

        // 24 bit RGB. used in GUI Utility
typedef struct RGB {
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGB;

int		init_graphics(void);
int		exit_graphics(void);
int		vga_getkey(void);
int		vga_clear_screen(void);
int		vga_draw_pixel(int, int, int);
int		vga_draw_line(int, int, int, int, int);
int		vga_blit(void);
void    vga_draw_image (RGBA *img, int x, int y, int width, int height, int max_x, int max_y);
int		is_graphics(void);
void	vga_graphicsintr(int (*) (void));
int     vga_draw_character(int x, int y, char ch, int color);
void    vga_draw_string(int x, int y, char *str, RGBA color);
void    vga_draw_string_with_max_width(int x, int y, int width, char *str, RGBA color);
int     vga_draw_icon(int x, int y, int icon, RGBA color);
void    vga_draw_mouse(int mode, int x, int y);
void    draw_rect_bound(int x, int y, int width, int height, RGBA fill, int max_x, int max_y);
int     sys_init_graphics(void);
void    vga_draw_rect_by_coord(int xmin, int ymin, int xmax, int ymax, RGBA fill);
void    vga_draw_rect(int x, int y, int width, int height, RGBA fill);
void    wmInit();
void    updateScreen();
void    vga_draw_rect_border (int color, int x, int y, int width, int height);
void    vga_draw_24_image_part (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, int subh, char *title);
int     vga_init_graphics (void);

//      cmos.c functions
void    cmostst ();
void    cmostime (struct cmos_time_struct *r);

//#endif

void    search_madt ();

int     chpr (int pid, int priority);
