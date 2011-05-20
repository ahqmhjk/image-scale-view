/*
  gif2png:

  Copyright (C) 1995 Alexander Lehmann

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications (see below), and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  4. Binary only distributions of the software must include the file README
     with the copyright statement. You are welcome to add a copyright
     statement for your modifications and a contact address, though.

  Note that this program uses the LZW decompression algorithm, which due to
  patent claims probably requires you to license if you use the algorithm
  in a commercial program or distribute this program on a for-profit basis.
  (See http://www.unisys.com)


  Alexander Lehmann <alex@hal.rhein-main.de>

 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get png type definitions */
#include "png.h"

#define GIFterminator ';'
#define GIFextension '!'
#define GIFimage ','

#define GIFcomment 0xfe
#define GIFapplication 0xff
#define GIFplaintext 0x01
#define GIFgraphicctl 0xf9

#define MAXCMSIZE 256

typedef unsigned char byte;

typedef png_color GifColor;

struct GIFimagestruct {
  GifColor colors[MAXCMSIZE];
  unsigned long color_count[MAXCMSIZE];
  int offset_x;
  int offset_y;
  int width;
  int height;
  int trans;
  int interlace;
};

struct GIFelement {
  struct GIFelement *next;
  char GIFtype;
#ifndef TMPFILE
  byte *data;
  long allocated_size;
#else
  unsigned long file_offset;
#endif
  long size;
  /* only used if GIFtype==GIFimage */
  struct GIFimagestruct *imagestruct;
};

extern struct gif_scr{
  unsigned int  Width;
  unsigned int  Height;
  GifColor      ColorMap[MAXCMSIZE];
  unsigned int  ColorMap_present;
  unsigned int  BitPixel;
  unsigned int  ColorResolution;
  int           Background;
  unsigned int  AspectRatio;
} GifScreen;

int ReadGIF(FILE *fd);
int gif2png(char *in, char *out);

void allocate_element(void);
void store_block(char *data, int size);
void allocate_image(void);
void set_size(long);

void *xalloc(unsigned long s);
void *xrealloc(void *p, unsigned long s);
void fix_current(void);
byte *access_data(struct GIFelement *e, unsigned long pos, unsigned long len);
void free_mem(void);

int interlace_line(int height, int line);
int inv_interlace_line(int height, int line);

extern struct GIFelement first;
extern struct GIFelement *current;
extern int recover;

#ifdef TMPFILE
extern FILE *tempfile;
#endif

extern const char version[];
extern const char compile_info[];

extern int skip_pte;

#ifdef __cplusplus
}
#endif /* __cplusplus */

