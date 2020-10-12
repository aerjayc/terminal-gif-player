#include <ncurses.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

/* TODO:
 *	- resize image before showing
 *	- allow scrolling
 *	- show image on command line (i.e. don't clear terminal)
 *	- use half block to double vertical resolution
 *	- better palette handling
 *		- if rgb_value already in palette, use index
 *	- error handling
 *	- more efficient image importing?
 *	- ~~create image data structure~~
 */

int print_image(uint8_t *image, int width, int height);


int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("Usage:\n\t./a.out filename\n");
		return 0;
	}

	// ncurses initialization
	WINDOW *scr = initscr();
	if(!has_colors()) {
		endwin();
		printf("Terminal does not support color.\n");
		exit(1);
	}
	cbreak();	// raw mode
	noecho();	// don't echo keypresses
	clear();	// clean screen, cursor to (0,0)
	start_color();

	int orig_width, orig_height, channels;
	uint8_t *image = stbi_load(argv[1], &orig_width, &orig_height,
				   &channels, 3);

	// determine new size
	// change this to width = COLS, height follows, and allow scrolling
	int width = COLS,
	    height = LINES;
	if((orig_width * height) >= (width * orig_height))
		height = (width * orig_height) / orig_width;
	else
		width = (height * orig_width) / orig_height;

	uint8_t *resized_image = calloc(width*height*channels, sizeof(uint8_t));
	stbir_resize_uint8(image, orig_width, orig_height, 0,
			   resized_image, width, height, 0, channels);
	print_image(resized_image, width, height);	//, channels);

	refresh();
	getch();
	endwin();
	printf("LINES = %i\tCOLS = %i\n", LINES, COLS);
	printf("COLOR_PAIRS = %i\n", COLOR_PAIRS);
	printf("width = %i, height = %i\n", width, height);

	stbi_image_free(image);
	stbi_image_free(resized_image);

	return 0;
}

int print_image(uint8_t *image, int width, int height) {
	/* Iterate over `image`,
	 *	image = 2d array (of size LINESxCOLS) of uint8_t triples
	 * Create color palette as you go through the pixels
	 * Go through two rows at a time using half block elements
	 */

	unsigned int i, j, rgb_value, pixel_index, pair_index;
	for(i = 0, pair_index = 1; (i < LINES) && (i < height); i++) {
		move(i, 0);	// go back to 1st column
		for(j = 0; (j < COLS) && (j < width); j++, pair_index++) {
			pixel_index = (i*width) + j;

			// generate rgb value from 3 bytes
			rgb_value  = image[3*pixel_index];
			rgb_value |= image[3*pixel_index + 1] << 8;
			rgb_value |= image[3*pixel_index + 2] << 16;

			// naive palette-ing
			// +1 is so that pair index `0` is not used
			// (as it is reserved)
			//init_extended_pair(pixel_index+1, rgb_value, rgb_value);
			//color_set(pixel_index+1, NULL);
			init_extended_pair(pair_index, rgb_value, rgb_value);
			color_set(0, &pair_index);
			printw("a", i, j);
		}
	}

	return 0;
}
