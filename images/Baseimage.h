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
//Create : size ũ���� ������ colornum�� ������ ����� backcolor�� �����.
	bool Create(SIZE size, int colornum,COLORREF backcolor);
//GetSize : �̹����� ũ�⸦ �ȴ�.
	SIZE GetSize();
//LoadImage : bmp ������ �ҷ��´�.
	bool LoadImage(const char * file); // 24��Ʈ 32��Ʈ
	bool DibReadBitmapInfo( u8** offset, BITMAPINFOHEADER *pdib );
	void Bitfield2RGB(u8 *src, u16 redmask, u16 greenmask, u16 bluemask, u8 bpp);
//SaveImage : bmp ������ �����Ѵ�.
	bool SaveImage(const char * file);
//���ϸ� ���
	char * GetFileName(){ return m_strFileName; }
//���� Į��� ���
	int GetColorNum(){ return m_nColor; }
//�ȷ�Ʈ ���
	LPBITMAPINFO GetBitmapInfo(){ return pBitmapInfo; };
//FindColorInPalette : �ȷ�Ʈ �ȿ��� fi �� ���� �ִ°��� ã�´�.
	int FindColorInPalette(COLORREF fi);
//SetSystemPalette : �����쿡 �����Ǿ��ִ� �ȷ�Ʈ�� ����Ѵ�.


	void SetPaletteInfo();

	COLORREF GetPixelColor(int cx,int cy);
	void SetPixelColor(int cx,int cy,COLORREF color);

	void ImageResize(int w,int h, int flag=0);//24 ��Ʈ �̹��� ����
	void ImageRsize(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH ,int flag) ;
	void ImageResample(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH);

	bool LoadTiff( char* tifname );
	bool LoadImageMemory(u8* data, u32 isize);
	u8* SaveImageToMemory(u32& isize);

	void makeTransColor();


	u8* Save32bitTGAMemory( u32& isize, COLORREF colorkey);

	//SetBmpInfo:	szBitmapInfo�� ���� �����ϴ� �Լ�
	bool SetBmpInfo(SIZE size, int colornum);
	//AllocBmpInfo : pBitmapInfo�� �޸𸮿� �����ϴ� �Լ�
	bool AllocBmpInfo(int colornum);
	//CreatePalette : mode�� 1�̸� pBitmapInfo ���� �ҷ����� �׷��� ������ ����
	bool CreatePalette(bool mode);
	bool CreateByDIB(SIZE size, int colornum,u8* dib);
	//void SaveImageToMemory(u32& isize,u8* data);


};
#endif
