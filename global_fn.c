#include "sys_fn.h"
#include "utim/util_image.h"
#include "random_draw.h"

/*
 * Preview window Control:
 */

static GLuint global_preview_tex;
static int global_flag_file_loaded;
static int global_flag_preview_update;
static const float global_preview_offx =  5;
static const float global_preview_offy =  35;
static const float global_preview_offw = -55;
static const float global_preview_offh = -55;
static const float global_preview_base =  780;
static float global_preview_width;
static float global_preview_height;
static struct nk_rect global_preview_rect =
		{.x = 10, .y = 10, .w = 780, .h = 780};
static UTIM_IMG *global_preview_img;

/*
 * Control Panel window Control:
 */

static struct nk_rect global_ctrl_panel_rect =
		{.x = 880, .y = 10, .w = 280, .h = 750};


/*
 * Render Control:
 */

enum { /* Stroke Types */
	ST_RADIATION,
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_SQUARE,
	ST_ANNULUS,
	ST_ENUM_END,
};
static int global_stroke_type;

static UTIM_IMG *global_render_raw;
static int global_flag_render;
static int global_render_ctrl_v0;
static int global_render_ctrl_v1 = 5;
static int global_render_ctrl_v2 = 5;
static int global_render_ctrl_v3;
static float global_draw_center_ratio_x;
static float global_draw_center_ratio_y;
static int global_flag_render_cancel;
static nk_size global_flag_prog_max;    /* Rerendering Progress Max */
static nk_size global_flag_rendering;   /* Rerendering Progress Cursor */

static UTIM_IMG *load_image(const char *filename)
{
	UTIM_IMG *imgdata = utim_read(filename);
	if (!imgdata)
		return NULL;
	if (utim_img2rgba(imgdata)) {
		utim_free_image(imgdata);
		return NULL;
	}
	return imgdata;
}

static GLuint update_image(UTIM_IMG *imgdata, struct nk_image *img)
{
	GLuint tex;
	struct nk_image timg;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgdata->xsize,
		imgdata->ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgdata->pixels);
	timg = nk_image_id((int)tex);
	memcpy(img, &timg, sizeof(struct nk_image));
	return tex;
}
