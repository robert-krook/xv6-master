/*
 *  proc.c -- handle processes.
 */

#include "types.h"

#include "process_info.h"

#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"


struct {
    struct spinlock lock;
    struct proc proc [NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit (void)
{
    initlock (&ptable.lock, "ptable");
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *
mycpu (void)
{
    int apicid, i;
  
    if (readeflags() & FL_IF)
        panic("mycpu called with interrupts enabled\n");
  
    apicid = lapicid();

    // APIC IDs are not guaranteed to be contiguous. Maybe we should have
    // a reverse map, or reserve a register to store &cpus[i].
    for (i = 0; i < ncpu; ++i) {
        if (cpus[i].apicid == apicid)
            return &cpus[i];
    }

    panic("unknown apicid\n");

}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc (void) 
{
//    struct cpu *c;
//    struct proc *p;

//     pushcli();

//     c = mycpu();
//     p = c->proc;

// //cprintf("User Process %s\n", proc->name);

//     popcli();
    //return p;
    return proc;
}

/*
 * Look in the process table for an UNUSED proc.
 * If found, change state to EMBRYO and initialize
 * state required to run in the kernel.
 * Otherwise return 0.
 */
static struct proc *
allocproc (void)
{
    struct proc *p;
    char *sp;

    acquire(&ptable.lock);

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if(p->state == UNUSED)
            goto found;

    release (&ptable.lock);

    return 0;

found:
    p->state = EMBRYO;
    p->pid = nextpid++;
    p->priority = 10;           // Default priority

    release(&ptable.lock);

    // Allocate kernel stack.
    if((p->kstack = kalloc()) == 0) 
    {
        p->state = UNUSED;
        return 0;
    }

    sp = p->kstack + KSTACKSIZE;
  
    // Leave room for trap frame.
    sp -= sizeof *p->tf;
    p->tf = (struct trapframe*)sp;
  
    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= sizeof(uintp);
    *(uintp*)sp = (uintp)trapret;

    sp -= sizeof *p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uintp)forkret;

    return p;
}

void map_bga (pde_t *pgdir);

// Set up first user process.
void
userinit (void)
{
    struct proc *p;

    extern char _binary_out_initcode_start[], 
                _binary_out_initcode_size[];
  
    p = allocproc ();

    initproc = p;

    if ((p->pgdir = setupkvm ()) == 0) {
        cprintf("\nuserinit()-1e\n");    
        panic("userinit: out of memory?");
    }

    inituvm (p->pgdir, _binary_out_initcode_start, (uintp)_binary_out_initcode_size);

    p->sz = PGSIZE;

    memset (p->tf, 0, sizeof(*p->tf));

    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;

#ifndef X64
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
#endif

    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of initcode.S

    safestrcpy (p->name, "initcode", sizeof(p->name));
  
    p->cwd = namei ("/");

    p->state = RUNNABLE;

    //map_bga (p->pgdir);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc (int n)
{
    uint sz;
  
    sz = proc->sz;
    if(n > 0) {
        if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    } else if(n < 0) {
        if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    }

    proc->sz = sz;

    switchuvm (proc);

    return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork (void)
{
    int i, pid;
    struct proc *np;

    // Allocate process.
    if((np = allocproc()) == 0)
        return -1;

    // Copy process state from p.
    if((np->pgdir = copyuvm (proc->pgdir, proc->sz)) == 0) 
    {
        kfree (np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }

    np->sz = proc->sz;
    np->parent = proc;
    *np->tf = *proc->tf;

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for( i = 0; i < NOFILE; i++)
        if (proc->ofile[i])
            np->ofile[i] = filedup (proc->ofile[i]);

    np->cwd = idup(proc->cwd);
 
    pid = np->pid;

    np->state = RUNNABLE;

    safestrcpy(np->name, proc->name, sizeof(proc->name));

    //cprintf ("pid = %d\n", pid);

    return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit (void)
{
    struct proc *p;
    int fd;

    if(proc == initproc)
        panic("init exiting");

    // Close all open files.
    for(fd = 0; fd < NOFILE; fd++)
    {
        if(proc->ofile[fd])
        {
            fileclose(proc->ofile[fd]);
            proc->ofile[fd] = 0;
        }
    }

    iput (proc->cwd);
    proc->cwd = 0;

    acquire (&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1 (proc->parent);

    // Pass abandoned children to init.
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {

        if(p->parent == proc) {

            p->parent = initproc;

            if(p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }

    // Jump into the scheduler, never to return.
    proc->state = ZOMBIE;

    sched();
    
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait (void)
{
    struct proc *p;
    int havekids, pid;

    acquire(&ptable.lock);

    for(;;)
    {
        // Scan through table looking for zombie children.
        havekids = 0;
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        {
            if(p->parent != proc)
                continue;
            havekids = 1;
            if(p->state == ZOMBIE)
            {
                // Found one.
                pid = p->pid;
                kfree(p->kstack);
                p->kstack = 0;
                freevm(p->pgdir);
                p->state = UNUSED;
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                release(&ptable.lock);
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if(!havekids || proc->killed)
        {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);  //DOC: wait-sleep
    }
}

// Change the priority of the process.
int
chpr (int pid, int priority)
{
    struct proc *p;

    acquire (&ptable.lock);

    for (p=ptable.proc; p<&ptable.proc [NPROC]; p++) {
        if (p->pid == pid ) {
            p->priority = priority;
            break;
        }
    }

    release (&ptable.lock);

    return 0;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler (void)
{
    struct proc *p = 0;
    struct proc *p1 = 0;

    for(;;) {

        // Enable interrupts on this processor.
        sti ();

        // no runnable processes? (did we hit the end of the table last time?)
        // if so, wait for irq before trying again.
        if (p == &ptable.proc [NPROC])
            hlt ();

// save fpu
//asm volatile("fxsave %0" : "=m"(myproc()->fpu_state) : : "memory");

struct proc *highP = nullptr;

        // Loop over process table looking for process to run.
        acquire (&ptable.lock);

        for(p = ptable.proc; p < &ptable.proc [NPROC]; p++) {

            if(p->state != RUNNABLE)
                continue;

            // Implement a priority algoritmen
highP = p;
// choose one with the highest priority
for (p1 = ptable.proc; p1 < &ptable.proc [NPROC]; p1++) {

    if (p1->state != RUNNABLE)
        continue;

    if (highP->priority > p1->priority) {
        highP = p1;
        //cprintf ("High=%s\n", highP->name);
    }

}

p = highP;

            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.
            proc = p;

            switchuvm (p);

  //asm volatile("fxrstor %0" : : "m"(p->fpu_state) : "memory");

            p->state = RUNNING;

            swtch (&cpu->scheduler, proc->context);

            switchkvm ();

            // Process is done running for now.
            // It should have changed its p->state before coming back.
            proc = 0;
        }

        release (&ptable.lock);
    }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched (void)
{
    int intena;

    if (!holding (&ptable.lock))
        panic("sched ptable.lock");

    if (cpu->ncli != 1)
        panic("sched locks");

    if (proc->state == RUNNING)
        panic("sched running");

    if (readeflags()&FL_IF)
        panic("sched interruptible");
    
    intena = cpu->intena;

    swtch (&proc->context, cpu->scheduler);

    cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield (void)
{
    acquire (&ptable.lock);  //DOC: yieldlock
    proc->state = RUNNABLE;
    sched ();
    release (&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret (void)
{
    static int first = 1;
    
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock);

    if (first) 
    {
        // Some initialization functions must be run in the context
        // of a regular process (e.g., they call sleep), and thus cannot 
        // be run from main().
        first = 0;
        initlog();
    }
  
    // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep (void *chan, struct spinlock *lk)
{
    if(proc == 0)
        panic("sleep");

    if(lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if(lk != &ptable.lock) {  //DOC: sleeplock0
        acquire(&ptable.lock);  //DOC: sleeplock1
        release(lk);
    }

    // Go to sleep.
    proc->chan = chan;
    proc->state = SLEEPING;
    sched();

    // Tidy up.
    proc->chan = 0;

    // Reacquire original lock.
    if(lk != &ptable.lock) {  //DOC: sleeplock2
        release(&ptable.lock);
        acquire(lk);
    }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1 (void *chan)
{
    struct proc *p;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if(p->state == SLEEPING && p->chan == chan)
            p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup (void *chan)
{
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill (int pid)
{
    struct proc *p;

    acquire(&ptable.lock);

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
        if(p->pid == pid)
        {
            p->killed = 1;
            // Wake process from sleep if necessary.
            if(p->state == SLEEPING)
                p->state = RUNNABLE;
            release(&ptable.lock);
            return 0;
        }
    }

    release(&ptable.lock);

    return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump (void)
{
    static char *states[] = 
    {
        [UNUSED]    "unused",
        [EMBRYO]    "embryo",
        [SLEEPING]  "sleep ",
        [RUNNABLE]  "runble",
        [RUNNING]   "run   ",
        [ZOMBIE]    "zombie"
    };
  
    int i;
    struct proc *p;
    char *state;
    uintp pc[10];
  
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
        if(p->state == UNUSED)
            continue;

        if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";

        cprintf("%d %s %s", p->pid, state, p->name);
        if(p->state == SLEEPING)
        {
            getstackpcs((uintp*)p->context->ebp, pc);
            for(i=0; i<10 && pc[i] != 0; i++)
            cprintf(" %p", pc[i]);
        }
        cprintf("\n");
    }
}


/* 
    my psc - print current process status + PPID
    INIT PPID is 1 and not the value saved in the PCB (under parant's pid)
    so additional conditdional expression was added to print it:
    if the process's PID is 1 than the process must be the INIT, and the correct PPID will be printed.
*/
int 
ps (void)
{
    struct proc *p;

    sti();

    acquire (&ptable.lock);
  
    cprintf ("PID \t STATE\t\t PPID \t CMD\n");

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == SLEEPING)
            cprintf("%d \t SLEEPING \t %d \t %s \n", p->pid, (p->pid == 1) ? 1 : p->parent->pid, p->name);
        else if ( p->state == RUNNING )
            cprintf("%d \t RUNNING \t %d \t %s \t %d \n", p->pid, (p->pid == 1) ? 1 : p->parent->pid, p->name, p->priority);
        else if ( p->state == RUNNABLE )
            cprintf("%d \t RUNNABLE \t %d \t %s \t %d \n", p->pid, (p->pid == 1) ? 1 : p->parent->pid, p->name, p->priority);
    }

    release(&ptable.lock);

    return 192;
}

// (...)
int 
get_processes_info (struct process_info *process_info_table) 
{
    struct proc *p;  
    int count = 0;
    int i;

    for (i = 0, p = ptable.proc; p < &ptable.proc[NPROC] && i < NPROC; i++,p++) {
        
        if(p->state == UNUSED) {
            continue;
        }

        count++;

        process_info_table[i].pid = p->pid;
        
        if(i == 0) {
            process_info_table[i].ppid = 0;
        } else {
            process_info_table[i].ppid = p->parent->pid;
        }
   
        process_info_table[i].state = p->state;
        process_info_table[i].sz = p->sz;

        for (int j = 0; j < 16; j++) {
            process_info_table[i].name[j] = p->name[j];
        }
    }

    p = nullptr;

    return count;
}
