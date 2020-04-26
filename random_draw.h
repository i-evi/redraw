#ifndef _RANDOM_DRAW_H_
#define _RANDOM_DRAW_H_

#ifdef __cplusplus
	extern "C" {
#endif

utim_image_t *random_redraw_radiation(utim_image_t *img, utim_image_t *buf,
	utim_point_t *center, int nline, int length_ctrl, int width, int seed);

#ifdef __cplusplus
	}
#endif

#endif /* _RANDOM_DRAW_H_ */