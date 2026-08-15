

#include "types.h"
#include "user.h"

#include "bash/constants.h"
#include "bash/history.h"

static char *history_list[MAX_HISTORY];

static int history_count = 0;

void 
history_init(void) 
{
    memset (history_list, 0, sizeof (history_list));
    history_count = 0;
}

void 
history_load (void) 
{
    // char *home = getenv("HOME");
    // if (!home) return;

    // char history_path[PATH_MAX];
    // snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE);
    
    // FILE *fp = fopen(history_path, "r");
    // if (!fp) return;

    // char line[SHELL_MAX_INPUT];
    // while (fgets(line, sizeof(line), fp)) {
    //     size_t len = strlen(line);
    //     if (len > 0 && line[len-1] == '\n') {
    //         line[len-1] = '\0';
    //     }
    //     history_add(line);
    // }
    // fclose(fp);
}
