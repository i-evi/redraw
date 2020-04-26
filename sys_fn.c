#include "sys_fn.h"

#ifdef USING_GTK

#include <gtk/gtk.h>

struct _file_select_callback_args {
	GtkWidget *fs;
	const char *pathname;
};

void file_ok_sel(GtkWidget *w, struct _file_select_callback_args *args)
{
	args->pathname = gtk_file_selection_get_filename(
		GTK_FILE_SELECTION((GtkFileSelection*)args->fs));
	gtk_widget_destroy(args->fs);
}

const char *sys_gui_select_file(const char *title, int mode)
{
	GtkWidget *filew;
	struct _file_select_callback_args args = {.fs=NULL, .pathname=NULL};
	gtk_init (NULL, NULL);
	filew = gtk_file_selection_new(title);
	args.fs = filew;
	g_signal_connect(G_OBJECT (filew), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(filew)->ok_button),
		"clicked", G_CALLBACK(file_ok_sel), &args);
	g_signal_connect_swapped(
		G_OBJECT(GTK_FILE_SELECTION(filew)->cancel_button),
		"clicked", G_CALLBACK(gtk_widget_destroy), filew);
	/* gtk_file_selection_set_filename(
		GTK_FILE_SELECTION(filew), "penguin.png"); */
	gtk_widget_show(filew);
	gtk_main();
	return args.pathname;
}

#endif

#ifdef USING_WINAPI

#include <Windows.h>

TCHAR _sys_gui_sf_szBuffer[128];
const char *sys_gui_select_file(const char *title, int mode)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = _sys_gui_sf_szBuffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 128;
	ofn.lpstrFilter = "Image\0*.jpg;*.png;*.bmp;*.tga;\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
	switch (mode) {
		case SYS_GUI_SAVE:
			GetSaveFileName(&ofn);
			break;
		case SYS_GUI_OPEN:
			GetOpenFileName(&ofn);
			break;
		default:
			return NULL;
	}
	if (!strlen(_sys_gui_sf_szBuffer))
		return NULL;
	return _sys_gui_sf_szBuffer;
}

#endif