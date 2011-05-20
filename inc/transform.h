#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int jpg2bmp(const char *in, const char *out);
extern size_t bmp2jpg(char *in,char *out, int quality);
extern int gif2png(char *in, char *out);
extern int bmp2png(char *in, char *out);
extern int png2bmp(char *in, char *out);

extern int grab_whole_screen();
extern int grab_window(int src_x, int src_y, int width, int height);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
