#include <stdafx.h>
//#include "CImageBMP.h"
#include "CImagePPM.h"
#include "../core/fast_atof.h"

CImagePPM::CImagePPM()
{
	m_fileSize=0;
}

CImagePPM::~CImagePPM()
{
	// do something?
}

// returns true if the file maybe is able to be loaded by this class
bool CImagePPM::isALoadableFileExtension(const c8* fileName)
{
	return strstr(fileName, ".ppm") || strstr(fileName, ".pgm") || strstr(fileName, ".pbm");
}

//! returns true if the file maybe is able to be loaded by this class
bool CImagePPM::isALoadableFileFormat(FILE* file)
{
	if (!file)		return false;

	c8 id[2];
	fread(&id, 2, 1,file);
	return (id[0]=='P' && id[1]>'0' && id[1]<'7');
}

//! creates a surface from the file
CImage* CImagePPM::loadImage( FILE* file )
{
	CImage* image;

	fseek(file, 0, SEEK_END);
	m_fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (m_fileSize < 12) return 0;

	c8 id[2];
	fread(&id, 2, 1, file);

	if (id[0]!='P' || id[1]<'1' || id[1]>'6')
		return 0;

	const u8 format = id[1] - '0';
	const bool binary = format>3;

	xstringc token;
	getNextToken(file, token);
	const u32 width = strtol10(token.c_str());

	getNextToken(file, token);
	const u32 height = strtol10(token.c_str());

	u8* data = 0;
	const u32 size = width*height;
	if (format==1 || format==4)
	{
		skipToNextToken(file); // go to start of data

		const u32 bytesize = size/8+(size & 3)?1:0;
		if (binary)
		{
			if (m_fileSize-ftell(file) < (long)bytesize)
				return 0;
			data = new u8[bytesize];
			fread(data, bytesize, 1, file);
		}
		else
		{
			if (m_fileSize-ftell(file) < (long)(2*size)) // optimistic test
				return 0;
			data = new u8[bytesize];
			memset(data, 0, bytesize);
			u32 shift=0;
			for (u32 i=0; i<size; ++i)
			{
				getNextToken(file, token);
				if (token == "1")
					data[i/8] |= (0x01 << shift);
				if (++shift == 8)
					shift=0;
			}
		}
		image = new CImage(ECF_A1R5G5B5, dimension2d<s32>(width, height));
		if (image)
			CColorConverter::convert1BitTo16Bit(data, (s16*)image->lock(), width, height);
	}
	else
	{
		getNextToken(file, token);
		const u32 maxDepth = strtol10(token.c_str());
		if (maxDepth > 255) // no double bytes yet
			return 0;

		skipToNextToken(file); // go to start of data

		if (format==2 || format==5)
		{
			if (binary)
			{
				if (m_fileSize-ftell(file) < (long)size)
					return 0;
				data = new u8[size];
				fread(data, size, 1, file);
				image = new CImage(ECF_A8R8G8B8, dimension2d<s32>(width, height));
				if (image)
				{
					u8* ptr = (u8*)image->lock();
					for (u32 i=0; i<size; ++i)
					{
						*ptr++ = data[i];
						*ptr++ = data[i];
						*ptr++ = data[i];
						*ptr++ = 255;
					}
				}
			}
			else
			{
				if (m_fileSize-ftell(file) < (long)(2*size)) // optimistic test
					return 0;
				image = new CImage(ECF_A8R8G8B8, dimension2d<s32>(width, height));
				if (image)
				{
					u8* ptr = (u8*)image->lock();
					for (u32 i=0; i<size; ++i)
					{
						getNextToken(file, token);
						const u32 num = strtol10(token.c_str());
						*ptr++ = num;
						*ptr++ = num;
						*ptr++ = num;
						*ptr++ = 255;
					}
				}
			}
		}
		else
		{
			const u32 bytesize = 3*size;
			if (binary)
			{
				if (m_fileSize-ftell(file) < (long)bytesize)
					return 0;
				data = new u8[bytesize];
				fread(data, bytesize, 1, file);
				image = new CImage(ECF_A8R8G8B8, dimension2d<s32>(width, height));
				if (image)
				{
					u8* ptr = (u8*)image->lock();
					for (u32 i=0; i<size; ++i)
					{
						*ptr++ = data[3*i];
						*ptr++ = data[3*i+1];
						*ptr++ = data[3*i+2];
						*ptr++ = 255;
					}
				}
			}
			else
			{
				if (m_fileSize-ftell(file) < (long)(2*bytesize)) // optimistic test
					return 0;
				image = new CImage(ECF_A8R8G8B8, dimension2d<s32>(width, height));
				if (image)
				{
					u8* ptr = (u8*)image->lock();
					for (u32 i=0; i<size; ++i)
					{
						getNextToken(file, token);
						*ptr++ = strtol10(token.c_str());
						getNextToken(file, token);
						*ptr++ = strtol10(token.c_str());
						getNextToken(file, token);
						*ptr++ = strtol10(token.c_str());
						*ptr++ = 255;
					}
				}
			}
		}
	}

	if (image)
		image->unlock();

	delete [] data;

	return image;
}

bool CImagePPM::writeImage( FILE *file, CImage *image, u32 param) const
{
	char cache[70];
	char size;

	const dimension2d<s32>& imageSize = image->getDimension();

	const bool binary = false;

	if (binary)
		size = _snprintf(cache, 70, "P6\n");
	else
		size = _snprintf(cache, 70, "P3\n");

	if (fwrite(cache, size, 1 ,file ) != size)
		return false;

	size = _snprintf(cache, 70, "%d %d\n", imageSize.Width, imageSize.Height);
	if (fwrite(cache, size, 1 ,file) != size)
		return false;

	size = _snprintf(cache, 70, "255\n");
	if (fwrite(cache, size, 1 ,file) != size)
		return false;

	if (binary)
	{
		for (s32 h = 0; h < imageSize.Height; ++h)
		{
			for (s32 c = 0; c < imageSize.Width; ++c)
			{
				const SColor& pixel = image->getPixel(c, h);
				const u8 r = pixel.getRed() & 0xff;
				const u8 g = pixel.getGreen() & 0xff;
				const u8 b = pixel.getBlue() & 0xff;
				fwrite(&r, 1, 1 ,file);
				fwrite(&g, 1, 1 ,file);
				fwrite(&b, 1, 1 ,file);
			}
		}
	}
	else
	{
		s32 n = 0;

		for (s32 h = 0; h < imageSize.Height; ++h)
		{
			for (s32 c = 0; c < imageSize.Width; ++c, ++n)
			{
				const SColor& pixel = image->getPixel(c, h);
				size = _snprintf(cache, 70, "%.3u %.3u %.3u%s", pixel.getRed(), pixel.getGreen(), pixel.getBlue(), n % 5 == 4 ? "\n" : "  ");	
				if (fwrite(cache, size, 1 ,file) != size)
					return false;
			}
		}
	}

	return true;
}






//! read the next token from file
void CImagePPM::getNextToken(FILE* file, xstringc& token) const
{
	token = "";
	c8 c;
	while(ftell(file)<m_fileSize)
	{
		fread(&c, 1, 1, file);
		if (c=='#')
		{
			while (c!='\n' && c!='\r' && (ftell(file)<m_fileSize))
				fread(&c, 1, 1, file);
		}
		else if (!isspace(c))
		{
			token.append(c);
			break;
		}
	}
	while(ftell(file)<m_fileSize)
	{
		fread(&c, 1, 1, file);
		if (c=='#')
		{
			while (c!='\n' && c!='\r' && (ftell(file)<m_fileSize))
				fread(&c, 1, 1, file);
		}
		else if (!isspace(c))
			token.append(c);
		else
			break;
	}
}


//! skip to next token (skip whitespace)
void CImagePPM::skipToNextToken( FILE* file ) const
{
	c8 c;
	while(ftell(file)<m_fileSize)
	{
		fread(&c, 1, 1, file);
		if (c=='#')
		{
			while (c!='\n' && c!='\r' && (ftell(file)<m_fileSize))
				fread(&c, 1, 1, file);
		}
		else if (!isspace(c))
		{
			fseek(file, -1,  SEEK_CUR );// put back
			break;
		}
	}
}

