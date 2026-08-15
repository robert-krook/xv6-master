// System call numbers
#define SYS_fork                    1   // Fork call split process
#define SYS_exit                    2   
#define SYS_wait                    3
#define SYS_pipe                    4
#define SYS_read                    5
#define SYS_kill                    6
#define SYS_exec                    7
#define SYS_fstat                   8
#define SYS_chdir                   9
#define SYS_dup                     10
#define SYS_getpid                  11
#define SYS_sbrk                    12
#define SYS_sleep                   13
#define SYS_uptime                  14
#define SYS_open                    15
#define SYS_write                   16
#define SYS_mknod                   17
#define SYS_unlink                  18
#define SYS_link                    19
#define SYS_mkdir                   20  // Create a directory
#define SYS_close                   21  // Close a file      

// Added system calls
#define SYS_ps                      22  // Get process status
#define SYS_getcwd                  23  // Get current working directory

#define SYS_init_graphics           24  // Start the VGA graphics card mode
#define SYS_draw_pixel              25  // Draw a pixel at the screen
#define SYS_blit                    26  // Refresh the screen
#define SYS_clear_screen            27  // Clear the screen
#define SYS_exit_graphics           28  // Exit to text mode
#define SYS_draw_line               29  // Draw a line at the screen 
#define SYS_getkey                  30  // Fetch a key stroke

#define SYS_GUI_createWindow        31  // Create window in graphics mode.
#define SYS_GUI_closeWindow         32  // Close window in graphics mode.
#define SYS_GUI_getMessage          33
#define SYS_GUI_updateScreen        34
#define SYS_GUI_maximizeWindow      35
#define SYS_GUI_minimizeWindow      36
#define SYS_GUI_turnoffScreen       37
#define SYS_GUI_createPopupWindow   38
#define SYS_GUI_closePopupWindow    39
#define SYS_GUI_getPopupMessage     40

#define SYS_date                    41

#define SYS_df                      42
#define SYS_GUI_select_style        43  // Select the GUI style (ex. Amiga, Alpha, etc.).
#define SYS_GUI_get_style           44  // Get the selected style (ex. Amiga, Alpha, etc.).

#define SYS_chpr                    45

#define SYS_change_console_colors   46
