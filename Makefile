 ##################################################
 # copyright-2010-Hu Jiakuan.                     #
 #                                                #
 #                                                #
 ###################################################

ifndef BINDIR
BINDIR  = /usr/local/bin
endif

ifndef INSTALL
INSTALL = install -s -m 755
endif
CC = gcc
CPP = g++
ifdef GTK
LIB = -lpng -lz -lm -ljpeg `pkg-config --cflags --libs gthread-2.0 gtk+-3.0` -lX11
CFLAGS=-g -D GTK3
else
LIB = -lpng -lz -lm -ljpeg `pkg-config --cflags --libs gthread-2.0 gtk+-2.0` -lX11
CFLAGS=-g
endif
INCLUDE = -I./inc -I./signal -I./utils -I./utils/private -I./utils/bmp_png -I./utils/bmp2jpg -I./utils/gif_png -I./utils/jpeg2bmp -I./utils/capture
RM = rm -rf
EXEC = gtkwin
OBJTMP = obj
OBJ = obj/main.o obj/bmp2png.o obj/common.o obj/signal.o obj/png2bmp.o obj/bmp2jpg.o obj/gif2png.o obj/gifread.o obj/memory.o obj/jpg2bmp.o obj/capture.o


$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LIB)
obj/main.o: test/maintest.c signal/p_signal.h
	$(CC) $(CFLAGS) -o obj/main.o -c test/maintest.c $(INCLUDE) $(LIB)
obj/signal.o: signal/p_signal.c signal/p_signal.h inc/transform.h
	$(CC) $(CFLAGS) -o obj/signal.o -c signal/p_signal.c $(INCLUDE) $(LIB) -lX11
obj/bmp2png.o: utils/bmp_png/bmp2png.c utils/bmp_png/bmp_png.h utils/private/common.h utils/private/bmphed.h
	$(CC) $(CFLAGS) -o obj/bmp2png.o -c utils/bmp_png/bmp2png.c $(INCLUDE) $(LIB)
obj/png2bmp.o: utils/bmp_png/png2bmp.c utils/bmp_png/bmp_png.h utils/private/common.h utils/private/bmphed.h
	$(CC) $(CFLAGS) -o obj/png2bmp.o -c utils/bmp_png/png2bmp.c $(INCLUDE) $(LIB)
obj/common.o: utils/private/common.c utils/private/common.h
	$(CC) $(CFLAGS) -o obj/common.o -c utils/private/common.c $(INCLUDE) $(LIB)
obj/bmp2jpg.o: utils/bmp2jpg/bmptojpg.c utils/bmp2jpg/bmp2jpg.h
	$(CC) $(CFLAGS) -o obj/bmp2jpg.o -c utils/bmp2jpg/bmptojpg.c $(INCLUDE) $(LIB)
obj/jpg2bmp.o: utils/jpeg2bmp/djpeg.c utils/jpeg2bmp/cdjpeg.h
	$(CC) $(CFLAGS) -o obj/jpg2bmp.o -c utils/jpeg2bmp/djpeg.c $(INCLUDE) $(LIB)
obj/gif2png.o: utils/gif_png/gif2png.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o obj/gif2png.o -c utils/gif_png/gif2png.c $(INCLUDE) $(LIB)
obj/gifread.o: utils/gif_png/gifread.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o obj/gifread.o -c utils/gif_png/gifread.c $(INCLUDE) $(LIB)
obj/memory.o: utils/gif_png/memory.c utils/gif_png/gif2png.h
	$(CC) $(CFLAGS) -o obj/memory.o -c utils/gif_png/memory.c $(INCLUDE) $(LIB)
obj/capture.o: utils/capture/capture.c utils/capture/capture.h
	$(CC) $(CFLAGS) -o obj/capture.o -c utils/capture/capture.c $(INCLUDE) $(LIB)

.PHONY: clean
clean:
	$(RM) $(OBJTMP)/*.o $(EXEC)


install:
	cp $(EXEC) /usr/bin/

uninstall:
	rm /usr/bin/$(EXEC)
