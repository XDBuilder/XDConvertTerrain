
#ifndef __C_IMAGE_H_INCLUDED__
#define __C_IMAGE_H_INCLUDED__

#include "../base/position2d.h"
#include "../base/scolor.h"
#include "../base/rect2d.h"
//#include "CColorConverter.h"

enum ECOLOR_FORMAT
{
	// 16 bit color format used by the software driver, and thus preferred
	// by all other engine video drivers. There are 5 bits for every
	// color component, and a single bit is left for alpha information.
	ECF_A1R5G5B5 = 0,

	// Standard 16 bit color format.
	ECF_R5G6B5,

	// 24 bit color, no alpha channel, but 8 bit for red, green and blue.
	ECF_R8G8B8,

	// Default 32 bit color format. 8 bits are used for every component: red, green, blue and alpha.
	ECF_A8R8G8B8
};
#ifndef DIB_HEADER_MARKER
#define DIB_HEADER_MARKER ((WORD)('M'<<8) | 'B')
#endif
// CXImage implementation with a lot of special image operations for
// 16 bit A1R5G5B5/32 Bit A8R8G8B8 images, which are used by the SoftwareDevice.
class CXImage
{
public:

	// constructor from raw image data
	// \param useForeignMemory: If true, the image will use the data pointer
	// directly and own it from now on, which means it will also try to delete [] the
	// data when the image will be destructed. If false, the memory will by copied.
	CXImage(ECOLOR_FORMAT format, const dimension2di& size,
		void* data, bool ownForeignMemory=true, bool deleteMemory = true);

	// constructor for empty image
	CXImage(ECOLOR_FORMAT format, const dimension2di& size);

	virtual ~CXImage();

	// Lock function.
	virtual void* lock(){ return Data;	};
	virtual void unlock() {};

	// Returns width and height of image data.
	virtual const dimension2di& getDimension() const;

	// Returns bits per pixel.
	virtual u32 getBitsPerPixel() const;
	// Returns bytes per pixel
	virtual u32 getBytesPerPixel() const;
	// Returns image data size in bytes
	virtual u32 getImageDataSizeInBytes() const;
	// Returns image data size in pixels
	virtual u32 getImageDataSizeInPixels() const;

	// returns mask for red,green,blue,alpha value of a pixel
	virtual u32 getRedMask() const;
	virtual u32 getGreenMask() const;
	virtual u32 getBlueMask() const;
	virtual u32 getAlphaMask() const;

	// returns a pixel
	virtual SColor getPixel(u32 x, u32 y) const;
	// sets a pixel
	virtual void setPixel(u32 x, u32 y, const SColor &color );
	// returns the color format
	virtual ECOLOR_FORMAT getColorFormat() const;

	// returns pitch of image
	virtual u32 getPitch() const { return Pitch; }

	static u32 getBitsPerPixelFromFormat(ECOLOR_FORMAT format);


private:
	// assumes format and size has been set and creates the rest
	void initData();

	void setBitMasks();


	void* Data;
	dimension2di Size;
	u32 BitsPerPixel;
	u32 BytesPerPixel;
	u32 Pitch;
	ECOLOR_FORMAT Format;

	bool DeleteMemory;

	u32 RedMask;
	u32 GreenMask;
	u32 BlueMask;
	u32 AlphaMask;
};

#endif
