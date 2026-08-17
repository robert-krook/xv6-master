/*
 *  sysproc.c - ...
 */

#include "types.h"
#include "x86.h"
#include "date.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#include "gui/gui.h"
#include "bga.h"

#include "process_info.h"

int
sys_chpr (void)
{
    int pid, pr;

    if (argint (0, &pid) < 0)
        return -1;
    
    if (argint (1, &pr) < 0)
        return -1;
    
    return chpr (pid, pr);
}

int
sys_fork (void)
{
    return fork();
}

int
sys_exit (void)
{
    exit();
    return 0;  // not reached
}

int
sys_wait (void)
{
    return wait();
}

int
sys_kill (void)
{
    int pid;

    if (argint (0, &pid) < 0)
        return -1;

    return kill (pid);
}

int
sys_getpid (void)
{
    return proc->pid;
}

uintp
sys_sbrk (void)
{
  uintp addr;
  uintp n;

  if(arguintp(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep (void)
{
    int n;
    uint ticks0;
  
    if(argint(0, &n) < 0)
        return -1;

    acquire(&tickslock);

    ticks0 = ticks;

    while(ticks - ticks0 < n) {

        if(proc->killed) {
            release (&tickslock);
            return -1;
        }
        
        sleep (&ticks, &tickslock);
    }

    release (&tickslock);

    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime (void)
{
    uint xticks;
  
    acquire(&tickslock);
    xticks = ticks;
    release (&tickslock);
    return xticks;
}

//  return the system call function from file proc.c
int 
sys_ps (void)
{
    return ps();
}

int
sys_draw_pixel (void)
{
	int a;
	int b;
	int c;

	if(argint(0, &a) < 0)
		return -1;
	if(argint(1, &b) < 0)
		return -1;
	if(argint(2, &c) < 0)
		return -1;
	
#ifdef __VGA__
	return vga_draw_pixel(a, b, c);
#else
    return vga_draw_pixel(a, b, c);
#endif
}

int
sys_blit(void)
{
#ifdef __VGA__    
	return vga_blit ();
#else
    return bga_blit ();    
#endif
}

int
sys_clear_screen(void)
{
	return vga_clear_screen();
}

int
sys_draw_line(void)
{
	int a;
	int b;
	int c;
	int d;
	int e;

	if(argint(0, &a) < 0)
		return -1;
	if(argint(1, &b) < 0)
		return -1;
	if(argint(2, &c) < 0)
		return -1;
	if(argint(3, &d) < 0)
		return -1;
	if(argint(4, &e) < 0)
		return -1;
	
	return vga_draw_line(a, b, c, d, e);
}

int
sys_getkey (void)
{
	return vga_getkey ();
}

int 
sys_date (void)
{
    struct cmos_time_struct *r;
    if(argptr(0, (void*)&r, sizeof(r)) < 0)
        return -1;
    cmostime(r);
    return 0;
}

int
sys_df (void)
{

    //unsigned char * lfb = (unsigned char *) (0x4000000A);
    //unsigned char * lfb = (unsigned char *) (0xFFFFFFFFFD000000);

// for (int i=100; i<200; i++)

//   *(lfb+i) = 50;


    return df ();
}

int
sys_init_graphics (void)
{
    cprintf ("sys_init_graphics\n");

// #ifdef __VGA__
//         vga_init_graphics ();
// #else
//         bga_init ();
// #endif

gui_init_graphics ();
	return 0; 
}

// (...)
int 
sys_get_processes_info (void)
{
    struct process_info *process_info_table;

    if (argptr (0,(char**) &process_info_table, sizeof (struct process_info) * NPROC) < 0) {
        return -1;
    }

    return get_processes_info (process_info_table);
}
