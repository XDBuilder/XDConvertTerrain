#ifndef __C_IMAGE_BMP_H_INCLUDED__
#define __C_IMAGE_BMP_H_INCLUDED__

#include "CImage.h"

// byte-align structures
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	error compiler not supported
#endif

	struct SBMPHeader
	{
		u16	Id;						//	BM - Windows 3.1x, 95, NT, 98, 2000, ME, XP
									//	BA - OS/2 Bitmap Array
									//	CI - OS/2 Color Icon
									//	CP - OS/2 Color Pointer
									//	IC - OS/2 Icon
									//	PT - OS/2 Pointer
		u32	FileSize;
		u32	Reserved;
		u32	BitmapDataOffset;
		u32	BitmapHeaderSize;		// should be 28h for windows bitmaps or
									// 0Ch for OS/2 1.x or F0h for OS/2 2.x
		u32 Width;
		u32 Height;
		u16 Planes;
		u16 BPP;					// 1: Monochrome bitmap
									// 4: 16 color bitmap
									// 8: 256 color bitmap
									// 16: 16bit (high color) bitmap
									// 24: 24bit (true color) bitmap
									// 32: 32bit (true color) bitmap

		u32  Compression;			// 0: none (Also identified by BI_RGB)
									// 1: RLE 8-bit / pixel (Also identified by BI_RLE4)
									// 2: RLE 4-bit / pixel (Also identified by BI_RLE8)
									// 3: Bitfields  (Also identified by BI_BITFIELDS)

		u32  BitmapDataSize;		// Size of the bitmap data in bytes. This number must be rounded to the next 4 byte boundary.
		u32  PixelPerMeterX;
		u32  PixelPerMeterY;
		u32  Colors;
		u32  ImportantColors;
	} PACK_STRUCT;


// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT


class CImageBMP
{
public:

	// constructor
	CImageBMP(): BmpData(0), PaletteData(0) {}

	// destructor
	virtual ~CImageBMP();

	// returns true if the file maybe is able to be loaded by this class based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName);

	// returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	// creates a surface from the file
	virtual CImage* loadImage(FILE* file);

	// creates a file from the surface
	virtual bool writeImage(FILE* file, CImage* image, u32 param);

private:

	void decompress8BitRLE(u8*& BmpData, s32 size, s32 width, s32 height, s32 pitch);

	void decompress4BitRLE(u8*& BmpData, s32 size, s32 width, s32 height, s32 pitch);

	u8*	 BmpData;
	s32* PaletteData;

};


#endif

