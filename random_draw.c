#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./utim/util_image.h"
#include "random_draw.h"

/* CAN ONLY HANDLE AN a WHILE a <= b */
#define random_int(a, b) (a+rand()%(b-a+1))

int random_point_on_line(utim_point_t *u,
	utim_point_t *v, utim_point_t *r)
{
	int l, h;
	float k, b, dx, dy;
	if (u->x == v->x && u->y == v->y)
		return -1;
	dx = u->x - v->x;
	dy = u->y - v->y;
	if (dx == 0) {
		l = u->y > v->y ? v->y : u->y;
		h = u->y < v->y ? v->y : u->y;
		r->x = u->x;
		r->y = random_int(l, h);
	} else {
		k = dy / dx;
		b = (float)u->y - k * u->x;
		l = u->x > v->x ? v->x : u->x;
		h = u->x < v->x ? v->x : u->x;
		r->x = random_int(l, h);
		r->y = (int)(k * r->x + b);
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
	utim_point_t *p, utim_color_t *c)
{
	int i, index = _index_calc(img->xsize, img->ysize, p->x, p->y);
	if (index < 0)
		return -1;
	for (i = 0; i < img->channels; ++i)
		c->rgba[i] = img->pixels[index * img->channels + i];
	return 0;
}

void get_line_avg_color(utim_image_t *img,
	utim_point_t *a, utim_point_t *b, utim_color_t *c)
{
	int i, point_cnt = 0, rgba[4] = {0};
	int dx, dy, orientation, xa, xb, ya, yb, t;
	utim_point_t p;
	utim_color_t c_tmp;
	dx = b->x - a->x;
	dx = dx < 0 ? -dx : dx;
	dy = b->y - a->y;
	dy = dy < 0 ? -dy : dy;
	if ((dx == 0) && (dy == 0)) {
		get_point_color(img, a, c);
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
			if (!get_point_color(img, &p, &c_tmp)) {
				for (i = 0; i < 4; ++i)
					rgba[i] += c_tmp.rgba[i];
				point_cnt++;
			}
		}
	} else {
		if (a->y > b->y) {
			t = xa; xa = xb; xb = t;
			t = ya; ya = yb; yb = t;
		}
		for (p.y = ya; p.y <= yb; ++p.y) {
			p.x = xa + (xb - xa) * (p.y - ya) / dy;
			if (!get_point_color(img, &p, &c_tmp)) {
				for (i = 0; i < 4; ++i)
					rgba[i] += c_tmp.rgba[i];
				point_cnt++;
			}
		}
	}
	for (i = 0; i < 4; ++i)
		c->rgba[i] = rgba[i] / point_cnt;
}

utim_image_t *random_redraw_radiation(utim_image_t *img, utim_image_t *buf,
	utim_point_t *center, int nline, int length_ctrl, int width, int seed)
{
	int i, lli;
	utim_color_t c;
	utim_point_t a;
	utim_point_t b;
	srand(clock() + (seed << 1));
	for (i = 0; i < nline; ++i) {
		b.x = rand() % buf->xsize;
		b.y = rand() % buf->ysize;
		random_point_on_line(center, &b, &a);
		lli = length_ctrl;
		while (lli--)
			random_point_on_line(&a, &b, &a);
		get_line_avg_color(img, &a, &b, &c);
		image_draw_line(buf, &a, &b, &c, random_int(1, width));
	}
	return buf;
}