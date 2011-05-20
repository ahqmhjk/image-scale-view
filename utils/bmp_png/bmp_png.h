#ifndef _BMP_PNG_H
#define _BMP_PNG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "p_global.h"

int bmp2png(char *in, char *out);

int png2bmp(char *in, char *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
