#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>

#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a "\n", __FUNCTION__, __FILE__, __LINE__, ##args);
 
typedef struct {
	union { int width,  w; };
	union { int height, h; };
	union { uint32_t *pixels, *p; };
} sprite_t;

sprite_t frame;

bool LoadSprite(sprite_t *sprite, const char *filename);

/* Bitmap file format
 *
 * SECTION
 * Address:Bytes	Name
 *
 * HEADER:
 *	  0:	2		"BM" magic number
 *	  2:	4		file size
 *	  6:	4		junk
 *	 10:	4		Starting address of image data
 * BITMAP HEADER:
 *	 14:	4		header size
 *	 18:	4		width  (signed)
 *	 22:	4		height (signed)
 *	 26:	2		Number of color planes
 *	 28:	2		Bits per pixel
 *	[...]
 * [OPTIONAL COLOR PALETTE, NOT PRESENT IN 32 BIT BITMAPS]
 * BITMAP DATA:
 *	DATA:	X	Pixels
 */
bool LoadSprite(sprite_t *sprite, const char *filename) {
	bool return_value = true;

	uint32_t image_data_address;
	int32_t width;
	int32_t height;
	uint32_t pixel_count;
	uint16_t bit_depth;
	uint8_t byte_depth;
	uint32_t *pixels;

	printf("Loading bitmap file: %s\n", filename);

	FILE *file;
	file = fopen(filename, "rb");
	if(file) {
		if(fgetc(file) == 'B' && fgetc(file) == 'M') {
			printf("BM read; bitmap file confirmed.\n");
			fseek(file, 8, SEEK_CUR);
			fread(&image_data_address, 4, 1, file);
			fseek(file, 4, SEEK_CUR);
			fread(&width, 4, 1, file);
			fread(&height, 4, 1, file);
			fseek(file, 2, SEEK_CUR);
			fread(&bit_depth, 2, 1, file);
			if(bit_depth != 32) {
				PRINT_ERROR("(%s) Bit depth expected %d is %d", filename, 32, bit_depth);
				return_value = false;
			}
			else { // Image metadata correct
				printf("image data address:\t%d\nwidth:\t\t\t%d pix\nheight:\t\t\t%d pix\nbit depth:\t\t%d bpp\n", image_data_address, width, height, bit_depth);
				pixel_count = width * height;
				byte_depth = bit_depth / 8;
				pixels = malloc(pixel_count * byte_depth);
				if(pixels) {
					fseek(file, image_data_address, SEEK_SET);
					int pixels_read = fread(pixels, byte_depth, pixel_count, file);
					printf("Read %d pixels\n", pixels_read);
					if(pixels_read == pixel_count) {
						sprite->w = width;
						sprite->h = height;
						sprite->p = pixels;
					}
					else {
						PRINT_ERROR("(%s) Read pixel count incorrect. Is %d expected %d", filename, pixels_read, pixel_count);
						free(pixels);
						return_value = false;
					}
				}
				else {
					printf("(%s) Failed to allocate %d pixels.\n", filename, pixel_count);
					return_value = false;
				}
			} // Done loading sprite
		}
		else {
			PRINT_ERROR("(%s) First two bytes of file are not \"BM\"", filename);
			return_value = false;
		}

		fclose(file);
	}
	else {
		PRINT_ERROR("(%s) Failed to open file", filename);
		return_value = false;
	}
	return return_value;
}




int main( int argc, char *argv[] )
{
  int gif_file;

//   if ( argc < 2 )
//   {
//     fprintf( stderr, "Usage: %s <path-to-gif-file>\n", argv[ 0 ] );
//     exit( 0 );
//   }

  //gif_file = open( argv[ 1 ], O_RDONLY );
//   gif_file = open( "test.gif", O_RDONLY );

//   if ( gif_file == -1 )
//   {
//     fprintf( stderr, "Unable to open file '%s'", argv[ 1 ] );
//     perror( ": " );
//   }

    sprite_t sprite;
	if(!LoadSprite(&sprite, "test.bmp")) {
		PRINT_ERROR("Failed to load sprite: \"colorwheel.bmp\"");
		return -1;
	}

	static uint32_t *p;
		p = frame.pixels;
		for(int n = 0; n < frame.w * frame.h; ++n) {
			*(p++) = (float)n / (float)(frame.w * frame.h) * (uint32_t)(-1);
		}


static uint32_t *frame_pointer, *sprite_pointer;
		static int sprite_byte_width;
		frame_pointer = frame.pixels;
		sprite_pointer = sprite.pixels;
		sprite_byte_width = sprite.w * 4;
		for(int y = 0; y < sprite.h; ++y) {
			memcpy(frame_pointer, sprite_pointer, sprite_byte_width);
			frame_pointer += frame.w; sprite_pointer += sprite.w;
		}



  //process_gif_stream( gif_file );

//  close( gif_file );
}
