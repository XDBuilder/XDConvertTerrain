
#ifndef __C_IMAGE_PCX_H_INCLUDED__
#define __C_IMAGE_PCX_H_INCLUDED__

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

	struct SPCXHeader
	{
		u8	Manufacturer;
		u8	Version;
		u8	Encoding;
		u8	BitsPerPixel;
		u16	XMin;
		u16	YMin;
		u16	XMax;
		u16	YMax;
		u16	HorizDPI;
		u16	VertDPI;
		u8	Palette[48];
		u8	Reserved;
		u8	Planes;
		u16	BytesPerLine;
		u16	PaletteType;
		u16	HScrSize;
		u16	VScrSize;
		u8	Filler[54];
	} PACK_STRUCT;


// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT


// Image Loader for Windows PCX bitmaps.
// This loader was written and sent in by Dean P. Macri. I modified	only some small bits of it.

class CImagePCX 
{
public:

	//! constructor
	CImagePCX();

	//! destructor
	virtual ~CImagePCX();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName);

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	//! creates a surface from the file
	virtual CImage* loadImage(FILE* file);

	bool pcx2bmp(char* pcxname,char* bmpname);

private:

	u8* PCXData;
	s32* PaletteData;
};


#endif

