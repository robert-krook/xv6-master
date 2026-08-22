
#include "types.h"
#include "user.h"
#include "hashtable.h"

#include "bash/shell.h"

#include "bash/command_registry.h"

char current_command[MAX_CMD_LEN];

// Our Environment variables
ht * environmentVariables2 = NULL;       // Change the structure name into 
                                        // something better

int
unsetenv (const char *name)
{
    setenv (name, "", 1);
    return 0;
}

int
setenv (const char *name, const char *value, int overwrite)
{
    if (environmentVariables2==NULL)
        environmentVariables2 = ht_create ();
    
    if (environmentVariables2!=NULL) {
        ht_set (environmentVariables2, name, value);
    }

    return 0;
}

char *
getenv (const char *name)
{
    //printf (0, "getenv\n");
    if (environmentVariables2!=NULL) {
        return ht_get (environmentVariables2, name);
    }
    return "";
}

void
show_env_variables ()
{
  if (environmentVariables2!=NULL) {
        hti _hti;
        _hti = ht_iterator (environmentVariables2);

        while (ht_next (&_hti)) {
            printf (2, "%s=%s\n", _hti.key, _hti.value);
        }
    }
}

void 
kill_job (int job_id) 
{
    // int pid = get_pid_by_job_id(job_id);
    // if (pid > 0) {
    //     if (kill(-pid, SIGTERM) < 0)
    //         printf (2, "kill (SIGTERM)");
    // } else {
    //     printf(2, "Invalid job number: %d\n", job_id);
    // }
}

void 
execute_command (command_t *cmd) 
{
    if (!cmd) 
        return;

    // Find the external program in the enviroment parameter 'PATH'.
    char *path = getenv ("PATH");
    char total_path [256];

    //printf (0, "%s\n", path);

    if (!cmd->args[0]) 
        return;

    const command_entry_t *entry = lookup_command (cmd->args[0]);
    if (entry) {
        entry->func(cmd);
        return;
    }


    if (sprintf (total_path, "%s%s", path, cmd->args[0])>0) {
    }

    int pid = fork ();
        if (pid==0) {
        // child process
        if (exec (total_path, cmd->args) == -1) {
            printf (2, "%s: command not found\n", cmd->args [0]);
            exit ();
        }
    } else if (pid > 0) {
        wait();
        strncpy(current_command, cmd->args[0], MAX_CMD_LEN - 1);
        current_command[MAX_CMD_LEN - 1] = '\0';
    }

}


void 
command_free (command_t *cmd) 
{    
    if (!cmd) 
        return;

    if (cmd->command) 
        free(cmd->command);

    if (cmd->args) {
        for (int i = 0; i < cmd->arg_count; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }

    // if (cmd->input_file) free(cmd->input_file);
    // if (cmd->output_file) free(cmd->output_file);
    // if (cmd->if_condition) free(cmd->if_condition);
    // if (cmd->then_branch) command_free(cmd->then_branch);
    // if (cmd->else_branch) command_free(cmd->else_branch);
    // if (cmd->while_condition) free(cmd->while_condition);
    // if (cmd->while_body) command_free(cmd->while_body);
    // if (cmd->for_variable) free(cmd->for_variable);
    // if (cmd->for_list) {
    //     for (int i = 0; cmd->for_list[i]; i++) {
    //         free(cmd->for_list[i]);
    //     }
    //     free(cmd->for_list);
    // }
    // if (cmd->for_body) command_free(cmd->for_body);
    // if (cmd->type == CMD_SUBSHELL && cmd->subshell_cmd) {
    //     command_free(cmd->subshell_cmd);
    // }
    // if (cmd->case_expression) free(cmd->case_expression);
    // if (cmd->case_entries) {
    //     for (int i = 0; i < cmd->case_entry_count; i++) {
    //         if (cmd->case_entries[i]) {
    //             if (cmd->case_entries[i]->pattern) 
    //                 free(cmd->case_entries[i]->pattern);
    //             if (cmd->case_entries[i]->body)
    //                 command_free(cmd->case_entries[i]->body);
    //             free(cmd->case_entries[i]);
    //         }
    //     }
    //     free(cmd->case_entries);
    // }


    if (cmd->next) 
        command_free(cmd->next);

    free(cmd);
}
