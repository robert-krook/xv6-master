
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "param.h"

#include "process_info.h"

enum procstate { UNUSED, EMBRYO, BLOCKED, READY, ACTIVE, ZOMBIE };
  static char *states[] = {
    [UNUSED]    "UNUSED  ",
    [EMBRYO]    "EMBRYO  ",
    [BLOCKED]  "BLOCKED",
    [READY]  "READY",
    [ACTIVE]     "ACTIVE   ",
    [ZOMBIE]     "ZOMBIE   "
    };

int main(int argc, char *argv[])
{
    struct process_info processInfoTable [NPROC];

    int numbers = get_processes_info (processInfoTable);
    int lineNumber;

    printf(1, "\n  PID\tSTATUS\t\tMEM\tPROC");
    printf(1, "\n  ---\t------\t\t---\t----\n");
   
    for (int i = 0; i < numbers; i++) {
        lineNumber = i + 1;
        printf(1, "%d %d\t%s\t%d\t%s",
        lineNumber,
        processInfoTable[i].pid,
        states[processInfoTable[i].state],
        processInfoTable[i].sz,
        processInfoTable[i].name);
        printf(1, "\n");
    }

    exit();
}
