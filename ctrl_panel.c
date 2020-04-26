#define _WIDGET_W 85
#define _WIDGET_H 30

#define _MIN_LINES_INT 100
#define _MIN_LINES_STR "100"

static char buf_dl[64] = "1000";
static char buf_lw[64] = "3";
static char buf_lc[64] = "3";
static void ctrl_panel(struct nk_context *ctx)
{
	int result;
	nk_flags event;
	const char *openfile;
	if (nk_begin(ctx, "Control Panel", global_ctrl_panel_rect,
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
	{
		/*=============================================
		 * Help:
		 *=============================================*/
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_label_wrap(ctx, "Help:");
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_label_wrap(ctx, "Open an image file, and click on it");
		/*=============================================
		 * Button: Open File / Save File / Submit
		 *=============================================*/
		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		if (nk_button_label(ctx, "Open File")) {
			openfile = sys_gui_select_file("File selection", SYS_GUI_OPEN);
			if (openfile) {
				free_image(global_preview_img);
				global_preview_img = load_image(openfile);
				if (global_preview_img) {
					global_flag_file_loaded    = 1;
					global_flag_preview_update = 1;
					if (global_render_raw)
						free_image(global_render_raw);
					global_render_raw = image_clone(global_preview_img);
				}
				else
					fprintf(stderr, "Failed open file %s\n", openfile);
			}
		}
		if (nk_button_label(ctx, "Save File")) {
			openfile = sys_gui_select_file("Save file", SYS_GUI_SAVE);
			if (openfile)
				result = image_write(openfile, global_preview_img);
		}

		/* nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W, 2);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Submit")) {
		} */
		/*=============================================
		 * Editor: Draw Lines / Line Width / LengthCtrl
		 *=============================================*/
		nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W * 2, 1);
		nk_label(ctx, "Drawing Control:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		nk_label(ctx, "Draw Lines(>=100):", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(ctx,
				NK_EDIT_BOX|NK_EDIT_AUTO_SELECT,
			buf_dl, sizeof(buf_dl), nk_filter_decimal);

		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		nk_label(ctx, "Line Width(0~9):", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(ctx,
				NK_EDIT_BOX|NK_EDIT_AUTO_SELECT, buf_lw, 2,
			nk_filter_decimal);

		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		nk_label(ctx, "LengthCtrl(0~9):", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(ctx,
				NK_EDIT_BOX|NK_EDIT_AUTO_SELECT, buf_lc, 2,
			nk_filter_decimal);

		global_draw_lines = atoi(buf_dl);
		if (global_draw_lines <= 100) {
			global_draw_lines = _MIN_LINES_INT;
			strcpy(buf_dl, _MIN_LINES_STR);
		}

		global_line_width = atoi(buf_lw);
		if (global_line_width <= 0)
			global_draw_lines = 1;

		global_lengthCtrl = atoi(buf_lc);
		if (global_lengthCtrl <= 0)
			global_lengthCtrl = 0;

		global_flag_prog_max = global_draw_lines;

		nk_layout_row_dynamic(ctx, _WIDGET_H, 0);
		/*=============================================
		 * Progree Bar:
		 *=============================================*/
		nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W * 2, 1);
		nk_label_wrap(ctx, "Render Progress:");
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		if (nk_progress(ctx, &global_flag_rerendering, global_flag_prog_max, 0)) {
			/* code */
		}
	}
	nk_end(ctx);
}

#undef _WIDGET_W
#undef _WIDGET_H