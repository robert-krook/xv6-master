
/*
 *  bash.c --   our implementation of the bash shell.
 */

#include "types.h"
#include "user.h"
#include "fcntl.h"

#include "stat.h"
#include "fs.h"

#include "hashtable.h"

#include "libc/stdlib.h"

#include "bash/constants.h"
#include "bash/shell.h"
#include "bash/history.h"

#include "bash/builtin_commands.h"
#include "bash/command_registry.h"

//char current_command[MAX_CMD_LEN];

extern char current_input_buffer [];
extern int current_input_length;
extern int current_cursor_pos;

char current_prompt[256];

int exiting = 0;

int command_mode = 0;
int in_input = 0;


void 
cleanup_background_processes (void) 
{
    exiting = 1;
    
    // int count = 0;

    // job_t *job_list = job_manager_get_all_jobs (&count);

    // for (int i = 0; i < count; i++) {
    //     pid_t pid = job_list[i].pid;
    //     kill(-pid, SIGTERM);
    //     waitpid(pid, NULL, 0);
    // }
}

void 
print_prompt (void) 
{
    char cwd [250];
    char prompt_buf [128];

    // Check if getcwd fails and report the error.
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
    //     perror("getcwd failed");
    //     // Fallback: use HOME or a known directory.
    //     char *home = getenv("HOME");
    //     if (home)
    //         strncpy(cwd, home, sizeof(cwd));
    //     else
             strcpy(cwd, "~");
    }

    sprintf (prompt_buf, "%s %s", cwd, "# ");
    strcpy (current_prompt, prompt_buf);
    printf(0, "%s", prompt_buf);
}

int
main (int argc,char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-c") == 0) 
        command_mode = 1;

    shell_init ();

    shell_loop ();

    exit ();
}

void 
shell_init (void) 
{
    // search path
    setenv ("PATH", "/bin/", 0);

    history_init ();
    history_load ();

    init_command_registry();
    register_builtin_commands();
}

void 
shell_loop (void) 
{
    char *input;
    command_t *cmd;
    int status = 1;

    while (status && !exiting) {

        // Show the prompt
        print_prompt ();

        current_input_length = 0;
        current_input_buffer[0] = '\0';

        in_input = 1;
        input = read_input ();
        in_input = 0;

        if (!input) { 
            printf(0, "\n"); 
            break; 
        }

        if (strlen(input) == 0) { 
            free(input); 
            continue; 
        }

        cmd = parse_input (input);
        if (cmd) {
              if (cmd->type == CMD_IF || cmd->type == CMD_WHILE || cmd->type == CMD_FOR ||
                cmd->type == CMD_CASE || cmd->type == CMD_SUBSHELL ||
                cmd->type == CMD_AND || cmd->type == CMD_OR || cmd->type == CMD_SEQUENCE) {
                    execute_command(cmd);
            } else if (cmd->command) {
                if (strcmp(cmd->command, "exit") == 0) { 
                    cleanup_background_processes (); 
                    status = 0; 
                } else { 
                    strncpy (current_command, cmd->args[0], MAX_CMD_LEN - 1); 
                    execute_command (cmd); 
                }
            }
        }

        free (input);
        command_free (cmd);
    }

}
