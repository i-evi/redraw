#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util_image.h"

#ifndef byte
	#define byte unsigned char
#endif

#define ASSERT_ON_BUILD(condition)\
	((void)sizeof(char[1 - 2*!!(condition)]))
/*=========== compile-time test ===========*/
void ____utim_datatype_test____()
{
	/*
	 * Ref: image_write(), image_write_ctrl()
	 * At simple bmp operations...
	 */
	ASSERT_ON_BUILD(sizeof(utim_int32_t) - 4);
}
#ifdef RUN_TIME_ENDIAN_CHECK
	static int _is_little_endian()
	{
		unsigned int i = 1;
		unsigned char *c = (unsigned char*)&i;
		if (*c)
			return 1;
		return 0;
	}

	static void _memrev(void *i, unsigned int s)
	{
		unsigned int j = s >> 1;
		unsigned char *c = (unsigned char*)i;
		for (s--; j > 0; j--) {
			*c ^= *(c + s);
			*(c + s) ^= *c;
			*c ^= *(c + s);
			c++, s -= 2;
		}
	}
#endif

#ifndef CONFIG_STD_C89
	inline
#endif
static int _index_calc(int xsize, int ysize, int x, int y)
{
	int size, index;
	if (x >= xsize || y >= ysize || x < 0 || y < 0)
		return -1;
	size = xsize * ysize;
	index = xsize * y + x;
	return index;
}

#ifndef CONFIG_STD_C89
	inline
#endif
static int _coordinate_calc(int index, int xsize, int ysize, int *x,int *y)
{
	int size = xsize * ysize;
	if(index > size)
		return -1;
	*x = index % xsize;
	*y = index / xsize;
	return 0;
}

static const char *file_extension(const char *filename)
{
	const char *p = filename + strlen(filename);
	while (filename != p--)
		if (*p == '.')
			break;
	return filename < p ? p + 1 : NULL;
}

static byte *_24bit_bmp_read(const char *filename, int *xsize, int *ysize)
{
	byte header[54];
	utim_uint32_t b, *x;
	utim_uint32_t r, windows_byte_cnt = 0;
	byte *image_tail, *pixels;
	FILE *fp;
	if (!(fp = fopen(filename, "rb"))) 
		return NULL;
	fread(header, sizeof(byte), 54, fp);
	x = (utim_uint32_t*)(header + 18);
	*xsize = x[0];
	*ysize = x[1];
#if   defined(RUN_TIME_ENDIAN_CHECK)
	if (!_is_little_endian) {
		_memrev(xsize, sizeof(utim_uint32_t));
		_memrev(ysize, sizeof(utim_uint32_t));
	}
#elif defined(BYTE_ORDER_BIG_ENDIAN)
	_memrev(xsize, sizeof(utim_uint32_t));
	_memrev(ysize, sizeof(utim_uint32_t));
#endif
	b = *(utim_uint32_t*)(header + 10);
	fseek(fp, b, SEEK_SET);
	if (!((*xsize * 3 ) % 4)) {
		windows_byte_cnt = 0;
	} else {
		int t = (*xsize * 3 ) / 4;
		windows_byte_cnt = ((t + 1) * 4) - *xsize * 3;
	}
	pixels = (byte*)malloc(*xsize * *ysize * 3);
	if (!pixels) {
		fclose(fp);
		return NULL;
	}
	image_tail = pixels + (*ysize * *xsize * 3);
	/* fread(pixels, sizeof(byte), *xsize * *ysize * 3, fp); */
	for (r = 0; r < *ysize; ++r) {
		image_tail -= (*xsize * 3);
		fread(image_tail , sizeof(byte), *xsize * 3, fp);
		fseek(fp, windows_byte_cnt, SEEK_CUR);
	}
	fclose(fp);
	return pixels;
}

static int _24bit_bmp_write(const char *filename,
	byte *pixels, int xsize, int ysize)
{
	byte header[54] = {
		0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
		54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0
	};
	utim_uint32_t windows_byte_cnt = 0;
	utim_uint32_t windows_byte_set = 0;
	utim_uint32_t r = (xsize * 3 ) % 4;
	utim_uint32_t *x;
	byte *image_tail = NULL;
	FILE *fp;
	if (!r) {
		windows_byte_cnt = 0;
	} else {
		int t = (xsize * 3 ) / 4;
		windows_byte_cnt = ((t + 1) * 4) - xsize * 3;
	}
	x = (utim_uint32_t*)(header + 2);
	x[0] = (xsize * 3 + windows_byte_cnt) * ysize + 54;
	x = (utim_uint32_t*)(header + 18);
	x[0] = xsize;
	x[1] = ysize;
#if   defined(RUN_TIME_ENDIAN_CHECK)
	if (!_is_little_endian) {
		_memrev(x + 0, sizeof(utim_uint32_t));
		_memrev(x + 1, sizeof(utim_uint32_t));
	}
#elif defined(BYTE_ORDER_BIG_ENDIAN)
	_memrev(x + 0, sizeof(utim_uint32_t));
	_memrev(x + 1, sizeof(utim_uint32_t));
#endif
	if (!(fp = fopen(filename, "wb+")))
		return -1;
	fwrite(header, sizeof(byte), 54, fp);
	/* fwrite(pixels, sizeof(byte) * 3, xsize * ysize, fp); */
	image_tail = pixels + (ysize * xsize * 3);
	for (r = 0; r < ysize; ++r)
	{
		image_tail -= (xsize * 3);
		fwrite(image_tail, sizeof(byte) * 3, xsize, fp);
		fwrite(&windows_byte_set, 1, windows_byte_cnt, fp);
	}
	fclose(fp);
	return 0;
}

/*
 * Use stb image. stb_image.h, stb_image_write.h are included. 
 */
#ifdef USE3RD_STB_IMAGE
	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
	#define STB_IMAGE_WRITE_IMPLEMENTATION
	#include "stb_image_write.h"
	#define STB_IMAGE_RESIZE_IMPLEMENTATION
	#include "stb_image_resize.h"
	int jpg_default_quality = 70;
	int png_default_compress = 8;
#endif

utim_image_t *image_read(const char*filename)
{
	const char *t;
	utim_image_t *img = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!img)
		return NULL;
#ifdef USE3RD_STB_IMAGE
	img->pixels = stbi_load(filename, &img->xsize, &img->ysize,
		&img->channels, 0);
	if (!img->pixels) {
		free(img);
		return NULL;
	}
#else /* Support 24bit bmp image only */
	t = file_extension(filename);
	if (strcmp(t, "bmp"))
		return NULL;
	img->pixels = _24bit_bmp_read(filename, &img->xsize, &img->ysize);
	if (!img->pixels) {
		free(img);
		return NULL;
	}
	img->channels = 3;
#endif
	return img;
}

int image_write(const char *filename, utim_image_t *img)
{
	return image_write_ctrl(filename, img, 0, 0);
}

int image_write_ctrl(const char *filename,
	utim_image_t *img, int comp, int quality)
{
	const char *t;
	char tbuf[8];
	char *p = tbuf;
	int state;
	t = file_extension(filename);
	strcpy(tbuf, t);
	while ((*p = tolower(*p))) ++p;
	if (strlen(t) < 3)
		return -1; /* Not supported file format */
#if   defined(RUN_TIME_ENDIAN_CHECK)
	if (!_is_little_endian())
		_memrev(tbuf, strlen(tbuf) + 1);
#elif defined(BYTE_ORDER_BIG_ENDIAN)
	_memrev(tbuf, strlen(tbuf) + 1);
#endif
#ifdef USE3RD_STB_IMAGE
	switch (*(utim_int32_t*)tbuf) {
		case UTIM_BMP:
			if (!comp)
				comp = img->channels;
			state = stbi_write_bmp(filename, img->xsize,
				img->ysize, comp, img->pixels);
			if (state != 1)
				return state;
			return 0;
		case UTIM_PNG:
			if (!comp)
				comp = img->channels;
			quality = quality ? quality : png_default_compress;
			stbi_write_png_compression_level = quality;
			state = stbi_write_png(filename, img->xsize,
				img->ysize, img->channels, img->pixels,
				img->xsize * img->channels);
			if (state != 1)
				return state;
			return 0;
		case UTIM_JPG:
			if (!comp)
				comp = img->channels;
			quality = quality ? quality : jpg_default_quality;
			state = stbi_write_jpg(filename, img->xsize,
				img->ysize, comp, img->pixels, quality);
			if (state != 1)
				return state;
			return 0;
		case UTIM_TGA:
			if (!comp)
				comp = img->channels;
			state = stbi_write_tga(filename, img->xsize,
				img->ysize, comp, img->pixels);
			if (state != 1)
				return state;
			return 0;
		default:
			return -1; /* Not supported file format */
	}
#else /* Support 24bit bmp image only */
	if (strcmp(tbuf, "bmp"))
		return -1;  /* Not supported file format */
	if (img->channels != 3)
		return -1; /* 24bit bmp only */
	return _24bit_bmp_write(filename,
		img->pixels, img->xsize, img->ysize);
#endif
}

void free_image(utim_image_t *img)
{
	if (!img)
		return;
	if (img->pixels)
		free(img->pixels);
	free(img);
}

utim_image_t *image_clone(utim_image_t *img)
{
	utim_image_t *clone;
	int size = img->xsize * img->ysize * img->channels;
	if (!size)
		return NULL;
	clone = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!clone)
		return NULL;
	memcpy(clone, img, sizeof(utim_image_t));
	clone->pixels = (byte*)malloc(size);
	if (!clone->pixels) {
		free(clone);
		return NULL;
	}
	memcpy(clone->pixels, img->pixels, size);
	return clone;
}

static byte *_image_resize_nearest(utim_image_t *img, int x, int y)
{
	byte *p;
	int i, j, ch_size = x * y;
	p = (byte*)malloc(ch_size * img->channels);
	if (!p)
		return NULL;
	for (j = 0; j < y; ++j) {
		for (i = 0; i < x; ++i) {
			memcpy(p + _index_calc(x, y, i, j) * img->channels,
				img->pixels + _index_calc
						(img->xsize, img->ysize,
				i * img->xsize / x, j * img->ysize / y) * 
				img->channels,img->channels);
		}
	}
	return p;
}

#ifdef USE3RD_STB_IMAGE
static byte *_image_resize_linear(utim_image_t *img, int x, int y)
{
	byte *p;
	int i, j, ch_size = x * y;
	p = (byte*)malloc(ch_size * img->channels);
	if (!p)
		return NULL;
	stbir_resize_uint8(img->pixels,
		img->xsize, img->ysize, 0, p, x, y, 0, img->channels);
	return p;
}
#endif

utim_image_t *image_resize(utim_image_t *img, int x, int y, int mode)
{
	utim_image_t *resize;
	resize = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!resize)
		return NULL;
	switch (mode) {
		case UTIM_RESIZE_NEAREST:
			resize->pixels = _image_resize_nearest(img, x, y);
			if (!resize->pixels) {
				free(resize);
				return NULL;
			}
			break;
#ifdef USE3RD_STB_IMAGE
		case UTIM_RESIZE_LINEAR:
			resize->pixels = _image_resize_linear(img, x, y);
			if (!resize->pixels) {
				free(resize);
				return NULL;
			}
			break;
#endif
		default:
			return NULL;
	}
	resize->xsize = x;
	resize->ysize = y;
	resize->channels = img->channels;
	return resize;
}

utim_image_t *image_swap_chl(utim_image_t *img, int a, int b)
{
	byte c;
	int i, ch_size;
	if (a == b || a >= img->channels || b >= img->channels)
		return NULL;
	ch_size = img->xsize * img->ysize;
	for (i = 0; i < ch_size; ++i) {
		c = *(img->pixels + img->channels * i + a);
		*(img->pixels + img->channels * i + a) =
			*(img->pixels + img->channels * i + b);
		*(img->pixels + img->channels * i + b) = c;
	}
	return img;
}

utim_image_t *image_2rgb(utim_image_t *img)
{
	switch (img->channels) {
		case 1:
			return image_gray2rgb(img);
		case 3:
			return img;
		case 4:
			return image_rgba2rgb(img);
		default:
			return NULL;
	}
}

utim_image_t *image_2gray(utim_image_t *img)
{
	switch (img->channels) {
		case 1:
			return img;
		case 3:
			return image_rgb2gray(img);
		case 4:
			return image_rgba2gray(img);
		default:
			return NULL;
	}
}

utim_image_t *image_2rgba(utim_image_t *img)
{
	switch (img->channels) {
		case 1:
			return image_gray2rgba(img);
		case 3:
			return image_rgb2rgba(img);
		case 4:
			return img;
		default:
			return NULL;
	}
}

utim_image_t *image_rgba2rgb(utim_image_t *rgba)
{
	byte *new_pixels, *p;
	int i, ch_size = rgba->xsize * rgba->ysize;
	if (!ch_size || rgba->channels != 4)
		return NULL;
	new_pixels = (byte*)malloc(ch_size * 3);
	if (!new_pixels)
		return NULL;
	p = new_pixels;
	for (i = 0; i < ch_size; ++i) {
		*p++ = *(rgba->pixels + i * 4);
		*p++ = *(rgba->pixels + i * 4 + 1);
		*p++ = *(rgba->pixels + i * 4 + 2);
	}
	free(rgba->pixels);
	rgba->pixels = new_pixels;
	rgba->channels = 3;
	return rgba;
}

utim_image_t *image_rgb_from_rgba(utim_image_t *rgba)
{
	utim_image_t *rgb;
	byte *p;
	int i, ch_size = rgba->xsize * rgba->ysize;
	if (!ch_size || rgba->channels != 4)
		return NULL;
	rgb = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!rgb)
		return NULL;
	rgb->pixels = (byte*)malloc(ch_size * 3);
	if (!rgb->pixels) {
		free(rgb);
		return NULL;
	}
	p = rgb->pixels;
	for (i = 0; i < ch_size; ++i) {
		*p++ = *(rgba->pixels + i * 4);
		*p++ = *(rgba->pixels + i * 4 + 1);
		*p++ = *(rgba->pixels + i * 4 + 2);
	}
	rgb->xsize = rgba->xsize;
	rgb->ysize = rgba->ysize;
	rgb->channels = 3;
	return rgb;
}

utim_image_t *image_rgb2bgr(utim_image_t *rgb)
{
	int n;
	byte *p1, *p2;
	if (rgb->channels != 3)
		return NULL;
	n = rgb->xsize * rgb->ysize;
	p1 = rgb->pixels;
	p2 = rgb->pixels + 2;
	while (n--) {
		*(p1) ^= *(p2);
		*(p2) ^= *(p1);
		*(p1) ^= *(p2);
		p1 += 3;
		p2 += 3;
	}
	return rgb;
}

utim_image_t *image_bgr_from_rgb(utim_image_t *rgb)
{
	utim_image_t *bgr = image_clone(rgb);
	if (!bgr)
		return NULL;
	image_rgb2bgr(bgr);
	return bgr;
}

utim_image_t *image_rgb2gray(utim_image_t *rgb)
{
	byte *new_pixels, *p;
	int i, sum, ch_size = rgb->xsize * rgb->ysize;
	if (rgb->channels != 3 || !ch_size)
		return NULL;
	new_pixels = (byte*)malloc(ch_size);
	if (!new_pixels)
		return NULL;
	p = new_pixels;
	for (i = 0; i < ch_size; ++i) {
		sum = 0;
		sum += *(rgb->pixels + i * 3);
		sum += *(rgb->pixels + i * 3 + 1);
		sum += *(rgb->pixels + i * 3 + 2);
		*p++ = sum / 3;
	}
	free(rgb->pixels);
	rgb->pixels = new_pixels;
	rgb->channels = 1;
	return rgb;
}

utim_image_t *image_rgba2gray(utim_image_t *rgba)
{
	byte *new_pixels, *p;
	int i, sum, ch_size = rgba->xsize * rgba->ysize;
	if (rgba->channels != 4 || !ch_size)
		return NULL;
	new_pixels = (byte*)malloc(ch_size);
	if (!new_pixels)
		return NULL;
	p = new_pixels;
	for (i = 0; i < ch_size; ++i) {
		sum = 0;
		sum += *(rgba->pixels + i * 4);
		sum += *(rgba->pixels + i * 4 + 1);
		sum += *(rgba->pixels + i * 4 + 2);
		*p++ = sum / 3;
	}
	free(rgba->pixels);
	rgba->pixels = new_pixels;
	rgba->channels = 1;
	return rgba;
}

utim_image_t *image_gray_from_rgb(utim_image_t *rgb)
{
	byte *p;
	utim_image_t *gray;
	int i, sum, ch_size = rgb->xsize * rgb->ysize;
	if (rgb->channels != 3 || !ch_size)
		return NULL;
	gray = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!gray)
		return NULL;
	gray->pixels = (byte*)malloc(ch_size);
	if (!gray->pixels) {
		free(gray);
		return NULL;
	}
	p = gray->pixels;
	for (i = 0; i < ch_size; ++i) {
		sum = 0;
		sum += *(rgb->pixels + i * 3);
		sum += *(rgb->pixels + i * 3 + 1);
		sum += *(rgb->pixels + i * 3 + 2);
		*p++ = sum / 3;
	}
	gray->xsize = rgb->xsize;
	gray->ysize = rgb->ysize;
	gray->channels = 1;
	return gray;
}

utim_image_t *image_gray2rgb(utim_image_t *gray)
{
	byte *new_pixels;
	int i, off, ch_size = gray->xsize * gray->ysize;
	if (!ch_size)
		return NULL;
	new_pixels = (byte*)malloc(ch_size * 3);
	if (!new_pixels)
		return NULL;
	for (i = 0; i < ch_size; ++i) {
		off = i * 3;
		*(new_pixels + off) = *(gray->pixels + i);
		*(new_pixels + off + 1) = *(gray->pixels + i);
		*(new_pixels + off + 2) = *(gray->pixels + i);
	}
	free(gray->pixels);
	gray->pixels = new_pixels;
	gray->channels = 3;
	return gray;
}

utim_image_t *image_gray2rgba(utim_image_t *gray)
{
	byte *new_pixels;
	int i, off, ch_size = gray->xsize * gray->ysize;
	if (!ch_size)
		return NULL;
	new_pixels = (byte*)malloc(ch_size * 4);
	if (!new_pixels)
		return NULL;
	for (i = 0; i < ch_size; ++i) {
		off = i * 4;
		*(new_pixels + off) = *(gray->pixels + i);
		*(new_pixels + off + 1) = *(gray->pixels + i);
		*(new_pixels + off + 2) = *(gray->pixels + i);
		*(new_pixels + off + 3) = 255;
	}
	free(gray->pixels);
	gray->pixels = new_pixels;
	gray->channels = 4;
	return gray;
}

utim_image_t *image_rgb2rgba(utim_image_t *rgb)
{
	byte *new_pixels;
	int i, off_rgba, off_rgb, ch_size = rgb->xsize * rgb->ysize;
	if (!ch_size)
		return NULL;
	new_pixels = (byte*)malloc(ch_size * 4);
	if (!new_pixels)
		return NULL;
	for (i = 0; i < ch_size; ++i) {
		off_rgb  = i * 3;
		off_rgba = i * 4;
		*(new_pixels + off_rgba) = *(rgb->pixels + off_rgb);
		*(new_pixels + off_rgba + 1) = *(rgb->pixels + off_rgb + 1);
		*(new_pixels + off_rgba + 2) = *(rgb->pixels + off_rgb + 2);
		*(new_pixels + off_rgba + 3) = 255;
	}
	free(rgb->pixels);
	rgb->pixels = new_pixels;
	rgb->channels = 4;
	return rgb;
}

utim_image_t *image_rgb_from_gray(utim_image_t *gray)
{
	utim_image_t *rgb;
	int i, off, ch_size = gray->xsize * gray->ysize;
	if (!ch_size)
		return NULL;
	rgb = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!rgb)
		return NULL;
	rgb->pixels = (byte*)malloc(ch_size * 3);
	if (!rgb->pixels) {
		free(rgb);
		return NULL;
	}
	for (i = 0; i < ch_size; ++i) {
		off = i * 3;
		*(rgb->pixels + off + 0) = *(gray->pixels + i);
		*(rgb->pixels + off + 1) = *(gray->pixels + i);
		*(rgb->pixels + off + 2) = *(gray->pixels + i);
	}
	rgb->xsize = gray->xsize;
	rgb->ysize = gray->ysize;
	rgb->channels = 3;
	return rgb;
}

utim_image_t *image_create(int x, int y, int nch, int c)
{
	utim_image_t *img;
	int size = x * y * nch;
	if (!size || nch > UTIM_MAX_CHANNELS)
		return NULL;
	img = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!img)
		return NULL;
	img->pixels = (byte*)malloc(size);
	if (!img->pixels) {
		free(img);
		return NULL;
	}
	memset(img->pixels, c, size);
	img->xsize = x;
	img->ysize = y;
	img->channels = nch;
	return img;
}

utim_image_t *image_stack(utim_image_t **chx, int nch)
{
	byte *p;
	utim_image_t *img;
	int i, j, ch_size = (*chx)->xsize * (*chx)->ysize;
	if (nch > UTIM_MAX_CHANNELS || nch <= 0)
		return NULL;
	for (i = 1; i < nch; ++i) { /* size check */
		if (chx[0]->xsize != chx[i]->xsize ||
			chx[0]->ysize != chx[i]->ysize)
			return NULL;
	}
	img = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!img)
		return NULL;
	img->pixels = (byte*)malloc(ch_size * nch);
	if (!img->pixels) {
		free(img);
		return NULL;
	}
	p = img->pixels;
	for (i = 0; i < ch_size; ++i) {
		for (j = 0; j < nch; ++j)
			*(p + i * nch + j) = chx[j]->pixels[i];
	}
	img->xsize = chx[0]->xsize;
	img->ysize = chx[0]->ysize;
	img->channels = nch;
	return img;
}

utim_image_t *image_pick_chl(utim_image_t *img, int ich)
{
	byte *p;
	utim_image_t *ch;
	int i, ch_size = img->xsize * img->ysize;
	if (!(ich < img->channels) || !ch_size)
		return NULL;
	ch = (utim_image_t*)malloc(sizeof(utim_image_t));
	if (!ch)
		return NULL;
	ch->pixels = (byte*)malloc(ch_size);
	if (!ch->pixels) {
		free(ch);
		return NULL;
	}
	p = ch->pixels;
	for (i = 0; i < ch_size; ++i)
		*p++ = *(img->pixels + i * img->channels + ich);
	ch->xsize = img->xsize;
	ch->ysize = img->ysize;
	ch->channels = 1;
	return ch;
}

utim_image_t *image_set_opacity(utim_image_t *img, int opacity)
{
	byte *c;
	int i, sum, ch_size;
	switch (img->channels) {
		case 1:
			if (!image_gray2rgba(img))
				return NULL;
		case 3:
			if (!image_rgb2rgba(img))
				return NULL;
		case 4: /* RGBA */
			break;
		default:
			return NULL;
	}
	ch_size = img->xsize * img->ysize;
	c = img->pixels;
	for (i = 0; i < ch_size; ++i) {
		sum = 0;
		sum += *c++;
		sum += *c++;
		sum += *c++;
		if (sum)
			*c = opacity;
		c++;
	}
	return img;
}

utim_image_t *image_set_color(utim_image_t *img, int ich, int color)
{
	byte *c;
	int i, ch_size;
	if (ich >= img->channels)
		return NULL;
	ch_size = img->xsize * img->ysize;
	c = img->pixels;
	for (i = 0; i < ch_size; ++i) {
		*(c + ich) = color;
		c += img->channels;
	}
	return img;
}

/* _compose_pixel: used for implementing image_superpose only */
#ifndef CONFIG_STD_C89
	inline
#endif
static void _compose_pixel(utim_image_t *bg,
	utim_image_t *im, int i, int j, utim_point_t *p)
{
	int k, index_bg, index_im;
	utim_color_t c_bg, c_im;
	index_bg = _index_calc(bg->xsize, bg->ysize, i, j);
	index_im = _index_calc(im->xsize, im->ysize, i - p->x, j - p->y);
	byte *rb = &c_bg.rgba[UTIM_COLOR_R];
	byte *gb = &c_bg.rgba[UTIM_COLOR_G];
	byte *bb = &c_bg.rgba[UTIM_COLOR_B];
	byte *ab = &c_bg.rgba[UTIM_COLOR_A];
	byte *ri = &c_im.rgba[UTIM_COLOR_R];
	byte *gi = &c_im.rgba[UTIM_COLOR_G];
	byte *bi = &c_im.rgba[UTIM_COLOR_B];
	byte *ai = &c_im.rgba[UTIM_COLOR_A];
	if (bg->channels == 4) { /* RGBA */
		for (k = 0; k < bg->channels; ++k)
			c_bg.rgba[k] =
				bg->pixels[index_bg * bg->channels + k];
		memset(c_im.rgba, UTIM_BACK_COLOR, sizeof(c_im.rgba));
		for (k = 0; k < im->channels; ++k)
			c_im.rgba[k] =
				im->pixels[index_im * bg->channels + k];
                *rb = ((255 - *ai) * (*rb) + (*ai) * (*ri)) / 255;
                *gb = ((255 - *ai) * (*gb) + (*ai) * (*gi)) / 255;
                *bb = ((255 - *ai) * (*bb) + (*ai) * (*bi)) / 255;
                *ab = *ab + ((255 - *ab) * (*ai) / 255);
		for (k = 0; k < bg->channels; ++k)
			bg->pixels[index_bg * bg->channels + k] = 
							c_bg.rgba[k];

	} else { /* Direct copy */
		for (k = 0; k < bg->channels; ++k)
			bg->pixels[index_bg + k] = im->pixels[index_im + k];
	}
}

utim_image_t *image_superpose(utim_image_t *bg,
	utim_image_t *img, utim_point_t *p)
{
	int i, j;
	if (p->x >= bg->xsize || p->y >= bg->ysize)
		return bg;
	for (j = p->y; j < bg->ysize; ++j) {
		for (i = p->x; i < bg->xsize; ++i)
			_compose_pixel(bg, img, i, j, p);
	}
	return bg;
}

/*
 * Simple drawing functions
 */

int image_draw_point(utim_image_t *img,
	utim_point_t *p, utim_color_t *c)
{
	int i, index = _index_calc(img->xsize, img->ysize, p->x, p->y);
	if (index < 0)
		return -1;
	for (i = 0; i < img->channels; ++i)
		img->pixels[index * img->channels + i] = c->rgba[i];
	return 0;
}

void image_draw_line(utim_image_t *img,
	utim_point_t *a, utim_point_t *b, utim_color_t *c, int width)
{
	int dx, dy, orientation, xa, xb, ya, yb, t;
	utim_point_t p;
	if (!width)
		return;
	width = width >  0 ? width : -width;
	dx = b->x - a->x;
	dx = dx < 0 ? -dx : dx;
	dy = b->y - a->y;
	dy = dy < 0 ? -dy : dy;
	if ((dx == 0) && (dy == 0)) {
		image_draw_point(img, a, c);
		return;
	}
	orientation = dx < dy ? 0 : 1;
	xa = a->x; ya = a->y;
	xb = b->x; yb = b->y;
	if (orientation) {
		if (a->x > b->x) {
			t = xa; xa = xb; xb = t;
			t = ya; ya = yb; yb = t;
		}
		for (p.x = xa; p.x <= xb; ++p.x) {
			p.y = ya + (yb - ya) * (p.x - xa) / dx;
			if (width == 1)
				image_draw_point(img, &p, c);
			else
				image_draw_filled_circle(img, &p, width, c);
		}
	} else {
		if (a->y > b->y) {
			t = xa; xa = xb; xb = t;
			t = ya; ya = yb; yb = t;
		}
		for (p.y = ya; p.y <= yb; ++p.y) {
			p.x = xa + (xb - xa) * (p.y - ya) / dy;
			if (width == 1)
				image_draw_point(img, &p, c);
			else
				image_draw_filled_circle(img, &p, width, c);
		}
	}
}

#ifndef CONFIG_STD_C89
	inline
#endif
static void _draw_circle_point(utim_image_t *img, utim_point_t *p,
	utim_point_t *center, int x, int y, utim_color_t *color, int width)
{
	p->x += x; p->y += y;
	if (width == 1)
		image_draw_point(img, p, color);
	else
		image_draw_filled_circle(img, p, width, color);
	p->x = center->x; p->y = center->y;
}
void image_draw_circle(utim_image_t *img,
	utim_point_t *center, int radius, utim_color_t *color, int width)
{
	int x, y, dx, dy, err;
	utim_point_t p;
	if (!width)
		return;
	x = radius - 1; y = 0;
	dx = 1; dy = 1; err = dx - (radius << 1);
	p.x = center->x; p.y = center->y;
	width = width >  0 ? width : -width;
	while(x >= y) {
		_draw_circle_point(img, &p, center,  x,  y, color, width);
		_draw_circle_point(img, &p, center,  y,  x, color, width);
		_draw_circle_point(img, &p, center, -y,  x, color, width);
		_draw_circle_point(img, &p, center, -x,  y, color, width);
		_draw_circle_point(img, &p, center, -x, -y, color, width);
		_draw_circle_point(img, &p, center, -y, -x, color, width);
		_draw_circle_point(img, &p, center,  y, -x, color, width);
		_draw_circle_point(img, &p, center,  x, -y, color, width);
		if (err <= 0) {
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0) {
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
}

void image_draw_filled_circle(utim_image_t *img,
	utim_point_t *center, int radius, utim_color_t *color)
{
	int x, y, r2;
	utim_point_t p, bounds[2];
	if (radius <= 0) return;
	bounds[0].x = center->x - radius;
	bounds[1].x = center->x + radius;
	bounds[0].y = center->y - radius;
	bounds[1].y = center->y + radius;

	for (x = bounds[0].x; x <= bounds[1].x; ++x) {
		for (y = bounds[0].y; y <= bounds[1].y; ++y) {
			p.x = x;
			p.y = y;
			r2 = radius * radius;
			if(((p.x - center->x) * (p.x - center->x) +
				(p.y - center->y) * (p.y - center->y)) < r2)
				image_draw_point(img, &p, color);
		}
	}
}