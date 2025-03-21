#ifndef __C_IMAGE_GIF_H_INCLUDED__
#define __C_IMAGE_GIF_H_INCLUDED__

#include "CImage.h"

///GIF////////////////////////////////////////////////////////////////
#define        MAXCOLORMAPSIZE			256
#define        TRUE						1
#define        FALSE					0
#define        CM_RED					0
#define        CM_GREEN					1
#define        CM_BLUE 					2
#define        MAX_LWZ_BITS				12
#define        INTERLACE				0x40
#define        LOCALCOLORMAP			0x80
#define        BitSet(byte, bit)		(((byte) & (bit)) == (bit))
#define        ReadOK(file,buffer,len)	(fread(buffer, len, 1, file) != 0)
#define        LM_to_uint(a,b) 		    (((b)<<8)|(a))

typedef unsigned char byte;
typedef unsigned short WORD;
typedef unsigned long DWORD;

static struct {
       unsigned int    Width;
       unsigned int    Height;
	   RGBQUAD         ColorMap[MAXCOLORMAPSIZE];
       unsigned int    BitPixel;
       unsigned int    ColorResolution;
       unsigned int    Background;
       unsigned int    AspectRatio;
       int	           GrayScale;
} GifScreen;

static struct {
       int     transparent;
       int     delayTime;
       int     inputFlag;
       int     disposal;
} Gif89 = { -1, -1, -1, 0 };


class CImageGIF
{
public:

	// constructor
	CImageGIF();

	// destructor
	virtual ~CImageGIF();

	// returns true if the file maybe is able to be loaded by this class
	// based on the file extension (e.g. ".png")
	virtual bool isALoadableFileExtension(const c8* fileName);

	// returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	// creates a surface from the file
	virtual CImage* loadImage(char* fileName);

   	// creates a file from the surface
//	virtual bool writeImage(char* gifname, CImage* image, u32 param);

	bool gif2bmp(char* gifname,char* bmpname);
private:
	
	int dib_wib(int bitcount, int wi);
	int spew_header(int wi, int hi, int n, RGBQUAD *cmap);
	int ReadColorMap ( FILE *fd, int number, RGBQUAD *b);
	int DoExtension ( FILE *fd, int label );
	int GetDataBlock ( FILE *fd, unsigned char  *buf );
	int GetCode ( FILE *fd, int code_size, int flag );
	int LWZReadByte ( FILE *fd, int flag, int input_code_size );
	void ReadGIF ( FILE	 *fd, int imageNumber );
	void ReadImage ( FILE *fd, int len, int height, RGBQUAD *cmap, int gray, int interlace, int ignore );


	BITMAPFILEHEADER hdr;
	BITMAPINFOHEADER bi;
	long	wib;
	RGBQUAD cmap[256];
	FILE   *fp;
	int     showComment;
	int     verbose;
	int     ZeroDataBlock;
};


#endif