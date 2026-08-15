
/*
 *  init.c - The initial user-level program
 *
 *  This is first process started by the kernel. It starts in the end the shell.
 *
 */ 

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv [] = { "bash", 0 };

void spawnshell (char *devname)
{
    int pid, wpid;

    open (devname, O_RDWR);

    dup(0);  // stdout
    dup(0);  // stderr

    for(;;) {
        printf (1, "init: starting /bin/sh\n");
        pid = fork ();
        if(pid < 0) {
            printf(1, "init: fork failed\n");
            exit();
        }

        if(pid == 0) {
            exec("/bin/bash", argv);
            printf(1, "init: exec /bin/sh failed\n");
            exit();
        }

        while((wpid=wait()) >= 0 && wpid != pid)
            ; // Clean up zombies
    }

}

int
main2(void)
{

  if(open("/dev/console", O_RDWR) < 0){
    mknod("/dev/console", 1, 0);
  }
  if(open("/dev/serial", O_RDWR) < 0){
    mknod("/dev/serial", 1, 1);
  }
  if(open("/dev/disk0", O_RDWR) < 0){
    mknod("/dev/disk0", 2, 0);
  }
  if(open("/dev/disk1", O_RDWR) < 0){
    mknod("/dev/disk1", 2, 1);
  }
  if(open("/dev/null", O_RDWR) < 0){
    mknod("/dev/null", 3, 0);
  }
  if(open("/dev/zero", O_RDWR) < 0){
    mknod("/dev/zero", 3, 1);
  }
  switch(fork()){
    case 0:  spawnshell("/dev/console");
    default: spawnshell("/dev/serial");
  }
}

int
main (void)
{
    int pid, wpid;

    // Create a pipe for communication with the console
    if (open("/dev/console", O_RDWR) < 0) {

        // Create a pipe for the console
        mknod ("/dev/console", 1, 1);
        
        // Open console for communication
        open ("/dev/console", O_RDWR);
    }

    dup (0); // stdout
    dup (0); // stderr

    for(;;) {

        printf(1, "init: starting sh\n");

        pid = fork();

        if(pid < 0) {
            printf(1, "init: fork failed\n");
            exit();
        }

        if(pid == 0) {
            // exec("sh", argv); -- old version
            // Start our own version of the shell with internal
            // commands.
            exec ("/bin/bash", argv);
            printf (1, "init: exec bash failed\n");
            exit ();
        }

        while ( ( wpid=wait () ) >= 0 && wpid != pid )
            printf (1, "zombie!\n");
        
        printf (1, "zombie out!\n");


    }
}
