// ImageConvert.h: interface for the CImageConvert class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __C_IMAGE_CONVERTER_H_INCLUDED__
#define __C_IMAGE_CONVERTER_H_INCLUDED__

#include "CImage.h"

#pragma comment(lib, "Image/ijl11.lib")

#define DIB_HEADER_MARKER ((WORD)('M'<<8) | 'B')

bool isLoadableImageFile(char* imgname);
HBITMAP GetImgHandle( char* imgname );
CImage* GetImagePtr( char* imgname );
bool ChangeSaveImageFormat( char* inputname, char* outputname );

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

int dib_wib(int bitcount, int wi);
int spew_header(int wi, int hi, int n, RGBQUAD *cmap);
int ReadColorMap ( FILE *fd, int number, RGBQUAD *b);
int DoExtension ( FILE *fd, int label );
int GetDataBlock ( FILE *fd, unsigned char  *buf );
int GetCode ( FILE *fd, int code_size, int flag );
int LWZReadByte ( FILE *fd, int flag, int input_code_size );
void ReadGIF ( FILE	 *fd, int imageNumber );
void ReadImage ( FILE *fd, int len, int height, RGBQUAD *cmap, int gray, int interlace, int ignore );
BOOL Gif2Bmp(LPCTSTR gifname,LPCTSTR bmpname);


BOOL Jpg2Bmp(LPCTSTR jpgname,LPCTSTR bmpname);
BOOL Bmp2Jpg(LPCTSTR bmpname,LPCTSTR jpgname);
BOOL Pcx2Bmp(LPCTSTR pcxname,LPCTSTR bmpname);
BOOL Tga2Bmp(LPCTSTR tganame,LPCTSTR bmpname);

byte* Jpg2BmpMemory( byte* jpgbuf, DWORD jsize, DWORD& bsize);
byte* Bmp2JpgMemory( byte* bmpbuf, DWORD dsize, DWORD& jsize, DWORD quality );

BOOL ImageFileYN(LPCTSTR imgname);

void FlipY(BYTE* image,CSize size,int width);
byte* Bmp2JpgMemoryByDIB( byte* bmpbuf,int bx,int by ,DWORD dsize, DWORD& jsize, DWORD quality );
byte* Jpg2BmpMemoryByDIB(byte* jpgbuf,DWORD jsize, DWORD& bsize,int &width,int &height);
bool BmpMemory2JpgFile( byte* bmpbuf, DWORD dsize, char * jpgname, DWORD quality=75 );
byte* JpgFile2BmpMemory(LPCTSTR jpgname,DWORD& bsize);

#endif // 
