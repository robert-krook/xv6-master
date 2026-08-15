#include "types.h"
#include "stat.h"
#include "user.h"

#include "param.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

  int fd;


void
writetest (void)
{
    if (write(fd, "aaaaaaaaaa", 10) != 10){
        //exit();
    }  
}

int
main(int argc, char *argv[])
{
    int i = 0;

    // fd = open("small", O_CREATE|O_RDWR);
    // if(fd >= 0) {
    // } else {
    //   exit();
    // }

    while (i < 5) {
        //writetest ();
        printf (1, "Daemon\n");
        sleep (5);
        i++;
    };

    //close(fd);

    exit();
}
