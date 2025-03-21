
#ifndef __C_IMAGE_H_INCLUDED__
#define __C_IMAGE_H_INCLUDED__

#include "../base/position2d.h"
#include "../base/scolor.h"
#include "../base/rect2d.h"
#include "CColorConverter.h"

#define DIB_HEADER_MARKER ((WORD)('M'<<8) | 'B')
// CImage implementation with a lot of special image operations for
// 16 bit A1R5G5B5/32 Bit A8R8G8B8 images, which are used by the SoftwareDevice.
class CImage 
{
public:

	// constructor from another image with format change
	CImage(ECOLOR_FORMAT format, CImage* imageToCopy);

	// constructor from raw image data
	// \param useForeignMemory: If true, the image will use the data pointer
	// directly and own it from now on, which means it will also try to delete [] the
	// data when the image will be destructed. If false, the memory will by copied.
	CImage(ECOLOR_FORMAT format, const dimension2di& size,
		void* data, bool ownForeignMemory=true, bool deleteMemory = true);

	// constructor for empty image
	CImage(ECOLOR_FORMAT format, const dimension2di& size);

	// constructor using a part from another image
	CImage(CImage* imageToCopy,	const position2di& pos, const dimension2di& size);

	virtual ~CImage();

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
	// draws a rectangle
	void drawRectangle(const rect2di& rect, const SColor &color);

	// copies this surface into another
	void copyTo(CImage* target, const position2di& pos=position2di(0,0));

	// copies this surface into another
	void copyTo(CImage* target, const position2di& pos, const rect2di& sourceRect, const rect2di* clipRect=0);

	// copies this surface into another, using the alpha mask, an cliprect and a color to add with
	void copyToWithAlpha(CImage* target, const position2di& pos, const rect2di& sourceRect, const SColor &color, const rect2di* clipRect = 0);

	// copies this surface into another, scaling it to fit.
	void copyToScaling(void* target, s32 width, s32 height, ECOLOR_FORMAT format, u32 pitch=0,bool bForce=false);

	// copies this surface into another, scaling it to fit.
	void copyToScaling(CImage* target,bool bForce=false); //이미지 크기 조정

	// copies this surface into another, scaling it to fit, appyling a box filter
	void copyToScalingBoxFilter(CImage* target, s32 bias = 0);

	// draws a line from to 라인 그리기.
	void drawLine(const position2di& from, const position2di& to, const SColor &color);

	// fills the surface with black or white
	void fill(const SColor &color);

	// returns pitch of image
	virtual u32 getPitch() const { return Pitch; }

	static u32 getBitsPerPixelFromFormat(ECOLOR_FORMAT format);


private:
	// assumes format and size has been set and creates the rest
	void initData();

	void setBitMasks();

	inline SColor getPixelBox ( s32 x, s32 y, s32 fx, s32 fy, s32 bias ) const;

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
