#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

/* TODO:
 *	- resize image before showing: Done
 *	- allow scrolling: skip (difficult)
 *	- show image on command line (i.e. don't clear terminal): skip
 *	- use half block to double vertical resolution
 *	- better palette handling
 *		- if rgb_value already in palette, use index
 *	- error handling
 *	- more efficient image importing?
 *	- ~~create image data structure~~
 */

int sleep_ms(long ms) {
	struct timespec ts;
	int res;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	do{
		res = nanosleep(&ts, &ts);
	} while(res);

	return res;
}

int print_image(uint8_t *image, int width, int height);


int main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("Usage:\n\t./a.out delay_ms filename [filename1..]\n");
		return 0;
	}

	// ncurses initialization
	setlocale(LC_ALL, "");		// allows weird characters
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

	int orig_width, orig_height, width, height, channels;
	uint8_t **images = calloc(argc - 1, sizeof(uint8_t *));
	uint8_t **resized_images = calloc(argc - 1, sizeof(uint8_t *));
	int i;
	for(i = 0; i < (argc-2); i++) {
		images[i] = stbi_load(argv[i+2], &orig_width, &orig_height, &channels, 3);

		// determine new size
		if(i == 0) {
			width = COLS;
			height = 2*LINES;
			if((orig_width * height) >= (width * orig_height))
				height = (width * orig_height) / orig_width;
			else
				width = (height * orig_width) / orig_height;
		}

		resized_images[i] = calloc(width*height*channels, sizeof(uint8_t));
		stbir_resize_uint8(images[i], orig_width, orig_height, 0,
				   resized_images[i], width, height, 0, channels);
	}

	i = 0;
	for(i = 0; i < (argc-2); i++) {
		print_image(resized_images[i], width, height);
		refresh();
		sleep_ms(atoi(argv[1]));
	}
	//getch();

	/*
	// determine new size
	int width = COLS,
	    height = 2*LINES;
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
	*/
	endwin();
	printf("LINES = %i\tCOLS = %i\n", LINES, COLS);
	printf("COLOR_PAIRS = %i\n", COLOR_PAIRS);
	printf("width = %i, height = %i\n", width, height);

	for(i = 0; i < (argc-2); i++) {
		stbi_image_free(images[i]);
		stbi_image_free(resized_images[i]);
	}
	free(images);
	free(resized_images);

	return 0;
}


int print_image(uint8_t *image, int width, int height) {
	/* Iterate over `image`,
	 *	image = array (of size LINESxCOLS) of uint8_t triples
	 * Create color palette as you go through the pixels
	 * Go through two rows at a time using half block elements
	 */

	wchar_t chaxel[] = L"\x2580";	// half-block element

	int i, j, top_pixel_index, bottom_pixel_index, pair_index;
	unsigned int top_rgb_value, bottom_rgb_value;
	for(i = 0, pair_index = 1; (i < height) && (i < 2*LINES); i += 2) {
		move(i / 2, 0);
		for(j = 0; (j < width) && (j < COLS); j++) {
			top_pixel_index = (i*width) + j;
			bottom_pixel_index = ((i+1)*width) + j;

			// generate rgb value from 3 bytes
			top_rgb_value  = image[3*top_pixel_index] << 16;
			top_rgb_value |= image[3*top_pixel_index + 1] << 8;
			top_rgb_value |= image[3*top_pixel_index + 2];

			if(i+1 < height) {
				bottom_rgb_value  = image[3*bottom_pixel_index] << 16;
				bottom_rgb_value |= image[3*bottom_pixel_index + 1] << 8;
				bottom_rgb_value |= image[3*bottom_pixel_index + 2];
			} else
				bottom_rgb_value = 0;

			// naive palette-ing
			init_extended_pair(pair_index, top_rgb_value, bottom_rgb_value);
			color_set(0, &pair_index);
			pair_index++;

			addwstr(chaxel);
		}
	}

	return pair_index;
}
