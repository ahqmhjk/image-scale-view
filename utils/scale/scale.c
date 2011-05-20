#include "scale.h"

#define GETSCALEOPT(opt, type) do{if (opt == FastScaled)                    \
							   		type =  GDK_INTERP_NEAREST;	    		\
								  else if (opt == AntialiasingScaled)		\
							    	type = GDK_INTERP_TILES;   				\
								  else if (opt == BestScaled)				\
									type = GDK_INTERP_HYPER;				\
								  else										\
									type = GDK_INTERP_BILINEAR;				\
								  type;										\
				    			}while(0)

int scale(const char *realName, const char *saveName, 
				 gRect r, ScaleOption opt)
{
	GError *err = NULL;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(realName, &err);
	if (NULL == pixbuf) {
		fprintf(stderr, "Failed to open the file: \"%s\"\n", err->message);
		err = NULL;
		return -1;
	}
	
	int dest_width, dest_height;
	if (r.width == 0) dest_width = gdk_pixbuf_get_width(pixbuf);
	else if (r.width < 0) dest_width = abs(r.width);
	else dest_width = r.width;
	if (r.height == 0) dest_height = gdk_pixbuf_get_height(pixbuf);
	else if (r.height < 0) dest_height = abs(r.height); 
	else dest_height = r.height;
	
	
	GdkInterpType type;
	GETSCALEOPT(opt, type);
	GdkPixbuf *newPixbuf = gdk_pixbuf_scale_simple(pixbuf, dest_width, 
												   dest_height, type);
	if (NULL == newPixbuf) {
		fprintf(stderr, "Failed to scale the image: \"%s\"\n", err->message);
		err = NULL;
		return -1;
	}

	GdkPixbufFormat *format = gdk_pixbuf_get_file_info(realName, NULL, NULL);
	gboolean retVal = gdk_pixbuf_save(newPixbuf, saveName ? saveName : realName, 				   					  gdk_pixbuf_format_get_name(format), 
									  &err, NULL);
	if (!retVal) {
		fprintf(stderr, "Failed to scale the image: \"%s\"\n", err->message);
        err = NULL;
        return -1;
	}

	return 0;
}
