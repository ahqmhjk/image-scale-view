/*
 * gif2png.c
 * Copyright (C) 1995 Alexander Lehmann
 * For conditions of distribution and use, see copyright notice in gif2png.h
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define PNG_INTERNAL
#include "gif2png.h"

#ifndef PNG_USER_MEM_SUPPORTED
#define PNG_USER_MEM_SUPPORTED
#else
#undef PNG_USER_MEM_SUPPORTED
#endif

typedef png_size_t png_alloc_size_t;
static png_byte png_tEXtP[5] = {116,  69,  88, 116, '\0'};
struct GIFelement first={NULL};
struct GIFelement *current;
int delete;
int histogram;
int interlaced;
int progress;
int recover;
int software_chunk;
int skip_pte;

typedef struct
{
   char *input;   /* The uncompressed input data */
   int input_len;   /* Its length */
   int num_output_ptr; /* Number of output pointers used */
   int max_output_ptr; /* Size of output_ptr */
   png_charpp output_ptr; /* Array of pointers to output */
} compression_state;

static png_voidp png_create_structP(int type)
{
#ifdef PNG_USER_MEM_SUPPORTED
   return (png_create_struct_2P(type, NULL, NULL));
}

/* Alternate version of png_create_structP, for use with user-defined malloc. */
static png_voidp png_create_struct_2P(int type, png_malloc_ptr malloc_fn, png_voidp mem_ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */
   png_size_t size;
   png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
      size = png_sizeof(png_info);
   else if (type == PNG_STRUCT_PNG)
      size = png_sizeof(png_struct);
   else
      return (png_get_copyright(NULL));

#ifdef PNG_USER_MEM_SUPPORTED
   if (malloc_fn != NULL)
   {
      png_struct dummy_struct;
      png_structp png_ptr = &dummy_struct;
      png_ptr->mem_ptr=mem_ptr;
      struct_ptr = (*(malloc_fn))(png_ptr, (png_uint_32)size);
   }
   else
#endif /* PNG_USER_MEM_SUPPORTED */
   struct_ptr = (png_voidp)malloc(size);
   if (struct_ptr != NULL)
      png_memset(struct_ptr, 0, size);
   return (struct_ptr);
}

static void png_destroy_structP(png_voidp struct_ptr)
{
#ifdef PNG_USER_MEM_SUPPORTED
   png_destroy_struct_2P(struct_ptr, NULL, NULL);

}

/* Free memory allocated by a png_create_structP() call */
static void png_destroy_struct_2P(png_voidp struct_ptr, png_free_ptr free_fn,
    png_voidp mem_ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */
   if (struct_ptr != NULL)
   {
#ifdef PNG_USER_MEM_SUPPORTED
      if (free_fn != NULL)
      {
         png_struct dummy_struct;
         png_structp png_ptr = &dummy_struct;
         png_ptr->mem_ptr=mem_ptr;
         (*(free_fn))(png_ptr, struct_ptr);
         return;
      }
#endif /* PNG_USER_MEM_SUPPORTED */

#if defined(__TURBOC__) && !defined(__FLAT__)
      farfree(struct_ptr);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
      hfree(struct_ptr);
# else
      free(struct_ptr);
# endif
#endif

   }
}
static void png_write_init_3_P(png_structpp ptr_ptr, png_const_charp user_png_ver,png_size_t png_struct_size);
static png_size_t png_check_keywordP(png_structp png_ptr, png_charp key, png_charpp new_key);
static void png_write_compressed_data_out(png_structp png_ptr, compression_state *comp);
static int png_text_compress(png_structp png_ptr,
        png_charp text, png_size_t text_len, int compression,
        compression_state *comp);
static void png_write_tEXtP(png_structp png_ptr, png_charp key, png_charp text, png_size_t text_len);
static void png_write_zTXtP(png_structp png_ptr, png_charp key, png_charp text, png_size_t text_len, int compression);
static int writefile(struct GIFelement *s, struct GIFelement *e, FILE *fp, int lastimg);

/* return the actual line # used to store an interlaced line */
int interlace_line(int height, int line)
{
  int res;

  if((line&7)==0) {
    return line>>3;
  }
  res=(height+7)>>3;
  if((line&7)==4) {
    return res+((line-4)>>3);
  }
  res+=(height+3)>>3;
  if((line&3)==2) {
    return res+((line-2)>>2);
  }
  return res+((height+1)>>2)+((line-1)>>1);
}

/* inverse function of above, used for recovery of interlaced images */

int inv_interlace_line(int height, int line)
{
  if((line<<3)<height) {
    return line<<3;
  }
  line-=(height+7)>>3;
  if((line<<3)+4<height) {
    return (line<<3)+4;
  }
  line-=(height+3)>>3;
  if((line<<2)+2<height) {
    return (line<<2)+2;
  }
  line-=(height+1)>>2;
  return (line<<1)+1;
}

/* do some adjustments of the color palette. We won't check for duplicate
   colors, this is probably only a very rare case, but we check if all used
   colors are grays, some GIF writers (e.g. some versions to ppmtogif) leave
   the unused entries uninitialized, which breaks the detection of grayscale
   images (e.g. in QPV). If the image is grayscale we will use color type 0,
   except when the transparency color is also appearing as a visible color. In
   this case we write a paletted image, another solution would be gray+alpha,
   but that would probably increase the image size too much.
   If there are only few gray levels (<=16), we try to create a 4 or less bit
   grayscale file, but if the gray levels do not fit into the necessary grid,
   we write a paletted image, e.g. if the image contains black, white and 50%
   gray, the grayscale image would require 8 bit, but the paletted image only
   2 bit. Even with filtering, the 8 bit file will be larger.
*/

static int graycheck(png_color c)
{
  /* check if r==g==b, moved to function to work around NeXTstep 3.3 cc bug.
   * life could be so easy if at least the basic tools worked as expected.
   */
  return c.red==c.green && c.green==c.blue;
}



int gif2png(char *inFile, char *outFile)
{
  char outname[1025]; /* MAXPATHLEN comes under too many names */
  int num_pics;
  struct GIFelement *start;
  int i;
  char *file_ext;
  FILE *fp = fopen(inFile, "rb");
  if(fp==NULL) return 1;

#ifdef TMPFILE
  fseek(tempfile, 0, SEEK_SET);
#endif

  current=&first;

  num_pics=ReadGIF(fp);

  fclose(fp);

  if(num_pics>=0)
    printf("number of images %d\n", num_pics);

  if(num_pics<=0) return 1;

  /* create output filename */

  strcpy(outname, outFile);

  file_ext=outname+strlen(outname)-4;
  if(strcmp(file_ext, ".gif")!=0 && strcmp(file_ext, ".GIF")!=0 &&
     strcmp(file_ext, "_gif")!=0 && strcmp(file_ext, "_GIF")!=0) {
    /* try to derive basename */
    file_ext=outname+strlen(outname);
    while(file_ext>=outname) {
      if(*file_ext=='.' || *file_ext=='/' || *file_ext=='\\') break;
      file_ext--;
    }
    if(file_ext<outname || *file_ext!='.') {
      /* as a last resort, just add .png to the filename */
      file_ext=outname+strlen(outname);
    }
  }

  strcpy(file_ext, ".png"); /* images are named .png, .p01, .p02, ... */

  start=NULL;

  i=0;

  for(current=first.next; current ; current=current->next) {
    if(start==NULL) start=current;
    if(current->GIFtype==GIFimage) {
      i++;
      if((fp=fopen(outname, "wb"))==NULL) {
        perror(outname);
        return 1;
      } else {
        writefile(start, current, fp, i==num_pics);
        fclose(fp);
        start=NULL;
        sprintf(file_ext, "P%02d.png", i);
      }
    }
  }

  free_mem();

  return 0;
}

/* check if stdin is a terminal, if your compiler is lacking isatty or fileno
   (non-ANSI functions), just return 0
*/

int input_is_terminal(void)
{
  return isatty(fileno(stdin));
}

static void png_write_tEXtP(png_structp png_ptr, png_charp key, png_charp text, png_size_t text_len)
{
   png_size_t key_len;
   png_charp new_key;

//   png_debug(1, "in png_write_tEXt\n");
   if (key == NULL || (key_len = png_check_keywordP(png_ptr, key, &new_key))==0)
   {
      png_warning(png_ptr, "Empty keyword in tEXt chunk");
      return;
   }

   if (text == NULL || *text == '\0')
      text_len = 0;

   /* make sure we include the 0 after the key */
   png_write_chunk_start(png_ptr, png_tEXtP, (png_uint_32)key_len+text_len+1);
   /*
    * We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE.  ISO PNG will forbid them.
    */
   png_write_chunk_data(png_ptr, (png_bytep)new_key, key_len + 1);
   if (text_len)
      png_write_chunk_data(png_ptr, (png_bytep)text, text_len);

   png_write_chunk_end(png_ptr);
   png_free(png_ptr, new_key);
}

static void png_write_zTXtP(png_structp png_ptr, png_charp key, png_charp text, png_size_t text_len, int compression)
{
   PNG_zTXt;
   png_size_t key_len;
   char buf[1];
   png_charp new_key;
   compression_state comp;

   png_debug(1, "in png_write_zTXt");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;
   comp.input_len = 0;

   if ((key_len = png_check_keywordP(png_ptr, key, &new_key))==0)
   {
      png_free(png_ptr, new_key);
      return;
   }

   if (text == NULL || *text == '\0' || compression==PNG_TEXT_COMPRESSION_NONE)
   {
      png_write_tEXtP(png_ptr, new_key, text, (png_size_t)0);
      png_free(png_ptr, new_key);
      return;
   }

   text_len = png_strlen(text);

   /* Compute the compressed data; do it now for the length */
   text_len = png_text_compress(png_ptr, text, text_len, compression,
       &comp);

   /* Write start of chunk */
   png_write_chunk_start(png_ptr, (png_bytep)png_zTXt,
     (png_uint_32)(key_len+text_len + 2));
   /* Write key */
   png_write_chunk_data(png_ptr, (png_bytep)new_key,
     (png_size_t)(key_len + 1));
   png_free(png_ptr, new_key);

   buf[0] = (png_byte)compression;
   /* Write compression */
   png_write_chunk_data(png_ptr, (png_bytep)buf, (png_size_t)1);
   /* Write the compressed data */
   png_write_compressed_data_out(png_ptr, &comp);

   /* Close the chunk */
   png_write_chunk_end(png_ptr);
}

static int png_text_compress(png_structp png_ptr,
        png_charp text, png_size_t text_len, int compression,
        compression_state *comp)
{
   int ret;

   comp->num_output_ptr = 0;
   comp->max_output_ptr = 0;
   comp->output_ptr = NULL;
   comp->input = NULL;
   comp->input_len = 0;

   /* We may just want to pass the text right through */
   if (compression == PNG_TEXT_COMPRESSION_NONE)
   {
       comp->input = text;
       comp->input_len = text_len;
       return((int)text_len);
   }

   if (compression >= PNG_TEXT_COMPRESSION_LAST)
   {
#ifdef PNG_STDIO_SUPPORTED
      char msg[50];
      png_snprintf(msg, 50, "Unknown compression type %d", compression);
      png_warning(png_ptr, msg);
#else
      png_warning(png_ptr, "Unknown compression type");
#endif
   }

   /* We can't write the chunk until we find out how much data we have,
    * which means we need to run the compressor first and save the
    * output.  This shouldn't be a problem, as the vast majority of
    * comments should be reasonable, but we will set up an array of
    * malloc'd pointers to be sure.
    *
    * If we knew the application was well behaved, we could simplify this
    * greatly by assuming we can always malloc an output buffer large
    * enough to hold the compressed text ((1001 * text_len / 1000) + 12)
    * and malloc this directly.  The only time this would be a bad idea is
    * if we can't malloc more than 64K and we have 64K of random input
    * data, or if the input string is incredibly large (although this
    * wouldn't cause a failure, just a slowdown due to swapping).
    */

   /* Set up the compression buffers */
   /* TODO: the following cast hides a potential overflow problem. */
   png_ptr->zstream.avail_in = (uInt)text_len;
   /* NOTE: assume zlib doesn't overwrite the input */
   png_ptr->zstream.next_in = (Bytef *)text;
   png_ptr->zstream.avail_out = png_ptr->zbuf_size;
   png_ptr->zstream.next_out = png_ptr->zbuf;

   /* This is the same compression loop as in png_write_row() */
   do
   {
      /* Compress the data */
      ret = deflate(&png_ptr->zstream, Z_NO_FLUSH);
      if (ret != Z_OK)
      {
         /* Error */
         if (png_ptr->zstream.msg != NULL)
            png_error(png_ptr, png_ptr->zstream.msg);
         else
            png_error(png_ptr, "zlib error");
      }
      /* Check to see if we need more room */
      if (!(png_ptr->zstream.avail_out))
      {
         /* Make sure the output array has room */
         if (comp->num_output_ptr >= comp->max_output_ptr)
         {
            int old_max;

            old_max = comp->max_output_ptr;
            comp->max_output_ptr = comp->num_output_ptr + 4;
            if (comp->output_ptr != NULL)
            {
               png_charpp old_ptr;

               old_ptr = comp->output_ptr;
               comp->output_ptr = (png_charpp)png_malloc(png_ptr,
                  (png_alloc_size_t)
                  (comp->max_output_ptr * png_sizeof(png_charpp)));
               png_memcpy(comp->output_ptr, old_ptr, old_max
                  * png_sizeof(png_charp));
               png_free(png_ptr, old_ptr);
            }
            else
               comp->output_ptr = (png_charpp)png_malloc(png_ptr,
                  (png_alloc_size_t)
                  (comp->max_output_ptr * png_sizeof(png_charp)));
         }

         /* Save the data */
         comp->output_ptr[comp->num_output_ptr] =
            (png_charp)png_malloc(png_ptr,
            (png_alloc_size_t)png_ptr->zbuf_size);
         png_memcpy(comp->output_ptr[comp->num_output_ptr], png_ptr->zbuf,
            png_ptr->zbuf_size);
         comp->num_output_ptr++;

         /* and reset the buffer */
         png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
         png_ptr->zstream.next_out = png_ptr->zbuf;
      }
   /* Continue until we don't have any more to compress */
   } while (png_ptr->zstream.avail_in);

   /* Finish the compression */
   do
   {
      /* Tell zlib we are finished */
      ret = deflate(&png_ptr->zstream, Z_FINISH);

      if (ret == Z_OK)
      {
         /* Check to see if we need more room */
         if (!(png_ptr->zstream.avail_out))
         {
            /* Check to make sure our output array has room */
            if (comp->num_output_ptr >= comp->max_output_ptr)
            {
               int old_max;

               old_max = comp->max_output_ptr;
               comp->max_output_ptr = comp->num_output_ptr + 4;
               if (comp->output_ptr != NULL)
               {
                  png_charpp old_ptr;

                  old_ptr = comp->output_ptr;
                  /* This could be optimized to realloc() */
                  comp->output_ptr = (png_charpp)png_malloc(png_ptr,
                     (png_alloc_size_t)(comp->max_output_ptr *
                     png_sizeof(png_charp)));
                  png_memcpy(comp->output_ptr, old_ptr,
                     old_max * png_sizeof(png_charp));
                  png_free(png_ptr, old_ptr);
               }
               else
                  comp->output_ptr = (png_charpp)png_malloc(png_ptr,
                     (png_alloc_size_t)(comp->max_output_ptr *
                     png_sizeof(png_charp)));
            }

            /* Save the data */
            comp->output_ptr[comp->num_output_ptr] =
               (png_charp)png_malloc(png_ptr,
               (png_alloc_size_t)png_ptr->zbuf_size);
            png_memcpy(comp->output_ptr[comp->num_output_ptr], png_ptr->zbuf,
               png_ptr->zbuf_size);
            comp->num_output_ptr++;

            /* and reset the buffer pointers */
            png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
            png_ptr->zstream.next_out = png_ptr->zbuf;
         }
      }
      else if (ret != Z_STREAM_END)
      {
         /* We got an error */
         if (png_ptr->zstream.msg != NULL)
            png_error(png_ptr, png_ptr->zstream.msg);
         else
            png_error(png_ptr, "zlib error");
      }
   } while (ret != Z_STREAM_END);

   /* Text length is number of buffers plus last buffer */
   text_len = png_ptr->zbuf_size * comp->num_output_ptr;
   if (png_ptr->zstream.avail_out < png_ptr->zbuf_size)
      text_len += png_ptr->zbuf_size - (png_size_t)png_ptr->zstream.avail_out;

   return((int)text_len);
}


static void png_write_init_3_P(png_structpp ptr_ptr, png_const_charp user_png_ver,png_size_t png_struct_size)
{
png_structp png_ptr=*ptr_ptr;
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp; /* to save current jump buffer */
#endif
  int i = 0;
  do
   {
     if (user_png_ver[i] != png_libpng_ver[i])
     {
 #ifdef PNG_LEGACY_SUPPORTED
       png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;
 #else
     png_ptr->warning_fn=(png_error_ptr)NULL;
      png_warning(png_ptr,
     "Application uses deprecated png_write_init() and should be recompiled.");
       break;
 #endif
     }
    } while (png_libpng_ver[i++]);
 
   png_debug(1, "in png_write_init_3\n");
 
#ifdef PNG_SETJMP_SUPPORTED
   /* save jump buffer and error functions */
   png_memcpy(tmp_jmp, png_ptr->jmpbuf, sizeof (jmp_buf));
 #endif

   if (sizeof(png_struct) > png_struct_size)
      {
        png_destroy_structP(png_ptr);
        png_ptr = (png_structp)png_create_structP(PNG_STRUCT_PNG);
        *ptr_ptr = png_ptr;
      }
 
    /* reset all variables to 0 */
    png_memset(png_ptr, 0, sizeof (png_struct));
 
 #ifdef PNG_SETJMP_SUPPORTED
    /* restore jump buffer */
    png_memcpy(png_ptr->jmpbuf, tmp_jmp, sizeof (jmp_buf));
 #endif
 
   png_set_write_fn(png_ptr, NULL, NULL, NULL);
 
    /* initialize zbuf - compression buffer */
    png_ptr->zbuf_size = PNG_ZBUF_SIZE;
    png_ptr->zbuf = (png_bytep)png_malloc(png_ptr,
    (png_uint_32)png_ptr->zbuf_size);
 
#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   png_set_filter_heuristics(png_ptr, PNG_FILTER_HEURISTIC_DEFAULT,
       1, NULL, NULL);
 #endif
}

static png_size_t png_check_keywordP(png_structp png_ptr, png_charp key, png_charpp new_key)
{
   png_size_t key_len;
   png_charp kp, dp;
   int kflag;
   int kwarn=0;

   png_debug(1, "in png_check_keyword");

   *new_key = NULL;

   if (key == NULL || (key_len = png_strlen(key)) == 0)
   {
      png_warning(png_ptr, "zero length keyword");
      return ((png_size_t)0);
   }

   png_debug1(2, "Keyword to be checked is '%s'", key);

   *new_key = (png_charp)png_malloc_warn(png_ptr, (png_uint_32)(key_len + 2));
   if (*new_key == NULL)
   {
      png_warning(png_ptr, "Out of memory while procesing keyword");
      return ((png_size_t)0);
   }

   /* Replace non-printing characters with a blank and print a warning */
   for (kp = key, dp = *new_key; *kp != '\0'; kp++, dp++)
   {
      if ((png_byte)*kp < 0x20 ||
         ((png_byte)*kp > 0x7E && (png_byte)*kp < 0xA1))
      {
#ifdef PNG_STDIO_SUPPORTED
         char msg[40];

         png_snprintf(msg, 40,
           "invalid keyword character 0x%02X", (png_byte)*kp);
         png_warning(png_ptr, msg);
#else
         png_warning(png_ptr, "invalid character in keyword");
#endif
         *dp = ' ';
      }
      else
      {
         *dp = *kp;
      }
   }
   *dp = '\0';

   /* Remove any trailing white space. */
   kp = *new_key + key_len - 1;
   if (*kp == ' ')
   {
      png_warning(png_ptr, "trailing spaces removed from keyword");

      while (*kp == ' ')
      {
         *(kp--) = '\0';
         key_len--;
      }
   }

   /* Remove any leading white space. */
   kp = *new_key;
   if (*kp == ' ')
   {
      png_warning(png_ptr, "leading spaces removed from keyword");

      while (*kp == ' ')
      {
         kp++;
         key_len--;
      }
   }

    /* Remove multiple internal spaces. */
   for (kflag = 0, dp = *new_key; *kp != '\0'; kp++)
   {
      if (*kp == ' ' && kflag == 0)
      {
         *(dp++) = *kp;
         kflag = 1;
      }
      else if (*kp == ' ')
      {
         key_len--;
         kwarn=1;
      }
      else
      {
         *(dp++) = *kp;
         kflag = 0;
      }
   }
   *dp = '\0';
   if (kwarn)
      png_warning(png_ptr, "extra interior spaces removed from keyword");

   if (key_len == 0)
   {
      png_free(png_ptr, *new_key);
      png_warning(png_ptr, "Zero length keyword");
   }

   if (key_len > 79)
   {
      png_warning(png_ptr, "keyword length must be 1 - 79 characters");
      (*new_key)[79] = '\0';
      key_len = 79;
   }

   return (key_len);
}


static void png_write_compressed_data_out(png_structp png_ptr, compression_state *comp)
{
   int i;

   /* Handle the no-compression case */
   if (comp->input)
   {
      png_write_chunk_data(png_ptr, (png_bytep)comp->input,
                            (png_size_t)comp->input_len);
      return;
   }

   /* Write saved output buffers, if any */
   for (i = 0; i < comp->num_output_ptr; i++)
   {
      png_write_chunk_data(png_ptr, (png_bytep)comp->output_ptr[i],
         (png_size_t)png_ptr->zbuf_size);
      png_free(png_ptr, comp->output_ptr[i]);
   }
   if (comp->max_output_ptr != 0)
      png_free(png_ptr, comp->output_ptr);
   /* Write anything left in zbuf */
   if (png_ptr->zstream.avail_out < (png_uint_32)png_ptr->zbuf_size)
      png_write_chunk_data(png_ptr, png_ptr->zbuf,
         (png_size_t)(png_ptr->zbuf_size - png_ptr->zstream.avail_out));

   /* Reset zlib for another zTXt/iTXt or image data */
   deflateReset(&png_ptr->zstream);
   png_ptr->zstream.data_type = Z_BINARY;
}

static int writefile(struct GIFelement *s, struct GIFelement *e, FILE *fp, int lastimg)
{
  int i;
  struct GIFimagestruct *img=e->imagestruct;
  unsigned long *count=img->color_count;
  GifColor *colors=img->colors;
  int gray;
  int last_color=0, colors_used=0;
  byte remap[MAXCMSIZE];
  int low_presc;
  png_struct *png_ptr = xalloc(sizeof (png_struct));
  png_info *info_ptr = xalloc(sizeof (png_info));
  int p;
  int bitdepth, gray_bitdepth;
  png_byte pal_trans[MAXCMSIZE];
  png_color_16 color16trans, color16back;
  byte buffer[24]; /* used for gIFt and gIFg */
  byte *data;
  int j;
  png_uint_16 histogr[MAXCMSIZE];
  unsigned long hist_maxvalue;
  int passcount;
  png_text software;

  if(img->trans!=-1 && !count[img->trans])
    img->trans=-1;

  gray=1;

  for(i=0;i<MAXCMSIZE;i++)
    if(count[i]) {
      gray &= graycheck(colors[i]);
      colors_used++;
      if(last_color<i) last_color=i;
    }

  if(gray) {
    fprintf(stderr, "image is grayscale\n");

    /* zero out unused colors */

    for(i=0;i<MAXCMSIZE;i++)
      if(!count[i]) {
        colors[i].red=colors[i].green=colors[i].blue=0;
      }

    if(img->trans!=-1) {
      for(i=0;i<MAXCMSIZE;i++) {
        if(i!=img->trans && colors[i].red==colors[img->trans].red) {
          gray=0;
          fprintf(stderr, "trans color is repeated in visible colors, using palette\n");
          break;
        }
      }
    }
  }

  bitdepth=8;
  if(last_color<16) bitdepth=4;
  if(last_color<4) bitdepth=2;
  if(last_color<2) bitdepth=1;

  if(gray) {
    for(i=0;i<MAXCMSIZE;i++)
      remap[i]=colors[i].red;
  }

  if(gray) {
    gray_bitdepth=8;

    /* try to adjust to 4 bit prescision grayscale */

    low_presc=1;
    for(i=0;i<MAXCMSIZE;i++) {
      if((remap[i]&0xf)*0x11!=remap[i]) {
        low_presc=0;
        break;
      }
    }
    if(low_presc) {
      for(i=0;i<MAXCMSIZE;i++) {
        remap[i] &= 0xf;
      }
      gray_bitdepth=4;
    }

    /* try to adjust to 2 bit prescision grayscale */

    if(low_presc) {
      for(i=0;i<MAXCMSIZE;i++) {
        if((remap[i]&3)*5!=remap[i]) {
          low_presc=0;
          break;
        }
      }
    }

    if(low_presc) {
      for(i=0;i<MAXCMSIZE;i++) {
        remap[i] &= 3;
      }
      gray_bitdepth=2;
    }

    /* try to adjust to 1 bit prescision grayscale */

    if(low_presc) {
      for(i=0;i<MAXCMSIZE;i++) {
        if((remap[i]&1)*3!=remap[i]) {
          low_presc=0;
          break;
        }
      }
    }
    if(low_presc) {
      for(i=0;i<MAXCMSIZE;i++) {
        remap[i] &= 1;
      }
      gray_bitdepth=1;
    }

    if(bitdepth<gray_bitdepth) {
      gray=0; /* write palette file */
    } else {
      bitdepth=gray_bitdepth;
    }
  }

  fprintf(stderr, "%d colors used, highest color %d, %s, bitdepth %d\n", colors_used, last_color, gray ? "gray":"palette", bitdepth);

  if(setjmp(png_ptr->jmpbuf)) {
    fprintf(stderr, "setjmp returns error condition\n");

    free(png_ptr);
    free(info_ptr);

    return 1;
  }

  png_write_init_3_P(&png_ptr,PNG_LIBPNG_VER_STRING, png_sizeof(png_struct));
  png_info_init(info_ptr);
  png_init_io(png_ptr, fp);
  info_ptr->width=current->imagestruct->width;
  info_ptr->height=current->imagestruct->height;
  info_ptr->bit_depth=bitdepth;
  info_ptr->color_type=gray ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_PALETTE;
  info_ptr->interlace_type=interlaced;

  if(GifScreen.AspectRatio!=0 && GifScreen.AspectRatio!=49) {
    info_ptr->x_pixels_per_unit=GifScreen.AspectRatio+15;
    info_ptr->y_pixels_per_unit=64;
    info_ptr->phys_unit_type=PNG_RESOLUTION_UNKNOWN;
    info_ptr->valid |= PNG_INFO_pHYs;
  }

  if(img->offset_x>0 && img->offset_y>0) {
    info_ptr->x_offset=img->offset_x;
    info_ptr->y_offset=img->offset_y;
    info_ptr->offset_unit_type=PNG_OFFSET_PIXEL;
    info_ptr->valid |= PNG_INFO_oFFs;
  }

  if(GifScreen.Background>0) { /* no background for palette entry 0 */
    /* if the backgroup color doesn't appear in local palette, we just
       leave it out, if the pic is part of an animation, at least some will
       use the global palette */

    if(gray) {
      if(graycheck(GifScreen.ColorMap[GifScreen.Background])) {
        color16back.gray=remap[GifScreen.Background];
        info_ptr->background=color16back;
        info_ptr->valid |= PNG_INFO_bKGD;
      }
    } else {
      for(i=0;i<MAXCMSIZE;i++) {
        if(GifScreen.ColorMap[GifScreen.Background].red==colors[i].red &&
           GifScreen.ColorMap[GifScreen.Background].green==colors[i].green &&
           GifScreen.ColorMap[GifScreen.Background].blue==colors[i].blue) {
          if(last_color<i) last_color=i;
          color16back.index=i;
          info_ptr->background=color16back;
          info_ptr->valid |= PNG_INFO_bKGD;
          break;
        }
      }
    }
  }

  if(img->trans != -1) {
    if(gray) {
      color16trans.gray=remap[img->trans];
      info_ptr->trans_values=color16trans;
    } else {
      for(i=0;i<MAXCMSIZE;i++) pal_trans[i]=255;
      pal_trans[img->trans]=0;
      info_ptr->trans=pal_trans;
      info_ptr->num_trans=img->trans+1;
    }
    info_ptr->valid |= PNG_INFO_tRNS;
  }

  if(!gray) {
    info_ptr->palette=current->imagestruct->colors;
    info_ptr->num_palette=last_color+1;
    info_ptr->valid |= PNG_INFO_PLTE;
  }

  if(histogram && !gray) {
    /* histogram are not supported for grayscale images */
    hist_maxvalue=0;
    for(i=0;i<MAXCMSIZE;i++)
      if(count[i]>hist_maxvalue)
        hist_maxvalue=count[i];
    if(hist_maxvalue<=65535) {
      /* no scaling necessary */
      for(i=0;i<MAXCMSIZE;i++)
        histogr[i]=(png_uint_16)count[i];
    } else {
      for(i=0;i<MAXCMSIZE;i++)
        if(count[i]) {
          histogr[i]=(png_uint_16)
#ifdef __GO32__
          /* avoid using fpu instructions on djgpp, so we don't need emu387 */
                                   (((long long)count[i]*65535)/hist_maxvalue);
#else
                                   ((double)count[i]*65535.0/hist_maxvalue);
#endif
          if(histogr[i]==0)
            histogr[i]=1;
        } else {
          histogr[i]=0;
        }
    }
    info_ptr->hist=histogr;
    info_ptr->valid |= PNG_INFO_hIST;
  }

  if(software_chunk) {
    software.compression=-1;
    software.key="Software";
    software.text=(char*)PNG_LIBPNG_VER_STRING;
    software.text_length=strlen(software.text);

    info_ptr->num_text=1;
    info_ptr->max_text=1;
    info_ptr->text=&software;
  }

  png_write_info(png_ptr, info_ptr);

  info_ptr->num_text=0;
  info_ptr->max_text=0;
  info_ptr->text=NULL;

  if(info_ptr->bit_depth<8)
    png_set_packing(png_ptr);

  /* loop over elements until we reach the image or the last element if
     this is the last image in a GIF */

  while(lastimg ? s!=NULL : s != e->next) {
    switch((byte)s->GIFtype) {
      case GIFimage:
        passcount=png_set_interlace_handling(png_ptr);
        for(p=0;p<passcount;p++)
          for(i=0;i<current->imagestruct->height;i++) {
            if(progress) {
              if(passcount>1)
                printf("%d/%d ", p+1, passcount);
              printf("%2d%%\r", (int)(((long)i*100)/current->imagestruct->height));
              fflush(stdout);
            }
            data=access_data(current, (long) (img->interlace ?
                             interlace_line(current->imagestruct->height,i) : i) *
                             current->imagestruct->width,
                             current->imagestruct->width);
#ifndef TMPFILE
            /* if we store the image in memory, we have to remap once */
            if(gray && p==0) {
#else
            /* if we store the image in a file, we have to remap each time */
            if(gray) {
#endif
              for(j=0;j<img->width;j++) {
                data[j]=remap[data[j]];
              }
            }
            png_write_row(png_ptr, data);
          }
        break;

      case GIFcomment:
        data=access_data(s, 0, s->size);
        if(s->size<500) {
          png_write_tEXtP(png_ptr, "Comment", data, s->size);
        } else {
          png_write_zTXtP(png_ptr, "Comment", data, s->size, 0);
        }
        break;

      case GIFapplication:
        data=access_data(s, 0, s->size);
        png_write_chunk_start(png_ptr, "gIFx", s->size);
        png_write_chunk_data(png_ptr, data, s->size);
        png_write_chunk_end(png_ptr);
        break;

/* Plain Text Extensions are currently not support due to an unresolved issue
   about transparency. Since there are practically no GIFs around that use
   this, this will not be a problem, but a future version of this program
   will probably support PTEs in full.
*/
#if 0
      case GIFplaintext:
        data=access_data(s, 0, s->size);
        /* gIFt is 12 bytes longer than GCE, due to 32 bit ints and
           rgb colors */
        png_write_chunk_start(png_ptr, "gIFt", s->size+12);
        memset(buffer, 0, 24);
        buffer[2]=data[1];
        buffer[3]=data[0];
        buffer[6]=data[3];
        buffer[7]=data[2];
        buffer[10]=data[5];
        buffer[11]=data[4];
        buffer[14]=data[7];
        buffer[15]=data[6];
        buffer[16]=data[8];
        buffer[17]=data[9];
        buffer[18]=GifScreen.ColorMap[data[10]].red;
        buffer[19]=GifScreen.ColorMap[data[10]].green;
        buffer[20]=GifScreen.ColorMap[data[10]].blue;
        buffer[21]=GifScreen.ColorMap[data[11]].red;
        buffer[22]=GifScreen.ColorMap[data[11]].green;
        buffer[23]=GifScreen.ColorMap[data[11]].blue;
        png_write_chunk_data(png_ptr, buffer, 24);
        png_write_chunk_data(png_ptr, data+12, s->size-12);
        png_write_chunk_end(png_ptr);
        break;
#endif

      case GIFgraphicctl:
        data=access_data(s, 0, s->size);
        buffer[0]=(data[0]>>2)&7;
        buffer[1]=(data[0]>>1)&1;
        buffer[2]=data[2];
        buffer[3]=data[1];

        png_write_chunk(png_ptr, "gIFg", buffer, 4);
        break;

      default:
        fprintf(stderr, "gif2png internal error: encountered unused element type %c\n", s->GIFtype);
        break;
      }
    s=s->next;
  }
  png_write_end(png_ptr, info_ptr);
//  png_write_destroy(png_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  free(png_ptr);
  free(info_ptr);

  return 0;
}
