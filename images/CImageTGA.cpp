#include <stdafx.h>
#include "CImageTGA.h"
#include "CColorConverter.h"

//! constructor
CImageTGA::CImageTGA()
{
}

//! destructor
CImageTGA::~CImageTGA()
{
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageTGA::isALoadableFileExtension(const c8* fileName)
{
	return strstr(fileName, ".tga") != 0;
}


//! loads a compressed tga.
u8 *CImageTGA::loadCompressedImage(FILE *file, const STGAHeader& header)
{
	// This was written and sent in by Jon Pry, thank you very much!
	// I only changed the formatting a little bit.

	s32 bytesPerPixel = header.PixelDepth/8;
	s32 imageSize =  header.ImageHeight * header.ImageWidth * bytesPerPixel;
	u8* data = new u8[imageSize];
	s32 currentByte = 0;

	while(currentByte < imageSize)
	{
		u8 chunkheader = 0;
		fread(&chunkheader, sizeof(u8), 1, file); // Read The Chunk's Header

		if(chunkheader < 128) // If The Chunk Is A 'RAW' Chunk
		{
			chunkheader++; // Add 1 To The Value To Get Total Number Of Raw Pixels

			fread(&data[currentByte], bytesPerPixel * chunkheader, 1, file);
			currentByte += bytesPerPixel * chunkheader;
		}
		else
		{
			// thnx to neojzs for some fixes with this code

			// If It's An RLE Header
			chunkheader -= 127; // Subtract 127 To Get Rid Of The ID Bit

			s32 dataOffset = currentByte;
			fread(&data[dataOffset], bytesPerPixel, 1, file);

			currentByte += bytesPerPixel;

			for(s32 counter = 1; counter < chunkheader; counter++)
			{
				for(s32 elementCounter=0; elementCounter < bytesPerPixel; elementCounter++)
					data[currentByte + elementCounter] = data[dataOffset + elementCounter];

				currentByte += bytesPerPixel;
			}
		}
	}

	return data;
}



//! returns true if the file maybe is able to be loaded by this class
bool CImageTGA::isALoadableFileFormat(FILE* file)
{
	if (!file)
		return false;

	return true;
}



//! creates a surface from the file
CImage* CImageTGA::loadImage(FILE* file)
{
	STGAHeader header;
	u8* colorMap = 0;

	fseek( file, 0, SEEK_SET);
	fread(&header, sizeof(STGAHeader), 1, file);

#ifdef __BIG_ENDIAN__
	header.ColorMapLength = byteswap(header.ColorMapLength);
	header.ImageWidth = byteswap(header.ImageWidth);
	header.ImageHeight = byteswap(header.ImageHeight);
#endif

	// skip image identification field
	if (header.IdLength)
		fseek(file, header.IdLength, SEEK_CUR);

	if (header.ColorMapType)
	{
		// read color map
		colorMap = new u8[header.ColorMapEntrySize/8 * header.ColorMapLength];
		fread(colorMap,header.ColorMapEntrySize/8 * header.ColorMapLength, 1, file);
	}

	// read image
	u8* data = 0;

	if (header.ImageType == 2)
	{
		const s32 imageSize = header.ImageHeight * header.ImageWidth * header.PixelDepth/8;
		data = new u8[imageSize];
	  	fread(data, imageSize, 1, file);
	}
	else if(header.ImageType == 10)
		data = loadCompressedImage(file, header);
	else
	{
//		os::Printer::log("Unsupported TGA file type", file->getFileName(), ELL_ERROR);
		if (colorMap)
			delete [] colorMap;
		return 0;
	}

	CImage* image = 0;

	switch(header.PixelDepth)
	{
	case 16:
		{
			image = new CImage(ECF_A1R5G5B5,
				dimension2di(header.ImageWidth, header.ImageHeight));
			if (image)
				CColorConverter::convert16BitTo16Bit((s16*)data,
					(s16*)image->lock(), header.ImageWidth,	header.ImageHeight, 0, (header.ImageDescriptor&0x20)==0);
		}
		break;
	case 24:
		{
			image = new CImage(ECF_R8G8B8,
				dimension2di(header.ImageWidth, header.ImageHeight));
			if (image)
				CColorConverter::convert24BitTo24Bit(
					(u8*)data, (u8*)image->lock(), header.ImageWidth, header.ImageHeight, 0, (header.ImageDescriptor&0x20)==0, true);
		}
		break;
	case 32:
		{
			image = new CImage(ECF_A8R8G8B8,
				dimension2di(header.ImageWidth, header.ImageHeight));
			if (image)
				CColorConverter::convert32BitTo32Bit((s32*)data,
					(s32*)image->lock(), header.ImageWidth, header.ImageHeight, 0, (header.ImageDescriptor&0x20)==0);
		}
		break;
	default:
//		os::Printer::log("Unsupported TGA format", file->getFileName(), ELL_ERROR);
		break;
	}
	if (image)
		image->unlock();
	delete [] data;
	if (colorMap)
		delete [] colorMap;

	return image;
}


bool CImageTGA::writeImage(FILE *file, CImage *image,u32 param)
{
	STGAHeader imageHeader;
	imageHeader.IdLength = 0;
	imageHeader.ColorMapType = 0;
	imageHeader.ImageType = 2;
	imageHeader.FirstEntryIndex[0] = 0;
	imageHeader.FirstEntryIndex[1] = 0;
	imageHeader.ColorMapLength = 0;
	imageHeader.ColorMapEntrySize = 0;
	imageHeader.XOrigin[0] = 0;
	imageHeader.XOrigin[1] = 0;
	imageHeader.YOrigin[0] = 0;
	imageHeader.YOrigin[1] = 0;
	imageHeader.ImageWidth = image->getDimension().Width;
	imageHeader.ImageHeight = image->getDimension().Height;

	// top left of image is the top. the image loader needs to
	// be fixed to only swap/flip
	imageHeader.ImageDescriptor = (1 << 5);

   // chances are good we'll need to swizzle data, so i'm going
	// to convert and write one scan line at a time. it's also
	// a bit cleaner this way
	void (*CColorConverter_convertFORMATtoFORMAT)(const void*, s32, void*) = 0;
	switch(image->getColorFormat())
	{
	case ECF_A8R8G8B8:
		CColorConverter_convertFORMATtoFORMAT
			= CColorConverter::convert_A8R8G8B8toA8R8G8B8;
		imageHeader.PixelDepth = 32;
		imageHeader.ImageDescriptor |= 8;
		break;
	case ECF_A1R5G5B5:
		CColorConverter_convertFORMATtoFORMAT
			= CColorConverter::convert_A1R5G5B5toA1R5G5B5;
		imageHeader.PixelDepth = 16;
		imageHeader.ImageDescriptor |= 1;
		break;
	case ECF_R5G6B5:
		CColorConverter_convertFORMATtoFORMAT
			= CColorConverter::convert_R5G6B5toA1R5G5B5;
		imageHeader.PixelDepth = 16;
		imageHeader.ImageDescriptor |= 1;
		break;
	case ECF_R8G8B8:
		CColorConverter_convertFORMATtoFORMAT
			= CColorConverter::convert_R8G8B8toR8G8B8;
		imageHeader.PixelDepth = 24;
		imageHeader.ImageDescriptor |= 0;
		break;
	}

	// couldn't find a color converter
	if (!CColorConverter_convertFORMATtoFORMAT)
		return false;

	if (fwrite(&imageHeader, sizeof(imageHeader), 1, file) != sizeof(imageHeader))
		return false;

	u8* scan_lines = (u8*)image->lock();
	if (!scan_lines)
		return false;

	// size of one pixel in bytes
	u32 pixel_size = image->getBytesPerPixel();

	// length of one row of the source image in bytes
	u32 row_stride = (pixel_size * imageHeader.ImageWidth);

	// length of one output row in bytes
	s32 row_size = ((imageHeader.PixelDepth / 8) * imageHeader.ImageWidth);

	// allocate a row do translate data into
	u8* row_pointer = new u8[row_size];

	u32 y;
	for (y = 0; y < imageHeader.ImageHeight; ++y)
	{
		// source, length [pixels], destination
		if (image->getColorFormat()==ECF_R8G8B8)
			CColorConverter::convert24BitTo24Bit(&scan_lines[y * row_stride], row_pointer, imageHeader.ImageWidth, 1, 0, 0, true);
		else
			CColorConverter_convertFORMATtoFORMAT(&scan_lines[y * row_stride], imageHeader.ImageWidth, row_pointer);
		if (fwrite(row_pointer, row_size, 1, file) != (u32)row_size)
			break;
	}

	delete [] row_pointer;

	image->unlock();

	STGAFooter imageFooter;
	imageFooter.ExtensionOffset = 0;
	imageFooter.DeveloperOffset = 0;
	strncpy(imageFooter.Signature, "TRUEVISION-XFILE.", 18);

	if (fwrite(&imageFooter, sizeof(imageFooter), 1, file) < (s32)sizeof(imageFooter))
		return false;

	return imageHeader.ImageHeight < y;
}


///////////////////////////////////////////////////////////////////////////
////Tga2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool CImageTGA::tga2bmp(char* tganame,char* bmpname)
{
    FILE* file = fopen( tganame, "rb" );
    if( NULL == file )
        return false;

    struct TargaHeader
    {
        BYTE IDLength;
        BYTE ColormapType;
        BYTE ImageType;
        BYTE ColormapSpecification[5];
        WORD XOrigin;
        WORD YOrigin;
        WORD ImageWidth;
        WORD ImageHeight;
        BYTE PixelDepth;
        BYTE ImageDescriptor;
    } tga;

    fread( &tga, sizeof(TargaHeader), 1, file );

    // Only true color, non-mapped images are supported
    if( ( 0 != tga.ColormapType ) || 
        ( tga.ImageType != 10 && tga.ImageType != 2 ) )
    {
        fclose( file );
        return false;
    }

    // Skip the ID field. The first byte of the header is the length of this field
    if( tga.IDLength )
        fseek( file, tga.IDLength, SEEK_CUR );

    int m_dwWidth   = tga.ImageWidth;
    int m_dwHeight  = tga.ImageHeight;
    int m_dwBPP     = tga.PixelDepth;

	
	DWORD* m_pRGBAData;
    m_pRGBAData = new DWORD[m_dwWidth*m_dwHeight];

    if( m_pRGBAData == NULL )
    {
        fclose(file);
        return false;
    }

	int x,y;
    for( y=0; y<m_dwHeight; y++ )
    {
        DWORD dwOffset = y*m_dwWidth;

        if( 0 == ( tga.ImageDescriptor & 0x0010 ) )
            dwOffset = (m_dwHeight-y-1)*m_dwWidth;

        for( x=0; x<m_dwWidth; x )
        {
            if( tga.ImageType == 10 )
            {
                BYTE PacketInfo = getc( file );
                WORD PacketType = 0x80 & PacketInfo;
                WORD PixelCount = ( 0x007f & PacketInfo ) + 1;

                if( PacketType )
                {
                    DWORD b = getc( file );
                    DWORD g = getc( file );
                    DWORD r = getc( file );
                    DWORD a = 0xff;
                    if( m_dwBPP == 32 )
                        a = getc( file );

                    while( PixelCount-- )
                    {
                        m_pRGBAData[dwOffset+x] = (r<<24L)+(g<<16L)+(b<<8L)+(a);
                        x++;
                    }
                }
                else
                {
                    while( PixelCount-- )
                    {
                        BYTE b = getc( file );
                        BYTE g = getc( file );
                        BYTE r = getc( file );
                        BYTE a = 0xff;
                        if( m_dwBPP == 32 )
                            a = getc( file );

                        m_pRGBAData[dwOffset+x] = (r<<24L)+(g<<16L)+(b<<8L)+(a);
                        x++;
                    }
                }
            }
            else
            {
                BYTE b = getc( file );
                BYTE g = getc( file );
                BYTE r = getc( file );
                BYTE a = 0xff;
                if( m_dwBPP == 32 )
                    a = getc( file );

                m_pRGBAData[dwOffset+x] = (r<<24L)+(g<<16L)+(b<<8L)+(a);
                x++;
            }
        }
    }

    fclose( file );

	BYTE *pBytes ;
	pBytes = new BYTE[m_dwWidth*m_dwHeight*3];
	int pos = 0;
    for( y=0; y<m_dwHeight; y++ )
    {
        for( x=0; x<m_dwWidth; x++ )
        {
            DWORD dwPixel = m_pRGBAData[y*m_dwWidth+x];
            BYTE r = (BYTE)((dwPixel>>24)&0x000000ff); pBytes[pos++] = r;
            BYTE g = (BYTE)((dwPixel>>16)&0x000000ff); pBytes[pos++] = g;
            BYTE b = (BYTE)((dwPixel>> 8)&0x000000ff); pBytes[pos++] = b;
        }
    }


    LPBITMAPINFO pBmpInfo;
	pBmpInfo = (LPBITMAPINFO) new char[sizeof(BITMAPINFO)];

	//bmiHeader의 구조체 크기 설정
	pBmpInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth=m_dwWidth;
	pBmpInfo->bmiHeader.biHeight=m_dwHeight;
	pBmpInfo->bmiHeader.biPlanes=1;
	pBmpInfo->bmiHeader.biBitCount=24;//한 픽셀길이가 24 bit 
	//가로길이 4배수 나머지 값 얻기
	int m_ndumy=(m_dwWidth*3)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	pBmpInfo->bmiHeader.biSizeImage=((m_dwWidth*3)+m_ndumy)*m_dwHeight;

	pBmpInfo->bmiHeader.biCompression=BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter=0;
	pBmpInfo->bmiHeader.biYPelsPerMeter=0;
	pBmpInfo->bmiHeader.biClrUsed =0;
	pBmpInfo->bmiHeader.biClrImportant =0;

 //CString test;
//test.Format("%s %d %d",jpgname,bx,by);
//AfxMessageBox(test);

 	BITMAPFILEHEADER bmpHeader;
	DWORD size,len;
	bmpHeader.bfType=DIB_HEADER_MARKER;
	bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize=sizeof(bmpHeader)-sizeof(RGBQUAD)+sizeof(BITMAPINFO)+pBmpInfo->bmiHeader.biSizeImage;
	bmpHeader.bfReserved1=0;
	bmpHeader.bfReserved2=0;

	HANDLE fd=CreateFile(bmpname,GENERIC_READ|GENERIC_WRITE,
			              0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(!WriteFile(fd,(LPSTR)&bmpHeader,sizeof(bmpHeader),&len,NULL))
		return false;
	size = sizeof(BITMAPINFO)-sizeof(RGBQUAD);
	if(!WriteFile(fd,pBmpInfo,size,&len,NULL))
		return false;

//	FlipY(pBytes,CSize(m_dwWidth,m_dwHeight),3);//이미지를 뒤집는다..

	if(!WriteFile(fd,pBytes,pBmpInfo->bmiHeader.biSizeImage,&len,NULL))
		return false;
	CloseHandle(fd);

	delete [] pBytes;

    return true;
}

