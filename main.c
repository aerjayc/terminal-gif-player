#include <ncurses.h>
#include <stdlib.h>

/* TODO:
 *	- error handling
 *	- create image data structure
 *	- use half block to double vertical resolution
 *	- better palette handling
 *	- handle resizing
 */

int print_image(uint8_t ***image, short height, short width);

int main()
{
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

	int i, j;

	// create 3d-array
	//uint8_t image[10][10][3];
	uint8_t ***image = calloc(10, sizeof(uint8_t **));
	if(image == NULL) {
		endwin();
		printf("Error: calloc() returned NULL. Exiting...\n");
		exit(1);
	}

	for(i = 0; i < 10; i++) {
		image[i] = calloc(10, sizeof(uint8_t *));
		for(j = 0; j < 10; j++) {
			image[i][j] = calloc(3, sizeof(uint8_t));
		}
	}

	// generate image
	for(i = 0; i < 10; i++) {
		for(j = 0; j < 10; j++) {
			image[i][j][0] = 159;
			image[i][j][1] = 222;
			image[i][j][2] = 42;
		}
	}

	print_image(image, 10, 10);
	refresh();
	getch();
	endwin();

	//destroy image
	for(i = 0; i < 10; i++) {
		for(j = 0; j < 10; j++)
			free(image[i][j]);
		free(image[i]);
	}
	free(image);

	return 0;
}


int print_image(uint8_t ***image, short height, short width) {
	/* Iterate over `image`,
	 *	image = 2d array (of size LINESxCOLS) of uint8_t triples
	 * Create color palette as you go through the pixels
	 * Go through two rows at a time using half block elements
	 */

	unsigned int i, j, rgb_value, pair_index;
	for(i = 0, pair_index = 1; i < height; i++) {
		move(i, 0);	// go back to 1st column
		for(j = 0; j < width; j++, pair_index++) {
			// generate rgb value from 3 bytes
			rgb_value  = image[i][j][0];
			rgb_value |= image[i][j][1] << 8;
			rgb_value |= image[i][j][2] << 16;
			//rgb_value  = image[0][0][0];
			//rgb_value = 0x0f5efa;

			// naive palette-ing
			init_extended_pair(pair_index, rgb_value, rgb_value);
			color_set(pair_index, NULL);
			printw("a", i, j);
			//addch(' ');
		}
	}

	return 0;
}
