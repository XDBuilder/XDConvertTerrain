#include "CImageBMP.h"
#include "../base/scolor.h"
#include "CColorConverter.h"
#include <stdio.h>
//! destructor
CImageBMP::~CImageBMP()
{
	if (PaletteData)
		delete [] PaletteData;

	if (BmpData)
		delete [] BmpData;
}


// returns true if the file maybe is able to be loaded by this class based on the file extension (e.g. ".tga")
bool CImageBMP::isALoadableFileExtension(const c8* fileName)
{
	return strstr(fileName, ".bmp") != 0;
}


//! returns true if the file maybe is able to be loaded by this class
bool CImageBMP::isALoadableFileFormat(FILE* file)
{
	u16 headerID;
	fread(&headerID, sizeof(u16),1,file);
#ifdef __BIG_ENDIAN__
	headerID = byteswap(headerID);
#endif
	return headerID == 0x4d42;
}


void CImageBMP::decompress8BitRLE(u8*& BmpData, s32 size, s32 width, s32 height, s32 pitch)
{
	u8* p = BmpData;
	u8* newBmp = new u8[(width+pitch)*height];
	u8* d = newBmp;
	u8* destEnd = newBmp + (width+pitch)*height;
	s32 line = 0;

	while (BmpData - p < size && d < destEnd)
	{
		if (*p == 0)
		{
			++p;

			switch(*p)
			{
			case 0: // end of line
				++p;
				++line;
				d = newBmp + (line*(width+pitch));
				break;
			case 1: // end of bmp
				delete [] BmpData;
				BmpData = newBmp;
				return;
			case 2:
				++p; d +=(u8)*p;  // delta
				++p; d += ((u8)*p)*(width+pitch);
				++p;
				break;
			default:
				{
					// absolute mode
					s32 count = (u8)*p; ++p;
					s32 readAdditional = ((2-(count%2))%2);
					s32 i;

					for (i=0; i<count; ++i)
					{
						*d = *p;
						++p;
						++d;
					}

					for (i=0; i<readAdditional; ++i)
						++p;
				}
			}
		}
		else
		{
			s32 count = (u8)*p; ++p;
			s32 color = (u8)*p; ++p;
			for (s32 i=0; i<count; ++i)
			{
				*d = color;
				++d;
			}
		}
	}

	delete [] BmpData;
	BmpData = newBmp;
}


void CImageBMP::decompress4BitRLE(u8*& BmpData, s32 size, s32 width, s32 height, s32 pitch)
{
	s32 lineWidth = (width+1)/2+pitch;
	u8* p = BmpData;
	u8* newBmp = new u8[lineWidth*height];
	u8* d = newBmp;
	u8* destEnd = newBmp + lineWidth*height;
	s32 line = 0;
	s32 shift = 4;

	while (BmpData - p < size && d < destEnd)
	{
		if (*p == 0)
		{
			++p;

			switch(*p)
			{
			case 0: // end of line
				++p;
				++line;
				d = newBmp + (line*lineWidth);
				shift = 4;
				break;
			case 1: // end of bmp
				delete [] BmpData;
				BmpData = newBmp;
				return;
			case 2:
				{
					++p;
					s32 x = (u8)*p; ++p;
					s32 y = (u8)*p; ++p;
					d += x/2 + y*lineWidth;
					shift = x%2==0 ? 4 : 0;
				}
				break;
			default:
				{
					// absolute mode
					s32 count = (u8)*p; ++p;
					s32 readAdditional = ((2-((count)%2))%2);
					s32 readShift = 4;
					s32 i;

					for (i=0; i<count; ++i)
					{
						s32 color = (((u8)*p) >> readShift) & 0x0f;
						readShift -= 4;
						if (readShift < 0)
						{
							++*p;
							readShift = 4;
						}

						u8 mask = 0x0f << shift;
						*d = (*d & (~mask)) | ((color << shift) & mask);

						shift -= 4;
						if (shift < 0)
						{
							shift = 4;
							++d;
						}

					}

					for (i=0; i<readAdditional; ++i)
						++p;
				}
			}
		}
		else
		{
			s32 count = (u8)*p; ++p;
			s32 color1 = (u8)*p; color1 = color1 & 0x0f;
			s32 color2 = (u8)*p; color2 = (color2 >> 4) & 0x0f;
			++p;

			for (s32 i=0; i<count; ++i)
			{
				u8 mask = 0x0f << shift;
				u8 toSet = (shift==0 ? color1 : color2) << shift;
				*d = (*d & (~mask)) | (toSet & mask);

				shift -= 4;
				if (shift < 0)
				{
					shift = 4;
					++d;
				}
			}
		}
	}

	delete [] BmpData;
	BmpData = newBmp;
}



//! creates a surface from the file
CImage* CImageBMP::loadImage(FILE* file)
{
	SBMPHeader header;

	fseek( file, 0, SEEK_SET);
	fread(&header, sizeof(header),1,file);

#ifdef __BIG_ENDIAN__
	header.Id = byteswap(header.Id);
	header.FileSize = byteswap(header.FileSize);
	header.BitmapDataOffset = byteswap(header.BitmapDataOffset);
	header.BitmapHeaderSize = byteswap(header.BitmapHeaderSize);
	header.Width = byteswap(header.Width);
	header.Height = byteswap(header.Height);
	header.Planes = byteswap(header.Planes);
	header.BPP = byteswap(header.BPP);
	header.Compression = byteswap(header.Compression);
	header.BitmapDataSize = byteswap(header.BitmapDataSize);
	header.PixelPerMeterX = byteswap(header.PixelPerMeterX);
	header.PixelPerMeterY = byteswap(header.PixelPerMeterY);
	header.Colors = byteswap(header.Colors);
	header.ImportantColors = byteswap(header.ImportantColors);
#endif

	s32 pitch = 0;

	//! return if the header is false

	if (header.Id != 0x4d42)
		return 0;

	if (header.Compression > 2) // we'll only handle RLE-Compression
	{
//		os::Printer::log("Compression mode not supported.", ELL_ERROR);
//AfxMessageBox("Compression mode not supported.");
		return 0;
	}

	// adjust bitmap data size to dword boundary
	header.BitmapDataSize += (4-(header.BitmapDataSize%4))%4;

	// read palette

	s32 pos = ftell(file);
	s32 paletteSize = (header.BitmapDataOffset - pos) / 4;

	if (paletteSize)
	{
		PaletteData = new s32[paletteSize];
		fread(PaletteData, paletteSize * sizeof(s32),1,file);
#ifdef __BIG_ENDIAN__
		for (int i=0; i<paletteSize; ++i)
			PaletteData[i] = byteswap(PaletteData[i]);
#endif
	}

	// read image data
	if (!header.BitmapDataSize)
	{
		// okay, lets guess the size
		// some tools simply don't set it
		fseek(file, 0, SEEK_END);
		s32 fileSize = ftell(file);
		header.BitmapDataSize = fileSize - header.BitmapDataOffset;
	}

	fseek(file,header.BitmapDataOffset,SEEK_SET);

	f32 t = (header.Width) * (header.BPP / 8.0f);
	s32 widthInBytes = (s32)t;
	t -= widthInBytes;
	if (t!=0.0f)
		++widthInBytes;

	s32 lineData = widthInBytes + ((4-(widthInBytes%4)))%4;
	pitch = lineData - widthInBytes;

	BmpData = new u8[header.BitmapDataSize];
	fread(BmpData, header.BitmapDataSize,1,file);

	// decompress data if needed
	switch(header.Compression)
	{
	case 1: // 8 bit rle
		decompress8BitRLE(BmpData, header.BitmapDataSize, header.Width, header.Height, pitch);
		break;
	case 2: // 4 bit rle
		decompress4BitRLE(BmpData, header.BitmapDataSize, header.Width, header.Height, pitch);
		break;
	}
//CString str;
//str.Format("header.BPP %d",header.BPP);
//AfxMessageBox(str);
	// create surface

	CImage* image = 0;
	switch(header.BPP)
	{
	case 1:// 1: Monochrome bitmap
		image = new CImage(ECF_A1R5G5B5, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert1BitTo16Bit(BmpData, (s16*)image->lock(), header.Width, header.Height, pitch, true);
		break;
	case 4:// 4: 16 color bitmap
		image = new CImage(ECF_A1R5G5B5, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert4BitTo16Bit(BmpData, (s16*)image->lock(), header.Width, header.Height, PaletteData, pitch, true);
		break;
	case 8:// 8: 256 color bitmap
		image = new CImage(ECF_A1R5G5B5, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert8BitTo16Bit(BmpData, (s16*)image->lock(), header.Width, header.Height, PaletteData, pitch, true);
		break;
	case 16:// 16: 16bit (high color) bitmap
		image = new CImage(ECF_A1R5G5B5, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert16BitTo16Bit((s16*)BmpData, (s16*)image->lock(), header.Width, header.Height, pitch, true);
		break;
	case 24:// 24: 24bit (true color) bitmap
		image = new CImage(ECF_R8G8B8, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert24BitTo24Bit(BmpData, (u8*)image->lock(), header.Width, header.Height, pitch, true, true);
		break;
	case 32: // 32: 32bit (true color) bitmap// thx to Reinhard Ostermeier
		image = new CImage(ECF_A8R8G8B8, dimension2di(header.Width, header.Height));
		if (image)
			CColorConverter::convert32BitTo32Bit((s32*)BmpData, (s32*)image->lock(), header.Width, header.Height, pitch, true);
		break;
	};

	if (image)
		image->unlock();

	// clean up
	delete [] PaletteData;
	PaletteData = 0;

	delete [] BmpData;
	BmpData = 0;

	return image;
}

bool CImageBMP::writeImage(FILE* file, CImage* image, u32 param)
{
	// we always write 24-bit color because nothing really reads 32-bit

	SBMPHeader imageHeader;
	imageHeader.Id = 0x4d42;
	imageHeader.Reserved = 0;
	imageHeader.BitmapDataOffset = sizeof(imageHeader);
	imageHeader.BitmapHeaderSize = 0x28;
	imageHeader.Width = image->getDimension().Width;
	imageHeader.Height = image->getDimension().Height;
	imageHeader.Planes = 1;
	imageHeader.BPP = 24;
	imageHeader.Compression = 0;
	imageHeader.PixelPerMeterX = 0;
	imageHeader.PixelPerMeterY = 0;
	imageHeader.Colors = 0;
	imageHeader.ImportantColors = 0;

	// data size is rounded up to next larger 4 bytes boundary
	imageHeader.BitmapDataSize = imageHeader.Width * imageHeader.BPP / 8;
	imageHeader.BitmapDataSize = (imageHeader.BitmapDataSize + 3) & ~3;
	imageHeader.BitmapDataSize *= imageHeader.Height;

	// file size is data size plus offset to data
	imageHeader.FileSize = imageHeader.BitmapDataOffset + imageHeader.BitmapDataSize;

	// bitmaps are stored upside down and padded so we always do this
	void (*CColorConverter_convertFORMATtoFORMAT)(const void*, s32, void*) = 0;
	switch(image->getColorFormat())
	{
	case ECF_R8G8B8:
		CColorConverter_convertFORMATtoFORMAT = CColorConverter::convert_R8G8B8toR8G8B8;
		break;
	case ECF_A8R8G8B8:
		CColorConverter_convertFORMATtoFORMAT = CColorConverter::convert_A8R8G8B8toR8G8B8;
		break;
	case ECF_A1R5G5B5:
		CColorConverter_convertFORMATtoFORMAT = CColorConverter::convert_A1R5G5B5toR8G8B8;
		break;
	case ECF_R5G6B5:
		CColorConverter_convertFORMATtoFORMAT = CColorConverter::convert_R5G6B5toR8G8B8;
		break;
	}

	// couldn't find a color converter
	if (!CColorConverter_convertFORMATtoFORMAT)
		return false;

	// write the bitmap header
	if (fwrite(&imageHeader, sizeof(imageHeader),1,file) != sizeof(imageHeader))
		return false;

	u8* scan_lines = (u8*)image->lock();
	if (!scan_lines)
		return false;

	// size of one pixel in bytes
	u32 pixel_size = image->getBytesPerPixel();

	// length of one row of the source image in bytes
	u32 row_stride = (pixel_size * imageHeader.Width);

	// length of one row in bytes, rounded up to nearest 4-byte boundary
	s32 row_size = ((3 * imageHeader.Width) + 3) & ~3;

	// allocate and clear memory for our scan line
	u8* row_pointer = new u8[row_size];
	memset(row_pointer, 0, row_size);

	// convert the image to 24-bit BGR and flip it over
	s32 y;
	for (y = imageHeader.Height - 1; 0 <= y; --y)
	{
		if (image->getColorFormat()==ECF_R8G8B8)
			CColorConverter::convert24BitTo24Bit(&scan_lines[y * row_stride], row_pointer, imageHeader.Width, 1, 0, false, true);
		else
			// source, length [pixels], destination
			CColorConverter_convertFORMATtoFORMAT(&scan_lines[y * row_stride], imageHeader.Width, row_pointer);
		if ( fwrite(row_pointer, row_size, 1, file) < (u32)row_size)
			break;
	}

	// clean up our scratch area
	delete [] row_pointer;

	// give back image handle
	image->unlock();

	return y < 0;
}


//! creates a loader which is able to load windows bitmaps
//IImageLoader* createImageLoaderBMP()
//{
//	return new CImageBMP;
//}

