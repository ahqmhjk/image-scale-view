#ifndef _GTKFM_H
#define _GTKFM_H

#include "p_global.h"


typedef struct _g_rect_ {
    int width;
    int height;
}gRect;

typedef enum {
    FastScaled,
    AntialiasingScaled,
    NormalScaled,
    BestScaled
}ScaleOption;

//return 0 if success or return -1;
int scale(const char *realName, const char *saveName,
                 gRect r, ScaleOption opt);


#endif //_GTKFM_H
