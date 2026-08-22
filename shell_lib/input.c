/*
 *  input.c -- handles the input from the keyboard.
 */

#include "types.h"
#include "user.h"

#include "bash/shell.h"

int current_cursor_pos = 0;

char current_input_buffer[256];
int current_input_length = 0;

// Helper function: simplified comment.
void 
update_buffer_state (char *buffer, int pos, int cursor) 
{
    strncpy (current_input_buffer, buffer, pos);
    current_input_buffer[pos] = '\0';
    current_input_length = pos;
    current_cursor_pos = cursor;
}

/*
 *  read_input -- read input from the keyboard.
 */
char *
read_input () 
{
    char buf[100];

    char * buffer = malloc (256);
 
    if (!buffer) {
        //perror("malloc failed");
        //exit(EXIT_FAILURE);
        return nullptr;
    }

    memset (buffer, 0, 256);

    int cursor = 0;
    int pos = 0;
    int c;


    while (1) {

        c = getchar ();

        if (c == -1) {
            // if (errno == EINTR) {
            //     continue;
            // }
            break;
        }

        if (c == '\n') {
            //putchar (0, '\n');
            //putchar ('\n');
            update_buffer_state (buffer, pos, cursor);
            break;
        } else if (c == '\t') {
            
            // hit enter and execute a command



        } else {
            buffer [pos++] = c;
            buffer [pos] = '\0';
            cursor ++;
        }

        update_buffer_state (buffer, pos, cursor);

    }

    buffer[pos] = '\0';
    
    // if (strlen(buffer) > 0) {
    //     history_add(buffer);
    // }
    
//    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return buffer;

}
