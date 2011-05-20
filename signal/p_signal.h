#ifndef _P_SIGNAL_H
#define _P_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "p_global.h"
#include <glib.h>
#if 0
typedef struct _scale_args {
	ScaleOption opt;
	const char *inFile;
	const char *outFile;
	gRect rect;
}SCALEARGS;
#endif


typedef struct _struct_gtkwidget {
	GtkWidget *gw1;
	GtkWidget *gw2;
	GtkHScale *hs;
	GtkVScale *vs;
}GtkWidgetList;

typedef struct scale_list {
	GtkWidget *frame;
	GtkHScale *hs;
	GtkVScale *vs;
}GScale;

gboolean on_quit_window(GtkWidget *widget, GdkEvent *event, gpointer data);

void on_open_filedialog(GtkWidget *widget, gpointer data);

void on_scale_changed(GtkWidget *widget, gpointer data);

void on_save(GtkWidget *widget, gpointer data);

void on_save_as(GtkWidget *widget, gpointer data);

void on_transform(GtkWidget *widget, gpointer data);

void on_grab_whole_screen(GtkWidget *widget, gpointer data);

void on_grabwindow(GtkWidget *widget, gpointer data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
