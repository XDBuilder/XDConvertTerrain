
#ifndef __C_COLOR_CONVERTER_H_INCLUDED__
#define __C_COLOR_CONVERTER_H_INCLUDED__
#include <string.h>
#include "../base/xdtypes.h"

// An enum for the color format of textures used by the Engine.
// A color format specifies how color information is stored. */
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


class CColorConverter
{
public:

	// converts a monochrome bitmap to A1R5G5B5
	static void convert1BitTo16Bit(const u8* in, s16* out, s32 width, s32 height, s32 linepad=0, bool flip=false);

	// converts a 4 bit palettized image to A1R5G5B5
	static void convert4BitTo16Bit(const u8* in, s16* out, s32 width, s32 height, const s32* palette, s32 linepad=0, bool flip=false);

	// converts a 8 bit palettized image to A1R5G5B5
	static void convert8BitTo16Bit(const u8* in, s16* out, s32 width, s32 height, const s32* palette, s32 linepad=0, bool flip=false);

	// converts R8G8B8 16 bit data to A1R5G5B5 data
	static void convert16BitTo16Bit(const s16* in, s16* out, s32 width, s32 height, s32 linepad=0, bool flip=false);

	// copies R8G8B8 24 bit data to 24 data, and flips and  mirrors the image during the process.
	static void convert24BitTo24Bit(const u8* in, u8* out, s32 width, s32 height, s32 linepad=0, bool flip=false, bool bgr=false);

	// Resizes the surface to a new size and converts it at the same time to an A8R8G8B8 format, returning the pointer to the new buffer.
	static void convert16bitToA8R8G8B8andResize(const s16* in, s32* out, s32 newWidth, s32 newHeight, s32 currentWidth, s32 currentHeight);

	// copies X8R8G8B8 32 bit data, and flips and mirrors the image during the process.
	static void convert32BitTo32Bit(const s32* in, s32* out, s32 width, s32 height, s32 linepad, bool flip=false);


	// functions for converting one image format to another efficiently and hopefully correctly.
	// \param sP pointer to source pixel data
	// \param sN number of source pixels to copy
	// \param dP pointer to destination data buffer. must be big enough to hold sN pixels in the output format.
	static void convert_A1R5G5B5toR8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_A1R5G5B5toB8G8R8(const void* sP, s32 sN, void* dP);
	static void convert_A1R5G5B5toA8R8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_A1R5G5B5toA1R5G5B5(const void* sP, s32 sN, void* dP);
	static void convert_A1R5G5B5toR5G6B5(const void* sP, s32 sN, void* dP);

	static void convert_A8R8G8B8toR8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_A8R8G8B8toB8G8R8(const void* sP, s32 sN, void* dP);
	static void convert_A8R8G8B8toA8R8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_A8R8G8B8toA1R5G5B5(const void* sP, s32 sN, void* dP);
	static void convert_A8R8G8B8toR5G6B5(const void* sP, s32 sN, void* dP);

	static void convert_A8R8G8B8toR3G3B2(const void* sP, s32 sN, void* dP);
	static void convert_R8G8B8toR8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_R8G8B8toA8R8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_R8G8B8toA1R5G5B5(const void* sP, s32 sN, void* dP);
	static void convert_R8G8B8toR5G6B5(const void* sP, s32 sN, void* dP);

	static void convert_R5G6B5toR5G6B5(const void* sP, s32 sN, void* dP);
	static void convert_R5G6B5toR8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_R5G6B5toB8G8R8(const void* sP, s32 sN, void* dP);
	static void convert_R5G6B5toA8R8G8B8(const void* sP, s32 sN, void* dP);
	static void convert_R5G6B5toA1R5G5B5(const void* sP, s32 sN, void* dP);
	static void convert_viaFormat(const void* sP, ECOLOR_FORMAT sF, s32 sN,	void* dP, ECOLOR_FORMAT dF);
};

#endif

