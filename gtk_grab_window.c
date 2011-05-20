#ifndef _GRAB_WINDOW_H
#define _GRAB_WINDOW_H

//#define HAVE_X11_EXTENSIONS_SHAPE_H


#include <stdio.h>
#include <string.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
#include <X11/extensions/shape.h>
#endif
typedef signed long int    INT32;




int main(int argc, char *argv[])
{

gtk_init(&argc, &argv);

GdkWindow *window = gdk_window_foreign_new(GDK_ROOT_WINDOW());
GdkWindow *root = gdk_window_foreign_new(GDK_ROOT_WINDOW());

  GdkPixbuf *screenshot;
  gint x_real_orig, y_real_orig;
  gint x_orig, y_orig;
  gint real_width, real_height;
  gint width, height;
GError *error = NULL;

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
  printf("HAVE X11\n");
  XRectangle *rectangles;
  GdkPixbuf *tmp;
  int rectangle_count, rectangle_order, i;
#endif

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

 #ifdef HAVE_X11_EXTENSIONS_SHAPE_H
  tmp = gdk_pixbuf_get_from_drawable (NULL, root, NULL,
				      x_orig, y_orig, 0, 0,
				      width, height);

  rectangles = XShapeGetRectangles (GDK_DISPLAY (), GDK_WINDOW_XWINDOW (window),
				    ShapeBounding, &rectangle_count, &rectangle_order);
  if (rectangle_count > 0)
    {
	printf("%i\n", rectangle_count);
      gboolean has_alpha = gdk_pixbuf_get_has_alpha (tmp);

      screenshot = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
				   width, height);
      gdk_pixbuf_fill (screenshot, 0);
	
      for (i = 0; i < rectangle_count; i++)
	{
	  gint rec_x, rec_y;
	  gint rec_width, rec_height;
          gint y;

	  rec_x = rectangles[i].x;
	  rec_y = rectangles[i].y;
	  rec_width = rectangles[i].width;
	  rec_height = rectangles[i].height;

	  if (x_real_orig < 0)
	    {
	      rec_x += x_real_orig;
	      rec_x = MAX(rec_x, 0);
	      rec_width += x_real_orig;
	    }
	  if (y_real_orig < 0)
	    {
	      rec_y += y_real_orig;
	      rec_y = MAX(rec_y, 0);
	      rec_height += y_real_orig;
	    }

	  if (x_orig + rec_x + rec_width > gdk_screen_width ())
	    rec_width = gdk_screen_width () - x_orig - rec_x;
	  if (y_orig + rec_y + rec_height > gdk_screen_height ())
	    rec_height = gdk_screen_height () - y_orig - rec_y;

	  for (y = rec_y; y < rec_y + rec_height; y++)
	    {
              guchar *src_pixels, *dest_pixels;
              gint x;
	      
	      src_pixels = gdk_pixbuf_get_pixels (tmp) +
		y * gdk_pixbuf_get_rowstride(tmp) +
		rec_x * (has_alpha ? 4 : 3);
	      dest_pixels = gdk_pixbuf_get_pixels (screenshot) +
		y * gdk_pixbuf_get_rowstride (screenshot) +
		rec_x * 4;
				
	      for (x = 0; x < rec_width; x++)
		{
		  *dest_pixels++ = *src_pixels ++;
		  *dest_pixels++ = *src_pixels ++;
		  *dest_pixels++ = *src_pixels ++;
		  if (has_alpha)
		    *dest_pixels++ = *src_pixels++;
		  else
		    *dest_pixels++ = 255;
		}
	    }
	}
      g_object_unref (tmp);
    }
  else
    {
      screenshot = tmp;
    }
#else /* HAVE_X11_EXTENSIONS_SHAPE_H */
  screenshot = gdk_pixbuf_get_from_drawable (NULL, root, NULL, /*x_orig, y_orig,*/ 100,100 ,0, 0,
					     width/2, height/2);
#endif /* HAVE_X11_EXTENSIONS_SHAPE_H */


gdk_pixbuf_save (screenshot, "/home/hjk/12.png","png", &error, "tEXt::CREATOR", "gnome-panel-screenshot", NULL);

}




#undef HAVE_X11_EXTENSIONS_SHAPE_H
#endif
