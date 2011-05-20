#ifndef _CAPTURE_H
#define _CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <p_global.h>

int grab_whole_screen();

int grab_window(int src_x, int src_y, int width, int height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
