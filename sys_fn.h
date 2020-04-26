#ifndef _SYS_FN_H_
#define _SYS_FN_H_ value

#ifdef __cplusplus
	extern "C" {
#endif

enum {
	SYS_GUI_SAVE,
	SYS_GUI_OPEN,
};

const char *sys_gui_select_file(const char *title, int mode);

#ifdef __cplusplus
	}
#endif

#endif /* _SYS_FN_H_ */