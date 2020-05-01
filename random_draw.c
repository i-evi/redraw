#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./utim/util_image.h"
#include "random_draw.h"

/* CAN ONLY HANDLE AN a WHILE a <= b */
#define random_int(a, b) (a+rand()%(b-a+1))

int random_point_on_line(utim_point_t u, utim_point_t v, utim_point_t r)
{
	int l, h;
	float k, b, dx, dy;
	if (u[0] == v[0] && u[1] == v[1])
		return -1;
	dx = u[0] - v[0];
	dy = u[1] - v[1];
	if (dx == 0) {
		l = u[1] > v[1] ? v[1] : u[1];
		h = u[1] < v[1] ? v[1] : u[1];
		r[0] = u[0];
		r[1] = random_int(l, h);
	} else {
		k = dy / dx;
		b = (float)u[1] - k * u[0];
		l = u[0] > v[0] ? v[0] : u[0];
		h = u[0] < v[0] ? v[0] : u[0];
		r[0] = random_int(l, h);
		r[1] = (int)(k * r[0] + b);
	}
	return 0;
}

int _index_calc(int xsize, int ysize, int x, int y)
{
	int size, index;
	if (x >= xsize || y >= ysize || x < 0 || y < 0)
		return -1;
	size = xsize * ysize;
	index = xsize * y + x;
	return index;
}

int get_point_color(utim_image_t *img,
	utim_point_t p, utim_color_t c)
{
	int i, index = _index_calc(img->xsize, img->ysize, p[0], p[1]);
	if (index < 0)
		return -1;
	for (i = 0; i < img->channels; ++i)
		c[i] = img->pixels[index * img->channels + i];
	return 0;
}

void get_line_avg_color(utim_image_t *img,
	utim_point_t a, utim_point_t b, utim_color_t c)
{
	int i, point_cnt = 0, rgba[4] = {0};
	int dx, dy, orientation, xa, xb, ya, yb, t;
	utim_point_t p;
	utim_color_t c_tmp;
	dx = b[0] - a[0];
	dx = dx < 0 ? -dx : dx;
	dy = b[1] - a[1];
	dy = dy < 0 ? -dy : dy;
	if ((dx == 0) && (dy == 0)) {
		get_point_color(img, a, c);
		return;
	}
	orientation = dx < dy ? 0 : 1;
	xa = a[0]; ya = a[1];
	xb = b[0]; yb = b[1];
	if (orientation) {
		if (a[0] > b[0]) {
			t = xa; xa = xb; xb = t;
			t = ya; ya = yb; yb = t;
		}
		for (p[0] = xa; p[0] <= xb; ++p[0]) {
			p[1] = ya + (yb - ya) * (p[0] - xa) / dx;
			if (!get_point_color(img, p, c_tmp)) {
				for (i = 0; i < 4; ++i)
					rgba[i] += c_tmp[i];
				point_cnt++;
			}
		}
	} else {
		if (a[1] > b[1]) {
			t = xa; xa = xb; xb = t;
			t = ya; ya = yb; yb = t;
		}
		for (p[1] = ya; p[1] <= yb; ++p[1]) {
			p[0] = xa + (xb - xa) * (p[1] - ya) / dy;
			if (!get_point_color(img, p, c_tmp)) {
				for (i = 0; i < 4; ++i)
					rgba[i] += c_tmp[i];
				point_cnt++;
			}
		}
	}
	for (i = 0; i < 4; ++i)
		c[i] = rgba[i] / point_cnt;
}

#ifndef CONFIG_STD_C89
	inline
#endif
static void _c_point_color(utim_image_t *img, utim_point_t p,
	utim_point_t center, int x, int y, int *rgba, int *point_cnt)
{
	int i;
	utim_color_t color; 
	p[0] += x; p[1] += y;
	if (!get_point_color(img, p, color)) {
		for (i = 0; i < 4; ++i)
			rgba[i] += color[i];
		(*point_cnt)++;
	}
	p[0] = center[0]; p[1] = center[1];
}
void get_cycle_avg_color(utim_image_t *img,
	utim_point_t center, int radius, utim_color_t color)
{
	utim_point_t p;
	int i, x, y, dx, dy, err, point_cnt = 0, rgba[4] = {0};
	x = radius - 1; y = 0;
	dx = 1; dy = 1; err = dx - (radius << 1);
	p[0] = center[0]; p[1] = center[1];
	while(x >= y) {
		_c_point_color(img, p, center,  x,  y, rgba, &point_cnt);
		_c_point_color(img, p, center,  y,  x, rgba, &point_cnt);
		_c_point_color(img, p, center, -y,  x, rgba, &point_cnt);
		_c_point_color(img, p, center, -x,  y, rgba, &point_cnt);
		_c_point_color(img, p, center, -x, -y, rgba, &point_cnt);
		_c_point_color(img, p, center, -y, -x, rgba, &point_cnt);
		_c_point_color(img, p, center,  y, -x, rgba, &point_cnt);
		_c_point_color(img, p, center,  x, -y, rgba, &point_cnt);
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
	if (!point_cnt)
		return;
	for (i = 0; i < 4; ++i)
		color[i] = rgba[i] / point_cnt;
}

static void get_rect_avg_color(utim_image_t *img,
	utim_point_t a, int w, int h, utim_color_t c)
{
	utim_point_t p;
	utim_color_t c_tmp;
	int i, j, point_cnt = 0, rgba[4] = {0};
	if (!w || !h)
		return;
	p[UTIM_POINT_X] = a[UTIM_POINT_X];
	p[UTIM_POINT_Y] = a[UTIM_POINT_Y];
	for (i = 0; i < w; ++i) {
		get_point_color(img, p, c_tmp);
		for (j = 0; j < 4; ++j)
			rgba[j] += c_tmp[j];
		point_cnt++;
		p[UTIM_POINT_X]++;
	}
	for (i = 0; i < h; ++i) {
		get_point_color(img, p, c_tmp);
		for (j = 0; j < 4; ++j)
			rgba[j] += c_tmp[j];
		point_cnt++;
		p[UTIM_POINT_Y]++;
	}
	for (i = 0; i < w; ++i) {
		get_point_color(img, p, c_tmp);
		for (j = 0; j < 4; ++j)
			rgba[j] += c_tmp[j];
		point_cnt++;
		p[UTIM_POINT_X]--;
	}
	for (i = 0; i < h; ++i) {
		get_point_color(img, p, c_tmp);
		for (j = 0; j < 4; ++j)
			rgba[j] += c_tmp[j];
		point_cnt++;
		p[UTIM_POINT_Y]--;
	}
	for (i = 0; i < 4; ++i)
		c[i] = rgba[i] / point_cnt;
	return;
}

utim_image_t *random_redraw_radiation(utim_image_t *img,
		utim_image_t *buf, utim_point_t center, int nline,
	int length_ctrl, int width, int seed, int fixed)
{
	int i, lli;
	utim_color_t c;
	utim_point_t a;
	utim_point_t b;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		b[0] = rand() % buf->xsize;
		b[1] = rand() % buf->ysize;
		random_point_on_line(center, b, a);
		if (!fixed) {
			lli = length_ctrl;
			while (lli--)
				random_point_on_line(a, b, a);
		}
		get_line_avg_color(img, a, b, c);
		utim_draw_line(buf, a, b, c, random_int(1, width));
	}
	return buf;
}

utim_image_t *random_redraw_horizontal(utim_image_t *img, utim_image_t *buf,
	int nline, int length_ctrl, int width, int seed, int fixed)
{
	int i, lli;
	utim_color_t c;
	utim_point_t a;
	utim_point_t b;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		a[0] = rand() % buf->xsize;
		a[1] = rand() % buf->ysize;
		b[1] = a[1];
		if (fixed){
			length_ctrl = length_ctrl < 1 ? 1 : length_ctrl;
			b[0] = a[0] + buf->xsize / (length_ctrl * length_ctrl);
		} else {
			b[0] = rand() % buf->xsize;
			lli = length_ctrl;
			while (lli--)
				random_point_on_line(a, b, a);
		}
		get_line_avg_color(img, a, b, c);
		utim_draw_line(buf, a, b, c, random_int(1, width));
	}
	return buf;
}

utim_image_t *random_redraw_vertical(utim_image_t *img, utim_image_t *buf,
	int nline, int length_ctrl, int width, int seed, int fixed)
{
	int i, lli;
	utim_color_t c;
	utim_point_t a;
	utim_point_t b;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		a[0] = rand() % buf->xsize;
		a[1] = rand() % buf->ysize;
		b[0] = a[0];
		if (fixed){
			length_ctrl = length_ctrl < 1 ? 1 : length_ctrl;
			b[1] = a[1] + buf->xsize / (length_ctrl * length_ctrl);
		} else {
			b[1] = rand() % buf->ysize;
			lli = length_ctrl;
			while (lli--)
				random_point_on_line(a, b, a);
		}
		get_line_avg_color(img, a, b, c);
		utim_draw_line(buf, a, b, c, random_int(1, width));
	}
	return buf;
}

utim_image_t *random_redraw_annulus(utim_image_t *img, utim_image_t *buf,
	int nline, int radius_ctrl, int width, int seed, int fixed)
{
	int i, lli, radius;
	utim_color_t color;
	utim_point_t center;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		center[0] = rand() % buf->xsize;
		center[1] = rand() % buf->ysize;
		if (fixed) {
			radius_ctrl = radius_ctrl < 1 ? 1 : radius_ctrl;
			radius = buf->xsize / (radius_ctrl * radius_ctrl);
		} else {
			lli = radius_ctrl;
			radius = buf->xsize;
			while (lli--)
				radius = random_int(1, radius);
		}
		get_cycle_avg_color(img, center, radius, color);
		utim_draw_circle(buf, center,
			radius, color, random_int(1, width));
	}
	return buf;
}

utim_image_t *random_redraw_square(utim_image_t *img, utim_image_t *buf,
	int nline, int length_ctrl, int width, int seed, int fixed)
{
	int i, lli, edge;
	utim_point_t p;
	utim_color_t color;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		if (fixed) {
			length_ctrl = length_ctrl < 1 ? 1 : length_ctrl;
			edge = buf->xsize / (length_ctrl * length_ctrl);
		} else {
			lli = length_ctrl;
			edge = buf->xsize;
			while (lli--)
				edge = random_int(1, edge);
		}
		p[0] = rand() % (buf->xsize - edge);
		p[1] = rand() % (buf->ysize - edge);
		get_rect_avg_color(img, p, edge, edge, color);
		utim_draw_rect(buf, p, edge, edge, color, random_int(1, width));
	}
	return buf;
}