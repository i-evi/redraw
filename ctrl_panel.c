#define _WIDGET_W 85
#define _WIDGET_H 30

#define _MIN_LINES_INT 100
#define _MIN_LINES_STR "100"

static char buf_v0[64] = "1000";
/* static char buf_v1[64] = "3"; */
/* static char buf_v2[64] = "3"; */
static const char *editor_hints[] = {
	"Unavailable",                    /* 0 */
	"Draw Elements(>0):",             /* 1 */
	"Line Width(1~10):",              /* 2 */
	"RadiusCtrl(1~10):",              /* 3 */
	"LengthCtrl(1~10):",              /* 4 */
};

static int editor_hint_id;
static void ctrl_panel(struct nk_context *ctx)
{
	int i, result;
	nk_flags event;
	const char *openfile;
	static const char *stroke_types[] = 
		{"Radiation","Horizontal","Vertical","Square","Annulus"};
	static int check = 1;
	static int selected_item = 0;
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
		nk_label_wrap(ctx, "Open an image file, and click on the Preview window");
		nk_layout_row_dynamic(ctx, _WIDGET_H, 0);
		/*=============================================
		 * Button: Open File / Save File
		 *=============================================*/
		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		if (nk_button_label(ctx, "Open File")) {
			openfile = sys_gui_select_file("File selection", SYS_GUI_OPEN);
			if (openfile) {
				utim_free_image(global_preview_img);
				global_preview_img = load_image(openfile);
				if (global_preview_img) {
					global_flag_file_loaded    = 1;
					global_flag_preview_update = 1;
					if (global_render_raw)
						utim_free_image(global_render_raw);
					global_render_raw = utim_clone(global_preview_img);
				}
				else
					fprintf(stderr, "Failed open file %s\n", openfile);
			}
		}
		if (nk_button_label(ctx, "Save File")) {
			openfile = sys_gui_select_file("Save file", SYS_GUI_SAVE);
			if (openfile)
				result = utim_write(openfile, global_preview_img);
		}

		/*=============================================
		 * Editor: Draw Lines / Line Width / LengthCtrl
		 *=============================================*/
		nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W * 2, 1);
		nk_label(ctx, "Drawing Control:", NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, _WIDGET_H, 2);
		editor_hint_id = 1;
		nk_label(ctx, editor_hints[editor_hint_id], NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(ctx,
				NK_EDIT_BOX|NK_EDIT_AUTO_SELECT,
			buf_v0, sizeof(buf_v0), nk_filter_decimal);

		switch (selected_item) {
			case ST_RADIATION:
			case ST_HORIZONTAL:
			case ST_VERTICAL:
			case ST_SQUARE:
			case ST_ANNULUS:
				editor_hint_id = 2; break;
			default:
				editor_hint_id = 0; break;
		}
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_property_int(ctx, editor_hints[editor_hint_id],
			1, &global_render_ctrl_v1, 10, 1, 0.5);

		switch (selected_item) {
			case ST_RADIATION:
			case ST_HORIZONTAL:
			case ST_VERTICAL:
			case ST_SQUARE:
				editor_hint_id = 4; break;
			case ST_ANNULUS:
				editor_hint_id = 3; break;
			default:
				editor_hint_id = 0; break;
		}
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_property_int(ctx, editor_hints[editor_hint_id],
			1, &global_render_ctrl_v2, 10, 1, 0.5);

		global_render_ctrl_v0 = atoi(buf_v0);
		if (global_render_ctrl_v0 < 0) {
			global_render_ctrl_v0 = _MIN_LINES_INT;
			strcpy(buf_v0, _MIN_LINES_STR);
		}

		global_flag_prog_max = global_render_ctrl_v0;

		nk_layout_row_dynamic(ctx, _WIDGET_H, 0);
		/*=============================================
		 * Check Box: Fixed Length/Radius
		 *=============================================*/
		nk_layout_row_dynamic(ctx, _WIDGET_H >> 2, 1);
		nk_label(ctx, "Check This:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_checkbox_label(ctx, "Fixed Length/Radius", &check);
		if (!check)
			global_render_ctrl_v3 = 1;
		else
			global_render_ctrl_v3 = 0;

		nk_layout_row_dynamic(ctx, _WIDGET_H, 0);
		/*=============================================
		 * Progree Bar:
		 *=============================================*/
		nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W * 2, 1);
		nk_label_wrap(ctx, "Render Progress:");
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		if (nk_progress(ctx, &global_flag_rendering, global_flag_prog_max, 0)) {
			/* code */
		}

		nk_layout_row_static(ctx, _WIDGET_H, _WIDGET_W, 2);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Cancel"))
			global_flag_render_cancel = 1;

		nk_layout_row_dynamic(ctx, _WIDGET_H, 0);
		nk_layout_row_dynamic(ctx, _WIDGET_H, 1);
		nk_label(ctx, "Stroke Type:", NK_TEXT_LEFT);
		if (nk_combo_begin_label(ctx, stroke_types[selected_item],
				nk_vec2(nk_widget_width(ctx), 200))) {
			nk_layout_row_dynamic(ctx, 25, 1);
			for (i = 0; i < ST_ENUM_END; ++i)
				if (nk_combo_item_label(ctx, stroke_types[i], NK_TEXT_LEFT))
					selected_item = i;
			nk_combo_end(ctx);
		}
		global_stroke_type = selected_item;
	}
	nk_end(ctx);
}

#undef _WIDGET_W
#undef _WIDGET_H