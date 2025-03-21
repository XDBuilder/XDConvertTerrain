#ifndef __C_IMAGE_TGA_H_INCLUDED__
#define __C_IMAGE_TGA_H_INCLUDED__

#include "CImage.h"


// byte-align structures
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	define PACK_STRUCT
#endif

	// these structs are also used in the TGA writer
	struct STGAHeader{
		u8 IdLength;
		u8 ColorMapType;
		u8 ImageType;
		u8 FirstEntryIndex[2];
		u16 ColorMapLength;
		u8 ColorMapEntrySize;
		u8 XOrigin[2];
		u8 YOrigin[2];
		u16 ImageWidth;
		u16 ImageHeight;
		u8 PixelDepth;
		u8 ImageDescriptor;
	} PACK_STRUCT;

	struct STGAFooter
	{
		u32 ExtensionOffset;
		u32 DeveloperOffset;
		c8  Signature[18];
	} PACK_STRUCT;

// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT

/*!
	Surface Loader for targa images
*/
class CImageTGA
{
public:

	//! constructor
	CImageTGA();

	//! destructor
	virtual ~CImageTGA();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName);

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	//! creates a surface from the file
	virtual CImage* loadImage(FILE* file);

	virtual bool writeImage(FILE *file, CImage *image,u32 param);

	bool tga2bmp(char* tganame,char* bmpname);
private:

	//! loads a compressed tga. Was written and sent in by Jon Pry, thank you very much!
	u8* loadCompressedImage(FILE *file, const STGAHeader& header);

};

#endif

