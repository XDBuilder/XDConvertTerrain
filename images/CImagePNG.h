// this file was created by rt (www.tomkorp.com), based on ttk's png-reader
// i wanted to be able to read in PNG images with irrlicht :)
// why?  lossless compression with 8-bit alpha channel!
#ifndef __C_IMAGE_LOADER_PNG_H_INCLUDED__
#define __C_IMAGE_LOADER_PNG_H_INCLUDED__

#include "CImage.h"
#include "../base/memfile.h"
//!  Surface Loader for PNG files
class CImagePNG 
{
public:

	// constructor
	CImagePNG();

	// destructor
	virtual ~CImagePNG();

	// returns true if the file maybe is able to be loaded by this class
	// based on the file extension (e.g. ".png")
	virtual bool isALoadableFileExtension(const c8* fileName);

	// returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	// creates a surface from the file
	virtual CImage* loadImage(FILE* file);

   	// creates a file from the surface
	virtual bool writeImage(FILE* file, CImage* image, u32 param);

//private:
public:
	//some variables
	u32 Width;
	u32 Height;
	s32 BitDepth;
	s32 ColorType;
	CImage* Image;
	u8** RowPointers; //Used to point to image rows
};

#endif
