#ifndef _RANDOM_DRAW_H_
#define _RANDOM_DRAW_H_

#ifdef __cplusplus
	extern "C" {
#endif

UTIM_IMG *random_redraw_radiation(UTIM_IMG *img,
		UTIM_IMG *buf, UTIM_POINT center, int nline,
	int length_ctrl, int width, int seed, int fixed);

UTIM_IMG *random_redraw_horizontal(UTIM_IMG *img, UTIM_IMG *buf,
	int nline, int length_ctrl, int width, int seed, int fixed);

UTIM_IMG *random_redraw_vertical(UTIM_IMG *img, UTIM_IMG *buf,
	int nline, int length_ctrl, int width, int seed, int fixed);

UTIM_IMG *random_redraw_square(UTIM_IMG *img, UTIM_IMG *buf,
	int nline, int length_ctrl, int width, int seed, int fixed);

UTIM_IMG *random_redraw_annulus(UTIM_IMG *img, UTIM_IMG *buf,
	int nline, int radius_ctrl, int width, int seed, int fixed);

#ifdef __cplusplus
	}
#endif

#endif /* _RANDOM_DRAW_H_ */