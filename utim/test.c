#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util_image.h"

int main(int argc, const char *argv[])
{
	int i, w, h, result;
	utim_font_t  *font;
	utim_image_t *img, *txt, *tmp;
	utim_color_t  color;
	utim_point_t  pa, pb;

	img = utim_create(640, 480, 4, 255);
	if (!img) {
		fprintf(stderr, "%s\n");
		exit(-1);
	}

	srand(clock());

	for (i = 0; i < 50; ++i) {
		pa[0]= 0;
		pa[1]= 0;
		pb[0]= rand() % img->xsize;
		pb[1]= rand() % img->ysize;
		color[0] = rand() % 256;
		color[1] = rand() % 256;
		color[2] = rand() % 256;
		color[3] = rand() %  75;  /* Alpha */
		utim_draw_line(img,
			pa, pb, color, rand()%5);
	}

	for (i = 0; i < 100; ++i) {
		w = rand() % 200;
		h = rand() % 200;
		pa[0]= rand() % img->xsize;
		pa[1]= rand() % img->ysize;
		color[0] = rand() % 256;
		color[1] = rand() % 256;
		color[2] = rand() % 256;
		color[3] = rand() % 127; /* Alpha */
		utim_draw_rect(img,
			pa, w, h, color, rand()%5);
	}

	for (i = 0; i < 100; ++i) {
		pa[0]= rand() % img->xsize;
		pa[1]= rand() % img->ysize;
		color[0] = rand() % 256;
		color[1] = rand() % 256;
		color[2] = rand() % 256;
		color[3] = rand() % 127; /* Alpha */
		utim_draw_filled_circle(img,
			pa, 5 + rand()%20, color);
	}

	font = utim_load_font("latin1.psf");
	if (!font) {
		utim_free_image(img);
		exit(-1);
	}
	utim_set_color(color, 0, 0, 0, 255);
	txt = utim_text(font, "UTIM: HELLO WORLD", color);
	if (!txt) {
		utim_free_image(img);
		utim_free_font(font);
	}
	tmp = utim_resize(txt, 400, 32, UTIM_RESIZE_NEAREST);
	if (!tmp) {
		utim_free_image(img);
		utim_free_image(txt);
		utim_free_font(font);
	}
	utim_free_image(txt);
	txt = tmp;
	utim_set_point(pa, 120, 240);
	utim_superpose(img, txt, pa);

	utim_negative_color(img);

	result = utim_write("out.bmp", img);
	printf("Save BMP: %d\n", result);
	result = utim_write("out.jpg", img);
	printf("Save JPG: %d\n", result);
	result = utim_write("out.png", img);
	printf("Save PNG: %d\n", result);
	result = utim_write("out.tga", img);
	printf("Save TGA: %d\n", result);

	utim_free_image(img);
	utim_free_image(txt);
	utim_free_font(font);
	return 0;
}