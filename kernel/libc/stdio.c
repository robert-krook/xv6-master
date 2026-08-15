
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "defs.h"
#include "proc.h"
#include "fcntl.h"
#include "fs.h"
#include "stat.h"
#include "file.h"

extern int sys_read (void);

int
argfd (int fd, struct file **pf)
{
    struct file *f;

    if (fd < 0 || fd >= NOFILE || (f=proc->ofile[fd]) == 0)
        return -1;

    // if (pfd)
    //     *pfd = fd;

    if(pf)
        *pf = f;

    return 0;
}

// int 
// read (int fd, void *buffer, int n);
// {

// }

int
fdalloc (struct file *f)
{
    int fd;

    for (fd = 0; fd < NOFILE; fd++) {
        if (proc->ofile[fd] == 0) {
            proc->ofile[fd] = f;
        return fd;
        }
    }

    return -1;
}

int
fopen (char *path, int omode)
{
//    char *path;
    int fd; //, omode;
    struct file *f;
    struct inode *ip;

    // if (argstr (0, &path) < 0 || argint(1, &omode) < 0)
    //     return -1;

cprintf ("fopen\n");
cprintf ("%s %d\n", path, omode);

    if (omode & O_CREATE) {
        // begin_trans();
        // ip = create (path, T_FILE, 0, 0);
        // commit_trans ();
        // if(ip == 0)
        //     return -1;
    } 
    else {
        if ((ip = namei (path)) == 0)
            return -1;

cprintf ("%s %d\n", path, omode);

        ilock(ip);

        if (ip->type == T_DIR && omode != O_RDONLY) 
        {
            iunlockput (ip);
            return -1;
        }
    }

cprintf ("%s %d\n", path, omode);

    if ((f = filealloc ()) == 0 || (fd = fdalloc (f)) < 0) {
        if (f)
            fileclose (f);
        iunlockput (ip);
        return -1;
    }

    iunlock(ip);

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    return fd;
}

int
fgetline (int fd, unsigned char *line, int size)
{
    unsigned char ch [1];
    int index = 0;
    int ret = 0;

    struct file *f;

    argfd (fd, &f);

    while ((ret=fileread (f, ch, 1))>0) {

        if (ch[0]==10 || ch[0]==13) {
            break;
        }

        *(line + index++) = ch[0];
    }
    
    if (ret<=0)
        return -1;

    *(line + index) = 0;

//cprintf ("line = %s\n", line);

    return index;
}

int
fclose (int fd)
{
    struct file *f;
  
    // if(argfd(0, &fd, &f) < 0)
    //     return -1;
    
    proc->ofile[fd] = 0;
    
    fileclose (f);

    return 0;
}

int
fread (int fd, char *p, int n)
{
    struct file *f;
    //int n;
    //char *p;

    argfd (fd, &f);

    // Check for correct arguments
    // if (argfd (0, 0, &f) < 0 || argint (2, &n) < 0 || argptr (1, &p, n) < 0)
    //     return -1;
    
    return fileread (f, p, n);
}

