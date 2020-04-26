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
static utim_image_t *global_preview_img;

/*
 * Control Panel window Control:
 */

static struct nk_rect global_ctrl_panel_rect =
		{.x = 880, .y = 10, .w = 280, .h = 400};


/*
 * Render Control:
 */

static utim_image_t *global_render_raw;
static int global_flag_rerender;
static int global_draw_lines;
static int global_line_width;
static int global_lengthCtrl;
static float global_draw_center_ratio_x;
static float global_draw_center_ratio_y;
static nk_size global_flag_prog_max;    /* Rerendering Progress Cursor */
static nk_size global_flag_rerendering; /* Rerendering Progress Cursor */

static utim_image_t *load_image(const char *filename)
{
	utim_image_t *imgdata = image_read(filename);
	if (!imgdata)
		return NULL;
	imgdata = image_2rgba(imgdata);
	if (!imgdata) {
		free_image(imgdata);
		return NULL;
	}
	return imgdata;
}

static GLuint update_image(utim_image_t *imgdata, struct nk_image *img)
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
