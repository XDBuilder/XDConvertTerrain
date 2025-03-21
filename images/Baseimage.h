#ifndef __BASEIMAGE_H_INCLUDED__
#define __BASEIMAGE_H_INCLUDED__
// BaseImage.h: interface for the CBaseImage class.
//
//////////////////////////////////////////////////////////////////////

#include <map>
#include "../common.h"

#define PALVERSION 0x300
#ifndef DIB_HEADER_MARKER
	#define DIB_HEADER_MARKER ((u16)('M'<<8) | 'B')
#endif
#define BOUND(x,a,b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define BI_RGB        0
#define BI_RLE8       1
#define BI_RLE4       2
#define BI_BITFIELDS  3
#define BI_JPEG       4
#define BI_PNG        5
typedef struct tagBITMAPFILEHEADER {
        unsigned short    	bfType;
        unsigned int   		bfSize;
        unsigned short    	bfReserved1;
        unsigned short    	bfReserved2;
        unsigned int  		bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;
class CBaseImage  
{
protected:

public:
	BITMAPFILEHEADER bmpHeader;
	LPBITMAPINFO pBitmapInfo;
	u8* m_pDib;
	int m_ndumy;
	int m_nColor;
	LPLOGPALETTE lpLogPalette;
	char m_strFileName[512];

	std::map<COLORREF, u8> m_palette;

public:
	CBaseImage();
	virtual ~CBaseImage();

public:
//Create : size 크기의 영상을 colornum의 색으로 배경을 backcolor로 만든다.
	bool Create(SIZE size, int colornum,COLORREF backcolor);
//GetSize : 이미지의 크기를 안다.
	SIZE GetSize();
//LoadImage : bmp 파일을 불러온다.
	bool LoadImage(const char * file); // 24비트 32비트
	bool DibReadBitmapInfo( u8** offset, BITMAPINFOHEADER *pdib );
	void Bitfield2RGB(u8 *src, u16 redmask, u16 greenmask, u16 bluemask, u8 bpp);
//SaveImage : bmp 파일을 저장한다.
	bool SaveImage(const char * file);
//파일명 얻기
	char * GetFileName(){ return m_strFileName; }
//파일 칼라수 얻기
	int GetColorNum(){ return m_nColor; }
//팔레트 얻기
	LPBITMAPINFO GetBitmapInfo(){ return pBitmapInfo; };
//FindColorInPalette : 팔레트 안에서 fi 의 색이 있는가를 찾는다.
	int FindColorInPalette(COLORREF fi);
//SetSystemPalette : 윈도우에 설정되어있는 팔레트를 사용한다.


	void SetPaletteInfo();

	COLORREF GetPixelColor(int cx,int cy);
	void SetPixelColor(int cx,int cy,COLORREF color);

	void ImageResize(int w,int h, int flag=0);//24 비트 이미지 기준
	void ImageRsize(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH ,int flag) ;
	void ImageResample(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH);

	bool LoadTiff( char* tifname );
	bool LoadImageMemory(u8* data, u32 isize);
	u8* SaveImageToMemory(u32& isize);

	void makeTransColor();


	u8* Save32bitTGAMemory( u32& isize, COLORREF colorkey);

	//SetBmpInfo:	szBitmapInfo에 값을 설정하는 함수
	bool SetBmpInfo(SIZE size, int colornum);
	//AllocBmpInfo : pBitmapInfo를 메모리에 설정하는 함수
	bool AllocBmpInfo(int colornum);
	//CreatePalette : mode가 1이면 pBitmapInfo 에서 불러오고 그렇지 않으면 만듬
	bool CreatePalette(bool mode);
	bool CreateByDIB(SIZE size, int colornum,u8* dib);
	//void SaveImageToMemory(u32& isize,u8* data);


};
#endif
