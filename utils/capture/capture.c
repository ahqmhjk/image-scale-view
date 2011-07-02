#include "capture.h"
#include <sys/param.h>
#include <time.h>

int grab_whole_screen()
{	
	GdkWindow *window, *root;
#ifndef GTK3
	window = gdk_window_foreign_new(GDK_ROOT_WINDOW());
	root = gdk_window_foreign_new(GDK_ROOT_WINDOW());
#else
	GdkDisplay *gdpy = gdk_display_get_default();
	window = root = gdk_x11_window_foreign_new_for_display(gdpy, GDK_ROOT_WINDOW());
#endif

  GdkPixbuf *screenshot;
  gint x_real_orig, y_real_orig;
  gint x_orig, y_orig;
  gint real_width, real_height;
  gint width, height;
	GError *error = NULL;
#ifndef GTK3
	gdk_drawable_get_size(window, &real_width, &real_height);
#else
	real_width = gdk_window_get_width(window);
	real_height = gdk_window_get_height(window); 
#endif
	gdk_window_get_origin(window, &x_real_orig, &y_real_orig);

  x_orig = x_real_orig;
  y_orig = y_real_orig;
  width = real_width;
  height = real_height;
 	if (x_orig < 0) {
  	width = width + x_orig;
    x_orig = 0;
  }
  if (y_orig < 0) {
    height = height + y_orig;
    y_orig = 0;
  }

  if (x_orig + width > gdk_screen_width ())
  	width = gdk_screen_width () - x_orig;
  if (y_orig + height > gdk_screen_height ())
   	height = gdk_screen_height () - y_orig;
	char path[MAXPATHLEN];
	time_t now;
	time(&now);
	snprintf(path, sizeof(path), "%s%i%s", "./screen-shot-whole-screen-", now, ".png");

#ifndef GTK3
	screenshot = gdk_pixbuf_get_from_drawable (NULL, root, NULL, x_orig, y_orig, 0, 0, width, height);
#else
	screenshot = gdk_pixbuf_get_from_window(root, x_orig, y_orig, width, height);
#endif

	gdk_pixbuf_save (screenshot, path, "png", &error, "tEXt::CREATOR", "gnome-panel-screenshot", NULL);
	return 0;
}

int grab_window(int src_x, int src_y, int width, int height)
{
	GdkWindow *window, *root;
#ifndef GTK3
	window = gdk_window_foreign_new(GDK_ROOT_WINDOW());
	root = gdk_window_foreign_new(GDK_ROOT_WINDOW());
#else
	GdkDisplay *gdpy = gdk_display_get_default();
	window = root = gdk_x11_window_foreign_new_for_display(gdpy, GDK_ROOT_WINDOW());
#endif

  GdkPixbuf *screenshot;
	GError *error = NULL;

  if (src_x + width > gdk_screen_width ())
  	width = gdk_screen_width () - src_x;
  if (src_y + height > gdk_screen_height ())
   	height = gdk_screen_height () - src_y;

	time_t now;
	time(&now);
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "%s%i%s", "./screen-shot-window", now, ".png");
	printf("%s\n", path);
#ifndef GTK3
  screenshot = gdk_pixbuf_get_from_drawable (NULL, root, NULL, src_x, src_y, 0, 0, width, height);
#else
	screenshot = gdk_pixbuf_get_from_window(root, src_x, src_y, width, height);
#endif

	printf("----------------%d\n", gdk_pixbuf_get_rowstride(screenshot));
	gdk_pixbuf_save (screenshot, path, "png", &error, "tEXt::CREATOR", "gnome-panel-screenshot", NULL);

	return 0;
}
