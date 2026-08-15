#ifndef SHELL_H
#define SHELL_H

#include "constants.h"
//#include "rc.h"
#include "history.h"
//#include "alias.h"
#include "command.h"

/*
 * Reads a line of input from the user.
 */
char *read_input( void);

/*
 * Parses input string into command structure.
 */
command_t *parse_input (char *input);

// Shell lifecycle functions
void shell_init (void);
void shell_loop (void);
void shell_cleanup (void);


#endif
