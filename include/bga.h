
/*
 *  bga.h
 */

unsigned short bga_read_register (unsigned short IndexValue);
int bga_is_available (void);
void bga_set_video_mode (unsigned int Width, unsigned int Height, unsigned int BitDepth, 
                         int UseLinearFrameBuffer, int ClearVideoMemory);
void bga_set_bank (unsigned short BankNumber);

// External calls
void bga_init ();
void bga_color_demo ();
void bga_blitter ();
void bga_clear_screen ();
int  bga_draw_pixel (int x, int y, int color);
int  bga_draw_line (int x1, int y1, int x2, int y2, int color);
int  bga_draw_character (int x, int y, char ch, int color);
void bga_draw_string (int x, int y, char *str, int color);
void bga_draw_string_with_max_width (int x, int y, int width, char *str, int color);
int  bga_draw_icon (int x, int y, int icon, int color);
void bga_draw_mouse(int mode, int x, int y);
void bga_draw_rect_bound (int x, int y, int width, int height, int fill, int max_x, int max_y);
void bga_draw_rect(int x, int y, int width, int height, int fill);
void bga_draw_rect_by_coord(int xmin, int ymin, int xmax, int ymax, int fill);
void bga_draw_24_image_part (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, int subh, char *title);
//void bga_draw_24_image_part (unsigned char *img, int x, int y, int width, int height, int subx, int suby, int subw, int subh, char *title);
void bga_draw_rect_border (int color, int x, int y, int width, int height);
void bga_graphicsintr(int (*getc) (void)) ;
int  bga_blit (void);

void bga_init_24 ();
void bga_clear_screen_24 ();
int  bga_blit_24 ();
int  bga_draw_pixel_24 (int x, int y, RGBA color);
int  bga_draw_line_24 (int x1, int y1, int x2, int y2, RGBA color);
int  bga_draw_character_24 (int x, int y, char ch, RGBA color);
void bga_draw_string_24 (int x, int y, char *str, RGBA color);
void bga_draw_string_with_max_width_24 (int x, int y, int width, char *str, RGBA color);
int  bga_draw_icon_24 (int x, int y, int icon, RGBA color);
void bga_draw_mouse_24(int mode, int x, int y);
void bga_draw_rect_bound_24 (int x, int y, int width, int height, RGBA fill, int max_x, int max_y);
void bga_draw_rect_24(int x, int y, int width, int height, RGBA fill);
void bga_draw_rect_by_coord_24(int xmin, int ymin, int xmax, int ymax, RGBA fill);
void bga_draw_24_image_part_24 (RGB *img, int x, int y, int width, int height, int subx, int suby, int subw, 
    int subh, char *title);
void bga_draw_rect_border_24 (RGBA color, int x, int y, int width, int height);
