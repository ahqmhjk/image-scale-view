/*
 * memory.c: memory storage for gif conversion
 * Copyright (C) 1995 Alexander Lehmann
 * For conditions of distribution and use, see copyright notice in gif2png.h
 */

/*
 * currently two modes are supported:
 * - (default) store complete images in continuos blocks in memory, this works
 *   with systems that support virtual memory, e.g. unix, djgpp, etc.
 * - (-DTMPFILE) store the image data in a temporary file, this should work on
 *  any system regardless of memory restrictions, but may be significantly
 *  slower than the full memory version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "gif2png.h"

#ifdef TMPFILE
FILE *tempfile;
#endif

void *xalloc(unsigned long s)
{
  void *p=malloc((size_t)s);

  if(p==NULL) {
    fprintf(stderr, "fatal error, out of memory\n");
    fprintf(stderr, "exiting ungracefully\n");
    exit(1);
  }

  return p;
}

void *xrealloc(void *p, unsigned long s)
{
  p=realloc(p,(size_t)s);

  if(p==NULL) {
    fprintf(stderr, "fatal error, out of memory\n");
    fprintf(stderr, "exiting ungracefully\n");
    exit(1);
  }

  return p;
}

#ifndef TMPFILE

/* assume unlimited memory */

/* allocate a new GIFelement, advance current pointer and initialize
   size to 0 */

#define ALLOCSIZE 8192

void allocate_element(void)
{
  struct GIFelement *new=xalloc(sizeof(*new));

  memset(new, 0, sizeof(*new));

  new->next=NULL; /* just in case NULL is not represented by a binary 0 */

  current->next=new;
  current=new;
}

/* set size of current element to at least size bytes */
void set_size(long size)
{
  long nalloc;

  if(current->allocated_size==0) {
    nalloc=size;
    if(nalloc<ALLOCSIZE) nalloc=ALLOCSIZE;
    current->data=xalloc(nalloc);
    current->allocated_size=nalloc;
  } else
  if(current->allocated_size<size) {
    nalloc=size-current->allocated_size;
    if(nalloc<ALLOCSIZE) nalloc=ALLOCSIZE;
    current->data=xrealloc(current->data, current->allocated_size+nalloc);
    current->allocated_size+=nalloc;
  }
}

void store_block(char *data, int size)
{
  set_size(current->size+size);
  memcpy(current->data+current->size, data, size);
  current->size+=size;
}

void allocate_image(void)
{
  allocate_element();
  current->GIFtype=GIFimage;
  current->imagestruct=xalloc(sizeof(*current->imagestruct));
  memset(current->imagestruct, 0, sizeof(*current->imagestruct));
}

void fix_current(void)
{
  if(current->allocated_size!=current->size) {
    current->data=xrealloc(current->data, current->size);
    current->allocated_size=current->size;
  }
}

byte *access_data(struct GIFelement *e, unsigned long pos, unsigned long len)
{
  return e->data+pos;
}

void free_mem(void)
{
  struct GIFelement *p,*t;

  p=first.next;
  first.next=NULL;

  while(p) {
    t=p;
    p=p->next;
    if(t->data) free(t->data);
    if(t->imagestruct) free(t->imagestruct);
    free(t);
  }
}
#else /* TMPFILE */

/* use temporary file for most of the data */

/* allocate a new GIFelement, advance current pointer and initialize
   size to 0 */

void allocate_element(void)
{
  struct GIFelement *new=xalloc(sizeof(*new));

  memset(new, 0, sizeof(*new));
  current->next=new;
  current=new;

  current->file_offset=ftell(tempfile);
}

void set_size(long size)
{
}

void store_block(char *data, int size)
{
  fwrite(data, 1, size, tempfile);
  current->size+=size;
}

void allocate_image(void)
{
  allocate_element();
  current->GIFtype=GIFimage;
  current->imagestruct=xalloc(sizeof(*current->imagestruct));
  memset(current->imagestruct, 0, sizeof(*current->imagestruct));
}

void fix_current(void)
{
}

byte *access_data(struct GIFelement *e, unsigned long pos, unsigned long len)
{
  static byte *data=NULL;
  static size_t allocated_size=0;

#ifdef PNG_MAX_ALLOC_64K
  if(len>65535) {
    fprintf(stderr, "single image element too large, use the 32 bit version of gif2png\n");
    exit(1);
  }
#endif

  if(len==0) {
    free(data);
    data=NULL;
    allocated_size=0;
    return NULL;
  }

  if(data==NULL) {
    data=xalloc(len);
    allocated_size=len;
  } else {
    if(allocated_size!=len) {
      data=xrealloc(data, len);
      allocated_size=len;
    }
  }

  fseek(tempfile, e->file_offset+pos, SEEK_SET);
  fread(data, 1, len, tempfile);

  return data;
}

void free_mem(void)
{
  struct GIFelement *p,*t;

  p=first.next;
  first.next=NULL;

  while(p) {
    t=p;
    p=p->next;
    if(t->imagestruct) free(t->imagestruct);
    free(t);
  }
  access_data(NULL, 0, 0);
}
#endif /* TMPFILE */

