#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>

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

int sleep_ms(long ms);
int print_image(uint8_t *image, int width, int height);
int loop_over_frames(char **fnames, int num_images, int width, int height, int delay);
char **list_files(const char *directory, const char *extension, int *num_names);

static int cmpstringp(const void *p1, const void *p2) {
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("Usage: ./a.out delay_ms DIRECTORY\n");
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

	int n;
	char **fnames = list_files(argv[2], ".jpg", &n);
	qsort(fnames, n, sizeof(fnames[0]), cmpstringp);
	loop_over_frames(fnames, n, -1, -1, atoi(argv[1]));

	endwin();
	printf("LINES = %i\tCOLS = %i\n", LINES, COLS);
	printf("COLOR_PAIRS = %i\n", COLOR_PAIRS);
	//printf("width = %i, height = %i\n", width, height);

	int i;
	for(i = 0; i < n; i++) {
		free(fnames[i]);
	}
	free(fnames);

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

int loop_over_frames(char **fnames, int num_images, int width, int height, int delay) {
	if(width <= 0)
		width = COLS;
	if(height <= 0)
		height = 2*LINES;

	uint8_t *image, *resized_image;
	int orig_width, orig_height, channels;
	int i;
	for(i = 0; i < num_images; i++) {
		image = stbi_load(fnames[i], &orig_width, &orig_height, &channels, 3);

		// determine new size
		if(i == 0) {
			if((orig_width * height) >= (width * orig_height))
				height = (width * orig_height) / orig_width;
			else
				width = (height * orig_width) / orig_height;
		}

		resized_image = calloc(width*height*channels, sizeof(uint8_t));
		stbir_resize_uint8(image, orig_width, orig_height, 0,
				   resized_image, width, height, 0, channels);

		// free image immediately
		stbi_image_free(image);

		clear();
		print_image(resized_image, width, height);
		refresh();

		//free resized_image immediately
		stbi_image_free(resized_image);

		sleep_ms(delay);
	}

	return i + 1;
}

// based on https://stackoverflow.com/a/4204758 and https://stackoverflow.com/a/17683417
char **list_files(const char *directory, const char *extension, int *num_names) {
	char **filenames = calloc(1, sizeof(char *));
	DIR *d;
	struct dirent *dir;
	char *filename;
	char *file_ext;
	int i = 0;
	d = opendir(directory);
	if(d) {
		for(i = 0; (dir = readdir(d)) != NULL;) {
			if(dir->d_type == DT_REG) {	// if regular file
				filename = dir->d_name;

				// check extension
				if(extension) {
					file_ext = strrchr(filename, extension[0]);
					if((file_ext == NULL) || strcmp(file_ext, extension))
						continue;
				}

				if(i > 0)
					filenames = reallocarray(filenames, i+1, sizeof(char *));
				filenames[i] = calloc(strlen(filename) + 1, sizeof(char));
				strcpy(filenames[i], filename);
				i++;
			}
		}
		closedir(d);
	}
	*num_names = i;

	return filenames;
}

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
