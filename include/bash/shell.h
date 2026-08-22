#ifndef SHELL_H
#define SHELL_H

#include "constants.h"
//#include "rc.h"
#include "history.h"
//#include "alias.h"
#include "command.h"

// Current command being executed
extern char current_command [MAX_CMD_LEN];

int setenv (const char *name, const char *value, int overwrite);
int unsetenv (const char *name);
void show_env_variables ();

char *read_input( void);                    // Reads a line of input from the user
command_t *parse_input (char *input);       // Parses input string into command structure
void execute_command(command_t *cmd);       // executes a single command

// Shell lifecycle functions
void shell_init (void);
void shell_loop (void);
void shell_cleanup (void);

#endif
