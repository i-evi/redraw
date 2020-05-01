#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>

#define LR_CTRL_MAX 11 /* Length/Radius Ctrl(1~10) */
#define MAX_CALL_N_JOBS 1000

static void *__call_render(void *arg)
{
	int i, njob, iter_count;
	utim_point_t center;
	utim_image_t *raw = global_render_raw;
	if (global_preview_img)
		utim_free_image(global_preview_img);
	global_preview_img = utim_create(raw->xsize, raw->ysize, 4, 255);
	njob = global_render_ctrl_v0 / 100;
	if (njob > MAX_CALL_N_JOBS) {
		njob = MAX_CALL_N_JOBS;
		iter_count = global_render_ctrl_v0 / njob;
	} else if (!njob) {
		iter_count = global_render_ctrl_v0;
		njob = 1;
	} else {
		iter_count = 100;
	}
	utim_set_draw_point_fn(utim_set_point);
	for (i = 0; i < iter_count; ++i) {
		if (global_flag_render_cancel) {
			global_flag_render_cancel = 0;
			break;
		}
		center[0] = (int)(global_draw_center_ratio_x * (raw->xsize - 1));
		center[1] = (int)(global_draw_center_ratio_y * (raw->ysize - 1));
		switch (global_stroke_type) {
			case ST_RADIATION:
				random_redraw_radiation(raw, global_preview_img,
					center, njob, LR_CTRL_MAX - global_render_ctrl_v2,
					global_render_ctrl_v1, i, global_render_ctrl_v3);
				break;
			case ST_HORIZONTAL:
				random_redraw_horizontal(raw, global_preview_img,
					njob, LR_CTRL_MAX - global_render_ctrl_v2,
					global_render_ctrl_v1, i, global_render_ctrl_v3);
				break;
			case ST_VERTICAL:
				random_redraw_vertical(raw, global_preview_img,
					njob, LR_CTRL_MAX - global_render_ctrl_v2,
					global_render_ctrl_v1, i, global_render_ctrl_v3);
				break;
			case ST_SQUARE:
				random_redraw_square(raw, global_preview_img,
					njob, LR_CTRL_MAX - global_render_ctrl_v2,
					global_render_ctrl_v1, i, global_render_ctrl_v3);
				break;
			case ST_ANNULUS:
				random_redraw_annulus(raw, global_preview_img,
					njob, LR_CTRL_MAX - global_render_ctrl_v2,
					global_render_ctrl_v1, i, global_render_ctrl_v3);
			default:
				break;
		}
		global_flag_preview_update = 1;
		global_flag_rendering += njob;
	}
	global_flag_preview_update = 1;
	global_flag_render = 0;
	global_flag_rendering = 0;
	return NULL;
}

static void preview(struct nk_context *ctx)
{
	pthread_t pid;
	static int flag_init;
	static struct nk_image *img;
	static float width;
	static float height;
	static struct nk_rect view_rect;
	if (!flag_init) {
		img = (struct nk_image*)malloc(sizeof(struct nk_image));
		global_preview_img = utim_create(100, 100, 4, 45);
		global_preview_tex = update_image(global_preview_img, img);
		flag_init = 1;
	}
	/*
	 * Update Preview
	 */
	if (global_flag_preview_update) {
		/* Release texture */
		glDeleteTextures(1, &global_preview_tex);
		global_preview_tex = update_image(global_preview_img, img);
		if (global_preview_img->xsize > global_preview_img->ysize) {
			nk_window_set_size(ctx, "Preview",
				nk_vec2(global_preview_base, global_preview_base*
				global_preview_img->ysize/global_preview_img->xsize));
		} else {
			nk_window_set_size(ctx, "Preview",
				nk_vec2(global_preview_base*global_preview_img->xsize/
				global_preview_img->ysize, global_preview_base));	
		}
		global_flag_preview_update = 0;
	}
	view_rect = nk_rect(
			global_preview_rect.x + global_preview_offx,
			global_preview_rect.y + global_preview_offy,
			global_preview_rect.w + global_preview_offw,
			global_preview_rect.h + global_preview_offh);
	if (nk_begin(ctx, "Preview", global_preview_rect,
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_TITLE))
	{
		global_preview_rect = nk_window_get_bounds(ctx);
		nk_layout_row_static(ctx, view_rect.h, view_rect.w, 1);
		nk_image(ctx, *img);
	}
	if (nk_window_is_active(ctx, "Preview") &&
		nk_input_is_mouse_click_in_rect(
			&ctx->input, NK_BUTTON_LEFT, view_rect)) {
		global_draw_center_ratio_x =
			(ctx->input.mouse.pos.x-view_rect.x)/view_rect.w;
		global_draw_center_ratio_y =
			(ctx->input.mouse.pos.y-view_rect.y)/view_rect.h;
		if (global_draw_center_ratio_x > 1.)
			global_draw_center_ratio_x = 1.;
		if (global_draw_center_ratio_y > 1.)
			global_draw_center_ratio_y = 1.;
		if (!global_flag_render && global_flag_file_loaded) {
			global_flag_render_cancel = 0;
			pthread_create(&pid, NULL, __call_render, NULL);
			global_flag_render = 1;
		}
	}
	nk_end(ctx);
}
