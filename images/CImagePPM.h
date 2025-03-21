#ifndef __C_IMAGE_PPM_H_INCLUDED__
#define __C_IMAGE_PPM_H_INCLUDED__

#include "CImage.h"
#include "../core/xdstring.h"


class CImagePPM
{
public:

	// constructor
	CImagePPM();

	// destructor
	virtual ~CImagePPM();

	// returns true if the file maybe is able to be loaded by this class
	// based on the file extension (e.g. ".png")
	virtual bool isALoadableFileExtension(const c8* fileName);

	// returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	// creates a surface from the file
	virtual CImage* loadImage(FILE* file);

   	// creates a file from the surface
	virtual bool writeImage( FILE *file, CImage *image, u32 param) const;

private:
	
	long  m_fileSize;
	//! read the next token from file
	void getNextToken( FILE* file, xstringc& token) const;
	//! skip to next token (skip whitespace)
	void skipToNextToken( FILE* file ) const;

};


#endif