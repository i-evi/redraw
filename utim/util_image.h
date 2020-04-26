#ifndef _UTIL_IMAGE_H_
#define _UTIL_IMAGE_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define UTIM_INT32 int

typedef UTIM_INT32 utim_int32_t;
typedef unsigned UTIM_INT32 utim_uint32_t;

#ifndef byte
	#define byte unsigned char
#endif

/*
 * Defined as little-endian
 */
#define UTIM_BMP 0x00706d62 /* b m p \0 */
#define UTIM_PNG 0x00676e70 /* p n g \0 */
#define UTIM_JPG 0x0067706a /* j p g \0 */
#define UTIM_TGA 0x00616774 /* t g a \0 */

#define UTIM_MAX_CHANNELS 4

typedef struct {
	byte *pixels;
	int xsize;
	int ysize;
	int channels;
} utim_image_t; /* For 256 color-depth images */

typedef struct {
	int x;
	int y;
} utim_point_t;

#define UTIM_COLOR_R 0
#define UTIM_COLOR_G 1
#define UTIM_COLOR_B 2
#define UTIM_COLOR_A 3

#define UTIM_BACK_COLOR 255

#define UTIM_RESIZE_NEAREST 0
#define UTIM_RESIZE_LINEAR  1

typedef struct {
	byte rgba[4];
} utim_color_t;

utim_image_t *image_read(const char *filename);

int image_write(const char *filename, utim_image_t *img);
int image_write_ctrl(const char *filename,
	utim_image_t *img, int comp, int quality);

utim_image_t *image_clone(utim_image_t *img);
utim_image_t *image_create(int x, int y, int nch, int c);

void free_image(utim_image_t *img);

utim_image_t *image_resize(utim_image_t *img, int x, int y, int mode);

/* Swap 2 channels */
utim_image_t *image_swap_chl(utim_image_t *img, int a, int b);

utim_image_t *image_2rgb(utim_image_t *img);
utim_image_t *image_2gray(utim_image_t *img);
utim_image_t *image_2rgba(utim_image_t *img);

/* gray2rgb will copy gray channel 2 times */
utim_image_t *image_gray2rgb(utim_image_t *gray);
utim_image_t *image_gray2rgba(utim_image_t *gray);
utim_image_t *image_rgb2bgr(utim_image_t *rgb);
utim_image_t *image_rgb2gray(utim_image_t *rgb);
utim_image_t *image_rgb2rgba(utim_image_t *rgb);
utim_image_t *image_rgba2rgb(utim_image_t *rgba);
utim_image_t *image_rgba2gray(utim_image_t *rgba);

utim_image_t *image_rgb_from_rgba(utim_image_t *rgba);
utim_image_t *image_bgr_from_rgb(utim_image_t *rgb);
utim_image_t *image_gray_from_rgb(utim_image_t *rgb);
utim_image_t *image_rgb_from_gray(utim_image_t *gray);

utim_image_t *image_stack(utim_image_t **chx, int nch);
utim_image_t *image_pick_chl(utim_image_t *img, int ich);

utim_image_t *image_set_color(utim_image_t *img, int ich, int color);

/*
 * Support Gray, RGB, RGBA images. 
 * If not RGBA image, will convert to RGBA image
 */
utim_image_t *image_set_opacity(utim_image_t *img, int opacity);

utim_image_t *image_superpose(utim_image_t *bg,
	utim_image_t *img, utim_point_t *p);

/*
 * Simple Drawing functions
 */
int image_draw_point(utim_image_t *img,
	utim_point_t *p, utim_color_t *c);

void image_draw_line(utim_image_t *img,
	utim_point_t *a, utim_point_t *b, utim_color_t *c, int width);

void image_draw_circle(utim_image_t *img,
	utim_point_t *center, int radius, utim_color_t *color, int width);

void image_draw_filled_circle(utim_image_t *img,
	utim_point_t *center, int radius, utim_color_t *color);

#undef byte

#ifdef __cplusplus
	}
#endif

#endif /* _UTIL_IMAGE_H_ */