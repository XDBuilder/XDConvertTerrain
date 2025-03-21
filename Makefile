INC_ROOT = .
#/home/hadoop/project/jsyun/kafka/XDConvertImage
#라이브러리 헤러파일과 라이브러리 파일 위치 확인
#CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 rdkafka)
#LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka)

CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include
#LDLIBS는 예약어라 기본적인 라이브러 폴더를 조사함, 그래서 위치를 설정 안해도 됨.
LDLIBS=-lm -lglib-2.0 -lrdkafka -lpthread -ldl
#USERLIBS는 사용자 지정은 위치를 설정해야함.
#USERLIBS=-L/home/hadoop/project/jsyun/kafka/XDConvertImage -lXDProj -L/home/hadoop/project/jsyun/kafka/XDConvertImage -lXDRaster -L/home/hadoop/project/jsyun/kafka/XDConvertImage -lgdal
USERLIBS=-L./ -lXDProj -L./ -lXDRaster
OBJDIR=./objs
# 
CC = g++
COMPLGCC = $(CC) -Wall -g -lstdc++fs -std=c++17 -fpermissive $(CFLAGS) -c $< -o $*.o
#COMPLGCC = $(CC) -Wall -g -lstdc++fs -std=c++17 -fpermissive $(CFLAGS) -c $< -o $(OBJDIR)/$*.o

OBJS_ALL = $(OBJDIR)/XDConvertTerrain.o $(OBJDIR)/common.o $(OBJDIR)/XDRasterAPI.o $(OBJDIR)/XDProjAPI.o $(OBJDIR)/gdalManage.o \
$(OBJDIR)/Baseimage.o $(OBJDIR)/CImage.o $(OBJDIR)/CImagePNG.o $(OBJDIR)/CColorConverter.o \
$(OBJDIR)/png.o $(OBJDIR)/pngerror.o $(OBJDIR)/pngget.o $(OBJDIR)/pngmem.o $(OBJDIR)/pngpread.o $(OBJDIR)/pngread.o $(OBJDIR)/pngrio.o \
$(OBJDIR)/pngrtran.o $(OBJDIR)/pngrutil.o $(OBJDIR)/pngset.o $(OBJDIR)/pngtrans.o $(OBJDIR)/pngvcrd.o $(OBJDIR)/pngwio.o $(OBJDIR)/pngwrite.o \
$(OBJDIR)/pngwtran.o $(OBJDIR)/pngwutil.o $(OBJDIR)/adler32.o $(OBJDIR)/compressz.o $(OBJDIR)/crc32z.o $(OBJDIR)/deflate.o $(OBJDIR)/gzio.o \
$(OBJDIR)/infback.o $(OBJDIR)/inffast.o $(OBJDIR)/inflate.o $(OBJDIR)/inftrees.o $(OBJDIR)/trees.o $(OBJDIR)/uncompr.o $(OBJDIR)/zutil.o

#OBJS_OUT = $(addprefix $(OBJDIR)/,$(OBJS_ALL))

# objes 파일 이용해서 실행파일 만들기, 라이브러리 있는 폴더
all:$(OBJS_ALL)
	$(CC) -g -fpermissive -o ./XDConvertTerrain $(OBJS_ALL) $(LDLIBS) $(USERLIBS)
clean:
	rm $(OBJDIR)/*.o 
################################
## OBJS_ALL
################################
$(OBJDIR)/XDConvertTerrain.o:$(INC_ROOT)/XDConvertTerrain.cpp
		$(COMPLGCC)
$(OBJDIR)/common.o:$(INC_ROOT)/common.cpp
		$(COMPLGCC)

################################
## OBJS_PROJ
################################
$(OBJDIR)/XDRasterAPI.o:$(INC_ROOT)/XDRasterAPI.cpp
		$(COMPLGCC)
$(OBJDIR)/XDProjAPI.o:$(INC_ROOT)/XDProjAPI.cpp
		$(COMPLGCC)

################################
## OBJS_GDAL
################################
$(OBJDIR)/gdalManage.o:$(INC_ROOT)/gdalManage.cpp
		$(COMPLGCC)
################################
## image
################################
$(OBJDIR)/Baseimage.o:$(INC_ROOT)/images/Baseimage.cpp
		$(COMPLGCC)
$(OBJDIR)/CImage.o:$(INC_ROOT)/images/CImage.cpp
		$(COMPLGCC)
$(OBJDIR)/CImagePNG.o:$(INC_ROOT)/images/CImagePNG.cpp 
		$(COMPLGCC)
$(OBJDIR)/CColorConverter.o:$(INC_ROOT)/images/CColorConverter.cpp
		$(COMPLGCC)
################################
## png
################################
$(OBJDIR)/png.o:$(INC_ROOT)/images/libpng/png.c	          
		$(COMPLGCC)
$(OBJDIR)/pngerror.o:$(INC_ROOT)/images/libpng/pngerror.c
		$(COMPLGCC)
$(OBJDIR)/pngget.o:$(INC_ROOT)/images/libpng/pngget.c
		$(COMPLGCC)
$(OBJDIR)/pngmem.o:$(INC_ROOT)/images/libpng/pngmem.c
		$(COMPLGCC)
$(OBJDIR)/pngpread.o:$(INC_ROOT)/images/libpng/pngpread.c
		$(COMPLGCC)
$(OBJDIR)/pngread.o:$(INC_ROOT)/images/libpng/pngread.c
		$(COMPLGCC)
$(OBJDIR)/pngrio.o:$(INC_ROOT)/images/libpng/pngrio.c
		$(COMPLGCC)
$(OBJDIR)/pngrtran.o:$(INC_ROOT)/images/libpng/pngrtran.c
		$(COMPLGCC)
$(OBJDIR)/pngrutil.o:$(INC_ROOT)/images/libpng/pngrutil.c
		$(COMPLGCC)
$(OBJDIR)/pngset.o:$(INC_ROOT)/images/libpng/pngset.c
		$(COMPLGCC)
$(OBJDIR)/pngtrans.o:$(INC_ROOT)/images/libpng/pngtrans.c
		$(COMPLGCC)
$(OBJDIR)/pngvcrd.o:$(INC_ROOT)/images/libpng/pngvcrd.c
		$(COMPLGCC)
$(OBJDIR)/pngwio.o:$(INC_ROOT)/images/libpng/pngwio.c
		$(COMPLGCC)
$(OBJDIR)/pngwrite.o:$(INC_ROOT)/images/libpng/pngwrite.c
		$(COMPLGCC)
$(OBJDIR)/pngwtran.o:$(INC_ROOT)/images/libpng/pngwtran.c
		$(COMPLGCC)
$(OBJDIR)/pngwutil.o:$(INC_ROOT)/images/libpng/pngwutil.c
		$(COMPLGCC)

################################
## zip
################################
$(OBJDIR)/adler32.o:$(INC_ROOT)/zip/zlib/adler32.c
		$(COMPLGCC)
$(OBJDIR)/compressz.o:$(INC_ROOT)/zip/zlib/compressz.c
		$(COMPLGCC)
$(OBJDIR)/crc32z.o:$(INC_ROOT)/zip/zlib/crc32z.c
		$(COMPLGCC)
$(OBJDIR)/deflate.o:$(INC_ROOT)/zip/zlib/deflate.c
		$(COMPLGCC)
$(OBJDIR)/gzio.o:$(INC_ROOT)/zip/zlib/gzio.c
		$(COMPLGCC)
$(OBJDIR)/infback.o:$(INC_ROOT)/zip/zlib/infback.c
		$(COMPLGCC)
$(OBJDIR)/inffast.o:$(INC_ROOT)/zip/zlib/inffast.c
		$(COMPLGCC)
$(OBJDIR)/inflate.o:$(INC_ROOT)/zip/zlib/inflate.c
		$(COMPLGCC)
$(OBJDIR)/inftrees.o:$(INC_ROOT)/zip/zlib/inftrees.c
		$(COMPLGCC)
$(OBJDIR)/trees.o:$(INC_ROOT)/zip/zlib/trees.c
		$(COMPLGCC)
$(OBJDIR)/uncompr.o:$(INC_ROOT)/zip/zlib/uncompr.c
		$(COMPLGCC)
$(OBJDIR)/zutil.o:$(INC_ROOT)/zip/zlib/zutil.c
		$(COMPLGCC)
