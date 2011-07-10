ifndef INSTALL
INSTALL = install -s -m 755
endif

ifeq ($(DEBUG), yes)
CFLAGS=-g
else
CFLAGS=
endif

ifeq ($(GTK), 3)
LIB = -lpng -lz -lm -ljpeg `pkg-config --cflags --libs gthread-2.0 gtk+-3.0` -lX11
CFLAGS+=-D GTK3
else
LIB = -lpng -lz -lm -ljpeg `pkg-config --cflags --libs gthread-2.0 gtk+-2.0` -lX11
endif
INCLUDE = -I./inc -I./signal -I./utils -I./utils/private -I./utils/bmp_png -I./utils/bmp2jpg -I./utils/gif_png -I./utils/jpeg2bmp -I./utils/capture
RM = rm -rf
OBJ = $(OUTDIR)/main.o $(OUTDIR)/bmp2png.o $(OUTDIR)/common.o $(OUTDIR)/signal.o $(OUTDIR)/png2bmp.o $(OUTDIR)/bmp2jpg.o $(OUTDIR)/gif2png.o $(OUTDIR)/gifread.o $(OUTDIR)/memory.o $(OUTDIR)/jpg2bmp.o $(OUTDIR)/capture.o


$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LIB)
$(OUTDIR)/main.o: test/maintest.c signal/p_signal.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/main.o -c test/maintest.c $(INCLUDE) $(LIB)
$(OUTDIR)/signal.o: signal/p_signal.c signal/p_signal.h inc/transform.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/signal.o -c signal/p_signal.c $(INCLUDE) $(LIB) -lX11
$(OUTDIR)/bmp2png.o: utils/bmp_png/bmp2png.c utils/bmp_png/bmp_png.h utils/private/common.h utils/private/bmphed.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/bmp2png.o -c utils/bmp_png/bmp2png.c $(INCLUDE) $(LIB)
$(OUTDIR)/png2bmp.o: utils/bmp_png/png2bmp.c utils/bmp_png/bmp_png.h utils/private/common.h utils/private/bmphed.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/png2bmp.o -c utils/bmp_png/png2bmp.c $(INCLUDE) $(LIB)
$(OUTDIR)/common.o: utils/private/common.c utils/private/common.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/common.o -c utils/private/common.c $(INCLUDE) $(LIB)
$(OUTDIR)/bmp2jpg.o: utils/bmp2jpg/bmptojpg.c utils/bmp2jpg/bmp2jpg.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/bmp2jpg.o -c utils/bmp2jpg/bmptojpg.c $(INCLUDE) $(LIB)
$(OUTDIR)/jpg2bmp.o: utils/jpeg2bmp/djpeg.c utils/jpeg2bmp/cdjpeg.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/jpg2bmp.o -c utils/jpeg2bmp/djpeg.c $(INCLUDE) $(LIB)
$(OUTDIR)/gif2png.o: utils/gif_png/gif2png.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/gif2png.o -c utils/gif_png/gif2png.c $(INCLUDE) $(LIB)
$(OUTDIR)/gifread.o: utils/gif_png/gifread.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/gifread.o -c utils/gif_png/gifread.c $(INCLUDE) $(LIB)
$(OUTDIR)/memory.o: utils/gif_png/memory.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/memory.o -c utils/gif_png/memory.c $(INCLUDE) $(LIB)
$(OUTDIR)/capture.o: utils/capture/capture.c utils/capture/capture.h
	$(CC) $(CFLAGS) -o $(OUTDIR)/capture.o -c utils/capture/capture.c $(INCLUDE) $(LIB)

.PHONY: clean distclean
clean:
	$(RM) $(OUTDIR) $(EXEC) 

distclean: clean
	$(RM) Makefile

install:
	cp -u $(EXEC) $(BINDIR)

uninstall:
	rm $(BINDIR)/$(EXEC)
