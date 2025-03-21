
#ifndef __C_IMAGE_PSD_H_INCLUDED__
#define __C_IMAGE_PSD_H_INCLUDED__

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

	struct PsdHeader
	{
		c8 signature [4];	// Always equal to 8BPS.
		u16 version;		// Always equal to 1
		c8 reserved [6];	// Must be zero
		u16 channels;		// Number of any channels inc. alphas
		u32 height;		// Rows Height of image in pixel
		u32 width;		// Colums Width of image in pixel
		u16 depth;		// Bits/channel
		u16 mode;		// Color mode of the file (Bitmap/Grayscale..)
	} PACK_STRUCT;


// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT

/*!
	Surface Loader for psd images
*/
class CImagePSD 
{
public:

	//! constructor
	CImagePSD();

	//! destructor
	virtual ~CImagePSD();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName);

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	//! creates a surface from the file
	virtual CImage* loadImage(FILE* file);

private:

	bool readRawImageData(FILE* file);
	bool readRLEImageData(FILE* file);
	s16 getShiftFromChannel(c8 channelNr);

	// member variables

	u32* imageData;
	PsdHeader header;
	bool error;
};


#endif

