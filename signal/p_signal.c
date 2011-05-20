#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "p_signal.h"
#include "transform.h"
#include <sys/param.h>
#include <unistd.h>
//#include <pthread.h>
#ifndef _P_GLOBAL_H
#include <gdk/gdkx.h>
#endif

#define IMAX(a,b) ((a)>(b)?(a):(b))
#define IMIN(a,b) ((a)<(b)?(a):(b))

#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MWM_HINTS_ELEMENTS 5

#define crosshair_width 32
#define crosshair_height 32
//#define crosshair_x_hot 15
//#define crosshair_y_hot 15
static char crosshair_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfc, 0x07, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static char crosshair_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xf0, 0x1f,
   0xfe, 0x0f, 0xf8, 0x3f, 0xfc, 0x07, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

typedef struct _mwmhints{
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    int32_t  input_mode;
    uint32_t status;
}MWMHints;

void* grab_screen(void *arg);
void* grab_widget(void *arg);
static void display(const gchar *fileName, GtkWidget *frame, GtkHScale *hs, GtkVScale *vs);
static void printError(GtkWindow *widget, const char *info);
static void showImage(GdkPixbuf* pixbuf, GtkWidget *frame, const char *fileName);
static int saveImage(GtkWidget *frame, const char *realName, const char *saveName);
static int transform(GtkWidget *frame, char *saveName);
static gboolean grab_pointer_position(gint *x_press, gint *y_press, gint *x_rel, gint *y_rel);
static void changeCursor(Display *display, Window window);


static GdkPixbuf *pixbuf = NULL;
static char realName[MAXPATHLEN];

gboolean on_quit_window(GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit ();
	return FALSE;
}

void on_open_filedialog(GtkWidget *widget, gpointer data)
{
	GtkWidgetList *dataList = (GtkWidgetList *)data;

	gchar *selectFile;
	GtkWidget *fileDialog = gtk_file_chooser_dialog_new("Open File",
							GTK_WINDOW(dataList->gw1), GTK_FILE_CHOOSER_ACTION_OPEN, 
							"OK", GTK_RESPONSE_OK, 
							"Cancel", GTK_RESPONSE_CLOSE, NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileDialog), ".");

	gint result = gtk_dialog_run(GTK_DIALOG(fileDialog));
	switch (result) {
		case GTK_RESPONSE_OK:
			selectFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileDialog));
			if (selectFile != NULL) {
				memset(realName, '\0', sizeof(realName));
				strcpy(realName, selectFile);
				display(selectFile, dataList->gw2, dataList->hs, dataList->vs);
				g_free(selectFile);
				gtk_widget_destroy(fileDialog);
				fileDialog = NULL;
			}
			else {
				printError(GTK_WINDOW(fileDialog), "Open File Error");			
			}
			break;
		default:
			gtk_widget_destroy(fileDialog);
			fileDialog = NULL;
			break;
	}
}

void on_scale_changed(GtkWidget *widget, gpointer data)
{
	if (pixbuf != NULL) {
		GtkHScale *hs = ((GScale*)data)->hs;
		GtkVScale *vs = ((GScale*)data)->vs;
		GtkWidget *frame = ((GScale*)data)->frame;
		gdouble hsv = gtk_range_get_value((GtkRange*)hs);
		gdouble vsv = gtk_range_get_value((GtkRange*)vs);
		
		GdkPixbuf *newPixbuf = gdk_pixbuf_scale_simple(pixbuf, (int)hsv,(int)vsv, GDK_INTERP_BILINEAR);	
		
		GList* childList = gtk_container_get_children(GTK_CONTAINER(frame));
		while (childList != NULL) {
			gtk_container_remove(GTK_CONTAINER(frame), (GtkWidget*)(childList->data));
			childList = childList->next;
		}
		g_list_free(childList);
		
		showImage(newPixbuf, frame, NULL);
	}
}


void on_save(GtkWidget *widget, gpointer data)
{
	GtkWidget *frame = (GtkWidget *)data;
	saveImage(frame, realName, realName);
}

void on_save_as(GtkWidget *widget, gpointer data)
{
	GtkWidget *frame = (GtkWidget*)data;
	gchar *selectFile;
	GtkWidget *fileDialog = gtk_file_chooser_dialog_new("Save File As",
							NULL, GTK_FILE_CHOOSER_ACTION_SAVE, 
							"Save", GTK_RESPONSE_OK, 
							"Cancel", GTK_RESPONSE_CLOSE, NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileDialog), ".");

	gint result = gtk_dialog_run(GTK_DIALOG(fileDialog));
	switch (result) {
		case GTK_RESPONSE_OK:
			selectFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileDialog));
			if (selectFile != NULL) {
				if (0 == saveImage(frame, realName, selectFile))
					printError(GTK_WINDOW(fileDialog), "Save Success!");			
				else
					printError(GTK_WINDOW(fileDialog), "Save Failed!");			
				g_free(selectFile);
				gtk_widget_destroy(fileDialog);
				fileDialog = NULL;
			}
			break;
		default:
			gtk_widget_destroy(fileDialog);
			fileDialog = NULL;
			break;
	}
	
}

void on_transform(GtkWidget *widget, gpointer data)
{
	GtkWidget *frame = (GtkWidget*)data;	
	gchar *selectFile;
	GtkWidget *fileDialog = gtk_file_chooser_dialog_new("Save File",
							NULL, GTK_FILE_CHOOSER_ACTION_SAVE, 
							"Save", GTK_RESPONSE_OK, 
							"Cancel", GTK_RESPONSE_CLOSE, NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileDialog), ".");

	gint result = gtk_dialog_run(GTK_DIALOG(fileDialog));
	switch (result) {
		case GTK_RESPONSE_OK:
			selectFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileDialog));
			if (selectFile != NULL) {
				if (1 == transform(frame, selectFile)) {
					printf("暂时不支持这种格式");
				}
				g_free(selectFile);
				gtk_widget_destroy(fileDialog);
				fileDialog = NULL;
			}
			break;
		default:
			gtk_widget_destroy(fileDialog);
			fileDialog = NULL;
			break;
	}
}

void on_grab_whole_screen(GtkWidget *widget, gpointer data)
{
	GtkWidget *window = (GtkWidget*)data;
	gtk_window_iconify(GTK_WINDOW(window));
#if 0
	grab_whole_screen();
#else
	g_thread_create(grab_screen, NULL, FALSE, NULL);	
#endif
}
#if 1
void* grab_screen(void* arg)
{
	sleep(2);
	gdk_threads_enter();
	grab_whole_screen();
	gdk_threads_leave();
}
#endif

gint x_press, y_press, x_rel, y_rel, width, height;
void *grab_widget(void *arg)
{
	sleep(2);
	gdk_threads_enter();	
	grab_window(IMIN(x_press, x_rel), IMIN(y_press, y_rel), width, height);
	gdk_threads_leave();
}

void on_grabwindow(GtkWidget *widget, gpointer data)
{
	GtkWidget *window = (GtkWidget*)data;
	gtk_window_iconify(GTK_WINDOW(window));

	GtkWidget *dialog = gtk_dialog_new_with_buttons(
                        "The Window",
                        /*GTK_WINDOW(window)*/NULL,
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_YES,
                        GTK_RESPONSE_YES,
						GTK_STOCK_NO,
						GTK_RESPONSE_NO,
                        NULL);
	GList *list;
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch (result) {
		case GTK_RESPONSE_NO:
			break;
		default: 
			grab_pointer_position(&x_press, &y_press, &x_rel, &y_rel);
			width = IMAX(x_press, x_rel) - IMIN(x_press, x_rel);
			height = IMAX(y_press, y_rel) - IMIN(y_press, y_rel);
			if (width != 0 && height != 0)
#if 1
				g_thread_create(grab_widget, NULL, FALSE, NULL);	
#else
				grab_window(IMIN(x_press, x_rel), IMIN(y_press, y_rel), width, height);
#endif
#if 0 //改变光标形状
			oldCursor = gdk_window_get_cursor(gdk_get_default_root_window());
			GdkCursor *newCursor = gdk_cursor_new(GDK_TCROSS);
			gdk_window_set_cursor(gdk_get_default_root_window(), newCursor);
#endif
			break;
	}
}

static void display(const char *fileName, GtkWidget *frame, GtkHScale *hs, GtkVScale *vs)
{
	GError *err = NULL;
	pixbuf = gdk_pixbuf_new_from_file(fileName, &err);
	GList* childList = gtk_container_get_children(GTK_CONTAINER(frame));
	while (childList != NULL) {
		gtk_container_remove(GTK_CONTAINER(frame), (GtkWidget*)(childList->data));
		childList = childList->next;
	}
	g_list_free(childList);

	gtk_range_set_value((GtkRange*)hs, gdk_pixbuf_get_height(pixbuf));
	gtk_range_set_value((GtkRange*)vs, gdk_pixbuf_get_width(pixbuf));
	
	if (NULL == pixbuf) {
		printf("<<<<<<%s>>>>>>>\n", err->message);
		printError(GTK_WINDOW(frame), err->message);
		err = NULL;
		return;
	}
	showImage(pixbuf, frame, fileName);
}

static void printError(GtkWindow *widget, const char *info)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
						info,
						widget,
						GTK_DIALOG_MODAL,
						GTK_STOCK_OK,
						GTK_RESPONSE_ACCEPT,
						NULL);
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT)
		gtk_widget_destroy(dialog);
}

static void showImage(GdkPixbuf* pixbuf, GtkWidget *frame, const char *fileName)
{
	GtkWidget *image = NULL;
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_set_size_request(image, gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
	if (fileName != NULL)
	gtk_frame_set_label(GTK_FRAME(frame), strrchr(fileName, '/'));
	gtk_container_add(GTK_CONTAINER(frame), image);
	gtk_widget_show(image);
}

static int saveImage(GtkWidget *frame, const char *realName, const char *saveName)
{
	GError *err = NULL;
    GdkPixbufFormat *format = gdk_pixbuf_get_file_info(realName, NULL, NULL);
	GList *list = gtk_container_get_children(GTK_CONTAINER(frame));
	GdkPixbuf *newPixbuf = gtk_image_get_pixbuf(GTK_IMAGE(list->data));
	if (strcmp(realName, saveName) == 0)
		remove(realName);
    gboolean retVal = gdk_pixbuf_save(newPixbuf, saveName, gdk_pixbuf_format_get_name(format), &err, NULL);
   if (!retVal) {
   	    fprintf(stderr, "Failed to scale the image: \"%s\"\n", err->message);
       	err = NULL;
    	return -1;
	}
	
	return 0;
}


static int transform(GtkWidget *frame, char *saveName)
{
	GdkPixbufFormat *format = gdk_pixbuf_get_file_info(realName, NULL, NULL);
	if (format == NULL) {
		printf("format error\n");
		return -1;
	}
	char *name = gdk_pixbuf_format_get_name(format);
	char extendName[MAXPATHLEN];
	snprintf(extendName, sizeof(extendName), "%s%s", ".", name);
	char *saveExtendName = strrchr(saveName, '.');
	if (0 == strcmp(extendName, saveExtendName)) {
		return saveImage(frame, realName, saveName);
	}
	if (0 == strcmp(name, "bmp") && 0 == strcmp(saveExtendName, ".png"))
		return bmp2png(realName, saveName);
	else if (0 == strcmp(name, "png") && 0 == strcmp(saveExtendName, ".bmp"))
		return png2bmp(realName, saveName);
	else if (0 == strcmp(name, "bmp") && 0 == strncmp(saveExtendName,".jpeg", 2))
		return bmp2jpg(realName, saveName, 80);
	else if (0 == strcmp(name, "jpeg") && 0 == strcmp(saveExtendName, ".bmp"))
		return jpg2bmp(realName, saveName);
	else {
		gif2png(realName, saveName);
	}
		return 1;
	
}


static gboolean grab_pointer_position(gint *x_press, gint *y_press, gint *x_rel, gint *y_rel)
{
	Display *dpy = XOpenDisplay(NULL);
    Window win;
    int scr = DefaultScreen(dpy);
    int width = DisplayWidth(dpy, scr);
    int height = DisplayHeight(dpy, scr);
    int bg = XDefaultColormap(dpy,scr);
    XSetWindowAttributes attr;
    attr.event_mask = ButtonPress | ButtonRelease;

    win = XCreateWindow(dpy, RootWindow(dpy, scr), 0, 0, width, height, 0, 0, InputOnly, DefaultVisual(dpy, scr), CWEventMask, &attr);
    /*Remove the TitleBar*/
    MWMHints mwmhints;
    Atom prop;
    memset(&mwmhints, 0, sizeof(mwmhints));
    prop = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
    mwmhints.flags = MWM_HINTS_DECORATIONS;
    mwmhints.decorations = 0;
    XChangeProperty(dpy, win, prop, prop, 32, PropModeReplace, (unsigned char *) &mwmhints, PROP_MWM_HINTS_ELEMENTS);

#if 1  //实现在linux桌面任意工作区可见
	Atom net_wm_state_sticky=XInternAtom(dpy, "_NET_WM_STATE_STICKY", True);
	Atom net_wm_state = XInternAtom (dpy, "_NET_WM_STATE", False);
  	XChangeProperty (dpy, win, net_wm_state, XA_ATOM, 32, PropModeAppend, (unsigned char *)&net_wm_state_sticky, 1);
#endif
#if 1 // 窗口始终置顶
	Atom net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	Atom net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	XChangeProperty (dpy, win, net_wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&net_wm_window_type_dock, 1);  
#endif
    changeCursor(dpy, win);
    XMapWindow(dpy, win);
    XSelectInput(dpy, win, ButtonPressMask | ButtonReleaseMask);

    int i = 0;
    while (i != 2) {
        XEvent event;
        XNextEvent(dpy, &event);
        if (event.type == ButtonPress && event.xbutton.button == 1) {
            *x_press = event.xbutton.x_root;
			*y_press = event.xbutton.y_root;
            ++i;
        }
        if (event.type == ButtonRelease && event.xbutton.button == 1) {
            *x_rel = event.xbutton.x_root;
			*y_rel = event.xbutton.y_root;
            ++i;
        }
    }
    XCloseDisplay(dpy);
}


static void changeCursor(Display *display, Window window)
{
	Colormap cmap;
    Cursor no_ptr;
    XColor black, dummy, white;
//  static char bm_no_data[] = {0, 0, 0, 0, 0, 0, 0, 0};//hide the mouse.

    cmap = DefaultColormap(display, DefaultScreen(display));
    XAllocNamedColor(display, cmap, "black", &black, &dummy);
    XAllocNamedColor(display, cmap, "white", &white, &dummy);
    Pixmap bm_crosshair = XCreateBitmapFromData(display, window, crosshair_bits, crosshair_width, crosshair_height);
    Pixmap bm_crosshair_mask = XCreateBitmapFromData(display, window, crosshair_mask_bits, crosshair_width, crosshair_height);
    no_ptr = XCreatePixmapCursor(display, bm_crosshair, bm_crosshair_mask, &white, &black, 0, 0);

    XDefineCursor(display, window, no_ptr);
    XFreeCursor(display, no_ptr);
    if (bm_crosshair != None)
    	XFreePixmap(display, bm_crosshair);
	if (bm_crosshair_mask != None)
        XFreePixmap(display, bm_crosshair_mask);
    
	XFreeColors(display, cmap, &black.pixel, 1, 0);
	XFreeColors(display, cmap, &white.pixel, 1, 0);
}

