#include "capture.h"
#include <sys/param.h>
#include <time.h>

int grab_whole_screen()
{	
	GdkWindow *window = gdk_window_foreign_new(GDK_ROOT_WINDOW());
	GdkWindow *root = gdk_window_foreign_new(GDK_ROOT_WINDOW());

  	GdkPixbuf *screenshot;
  	gint x_real_orig, y_real_orig;
  	gint x_orig, y_orig;
  	gint real_width, real_height;
  	gint width, height;
	GError *error = NULL;
	gdk_drawable_get_size(window, &real_width, &real_height);
	gdk_window_get_origin(window, &x_real_orig, &y_real_orig);

  	x_orig = x_real_orig;
  	y_orig = y_real_orig;
  	width = real_width;
  	height = real_height;
 	if (x_orig < 0)
    {
   		width = width + x_orig;
      	x_orig = 0;
    }
  	if (y_orig < 0)
    {
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
  	screenshot = gdk_pixbuf_get_from_drawable (NULL, root, NULL, x_orig, y_orig, 0, 0, width, height);
	gdk_pixbuf_save (screenshot, path, "png", &error, "tEXt::CREATOR", "gnome-panel-screenshot", NULL);
	return 0;
}

int grab_window(int src_x, int src_y, int width, int height)
{
	GdkWindow *window = gdk_window_foreign_new(GDK_ROOT_WINDOW());
	GdkWindow *root = gdk_window_foreign_new(GDK_ROOT_WINDOW());

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
  	screenshot = gdk_pixbuf_get_from_drawable (NULL, root, NULL, src_x, src_y, 0, 0, width, height);
	printf("----------------%d\n", gdk_pixbuf_get_rowstride(screenshot));
	gdk_pixbuf_save (screenshot, path, "png", &error, "tEXt::CREATOR", "gnome-panel-screenshot", NULL);

	return 0;
}
