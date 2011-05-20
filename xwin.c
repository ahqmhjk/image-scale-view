#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>

#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MWM_HINTS_ELEMENTS 5

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

int main()
{
	printf("----------------\n");
	Display *dpy = XOpenDisplay(NULL);
	Window win;
	int scr = DefaultScreen(dpy);
	int width = DisplayWidth(dpy, scr);
	int height = DisplayHeight(dpy, scr);
	int bg = XDefaultColormap(dpy,scr);
	XSetWindowAttributes attr;
	attr.event_mask = ButtonPress | ButtonRelease;
//	attr.override_redirect = 1;
	win = XCreateWindow(dpy, RootWindow(dpy, scr), 0, 0, width, height, 0, 0, InputOnly, DefaultVisual(dpy, scr), CWEventMask, &attr);
	MWMHints mwmhints;
	printf("%d, %d\n", width, height);
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
	XMapWindow(dpy, win);
	XSelectInput(dpy, win, ButtonPressMask | ButtonReleaseMask | ExposureMask);
	static int i = 0;
	
	while (i != 2) {
		XEvent event;
		XNextEvent(dpy, &event);
		if (event.type == ButtonPress) {
			printf("state = %i\n", event.xbutton.state);
			printf("x = %i, y = %i, x_root = %i, y_root = %i\n", event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root);
			if (event.xbutton.button == 1)
			++i;
		}
		if (event.type == ButtonRelease) {
           printf("x = %i, y = %i, x_root = %i, y_root = %i\n", event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root);
			++i;
		}
		
		if (event.type == Expose) {   
		}
	}
	XCloseDisplay(dpy);
	return 0;
}
