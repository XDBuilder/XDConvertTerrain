
#include <stdafx.h>
#include "../core/scolor.h"
#include "CColorConverter.h"
#include "CImagePCX.h"

CImagePCX::CImagePCX() : PCXData(0), PaletteData(0)
{

}

CImagePCX::~CImagePCX()
{
	if (PaletteData)
		delete [] PaletteData;

	if (PCXData)
		delete [] PCXData;
}



// returns true if the file maybe is able to be loaded by this class
// based on the file extension (e.g. ".tga")
bool CImagePCX::isALoadableFileExtension(const c8* fileName)
{
	return (strstr(fileName, ".PCX") != 0) || (strstr(fileName, ".pcx") != 0);
}


// returns true if the file maybe is able to be loaded by this class
bool CImagePCX::isALoadableFileFormat(FILE* file)
{
	u8 headerID;
	fread(&headerID, sizeof(headerID), 1, file );
	return headerID == 0x0a;
}


//! creates a image from the file
CImage* CImagePCX::loadImage(FILE* file)
{
	SPCXHeader header;

	fread(&header, sizeof(header), 1, file);
	#ifdef __BIG_ENDIAN__
		header.XMin = byteswap(header.XMin);
		header.YMin = byteswap(header.YMin);
		header.XMax = byteswap(header.XMax);
		header.YMax = byteswap(header.YMax);
		header.HorizDPI = byteswap(header.HorizDPI);
		header.VertDPI = byteswap(header.VertDPI);
		header.BytesPerLine = byteswap(header.BytesPerLine);
		header.PaletteType = byteswap(header.PaletteType);
		header.HScrSize = byteswap(header.HScrSize);
		header.VScrSize = byteswap(header.VScrSize);
	#endif

	//! return if the header is wrong
	if (header.Manufacturer != 0x0a && header.Encoding != 0x01)
		return 0;

	// return if this isn't a supported type
	if( (header.BitsPerPixel != 8) && (header.BitsPerPixel != 24) )
	{
//		os::Printer::log("Unsupported bits per pixel in PCX file.",	file->getFileName(), ELL_WARNING);
		return 0;
	}

	// read palette
	if( header.BitsPerPixel == 8 )
	{
		// the palette indicator (usually a 0x0c is found infront of the actual palette data)
		// is ignored because some exporters seem to forget to write it. This would result in
		// no image loaded before, now only wrong colors will be set.
		s32 pos = ftell(file);
		fseek(file, pos-256*3, SEEK_SET );

		u8 *tempPalette = new u8[768];
		PaletteData = new s32[256];
		memset(PaletteData, 0, 256*sizeof(s32));
		fread( tempPalette, 768, 1, file);

		for( s32 i=0; i<256; i++ )
		{
			PaletteData[i] = (tempPalette[i*3+0] << 16) |
					 (tempPalette[i*3+1] << 8) | 
					 (tempPalette[i*3+2] );
		}

		delete [] tempPalette;

		fseek( file, pos, SEEK_SET );
	}
	else if( header.BitsPerPixel == 4 )
	{
		PaletteData = new s32[16];
		memset(PaletteData, 0, 16*sizeof(s32));
		for( s32 i=0; i<256; i++ )
		{
			PaletteData[i] = (header.Palette[i*3+0] << 16) |
					 (header.Palette[i*3+1] << 8) | 
					 (header.Palette[i*3+2]);
		}
	}

	// read image data
	s32 width, height, imagebytes;
	width = header.XMax - header.XMin + 1;
	height = header.YMax - header.YMin + 1;
	imagebytes = header.BytesPerLine * height * header.Planes * header.BitsPerPixel / 8;
	PCXData = new u8[imagebytes];

	u8 cnt, value;
	for( s32 offset = 0; offset < imagebytes; )
	{
		fread( &cnt, 1 , 1, file);
		if( !((cnt & 0xc0) == 0xc0) )
		{
			value = cnt;
			cnt = 1;
		}
		else
		{
			cnt &= 0x3f;
			fread( &value, 1, 1, file);
		}
		memset(PCXData+offset, value, cnt);
		offset += cnt;
	}

	// create image
	CImage* image = 0;
	s32 pitch = header.BytesPerLine - width * header.Planes * header.BitsPerPixel / 8;

	if (pitch < 0)
		pitch = -pitch;

	switch(header.BitsPerPixel) // TODO: Other formats
	{
	case 8:
		image = new CImage(ECF_A1R5G5B5, dimension2di(width, height));
		if (image)
			CColorConverter::convert8BitTo16Bit(PCXData, (s16*)image->lock(), width, height, PaletteData, pitch);
		break;
	case 24:
		image = new CImage(ECF_R8G8B8, dimension2di(width, height));
		if (image)
			CColorConverter::convert24BitTo24Bit(PCXData, (u8*)image->lock(), width, height, pitch);
		break;
	};
	if (image)
		image->unlock();

	// clean up

	if( PaletteData )
		delete [] PaletteData;
	PaletteData = 0;

	if( PCXData )
		delete [] PCXData;
	PCXData = 0;

	return image;
}



void flipY( BYTE* image , int cx, int cy, int width)
{
	//한 스캔라인에 소요된 실제 바이트수 4의 배수를 구합니다.
	int RealWidth = (cx * width + 3)/4*4;
	//이미지의 높이를 구합니다.
	int Height = cy;	
	//Flip을 하기 위해서 필요한 임지저장 장소에 메모리를 할당합니다.
	BYTE *pLine = new BYTE[RealWidth * Height];
	for( int i=0; i<Height/2; i++)
	{	//윗부분의 한 스캔라인을 임시저장 장소에 카피고,
		memcpy(pLine, &image[i * RealWidth], RealWidth);
		//밑부분의 한 스캔라인을 윗부분의 스캔라인 위치고 카피하고,
		memcpy(&image[i * RealWidth], &image[(Height - 1 - i) * RealWidth], RealWidth);
		//임시저장장소에 있던 내용을 밑부분의 한 스캔라인으로 카피합니다.
		memcpy(&image[(Height - 1 - i) * RealWidth], pLine, RealWidth);
	}
	//임시 저장장소를 해제합니다.
	delete [] pLine;
}

///////////////////////////////////////////////////////////////////////////
////Pcx2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool CImagePCX::pcx2bmp(char* pcxname,char* bmpname)
{
	// Standard PCX header
	struct PCXHead {
		char   ID;
		char   Version;
		char   Encoding;
		char   BitPerPixel;
		short  X1;
		short  Y1;
		short  X2;
		short  Y2;
		short  HRes;
		short  VRes;
		char   ClrMap[16*3];
		char   Reserved1;
		char   NumPlanes;
		short  BPL;
		short  Pal_t;
		char   Filler[58];
	} sHeader;

	CSize			BitmapSize;
	LPBITMAPINFO	pBmpInfo;
	BYTE           *pabRawBitmap;

	// Open the file and put its entire content in memory
	FILE *pFile = fopen( pcxname, "rb" );
	if ( !pFile )
	{
		AfxMessageBox("Unable to open the PCX file");
		return FALSE;
	}
	fseek( pFile ,0 ,SEEK_END );
	const long clFileSize = ftell(pFile);
	BYTE *pabFileData = (BYTE *)new BYTE[ clFileSize ];
	fseek( pFile ,0 ,SEEK_SET );
	fread( pabFileData, clFileSize, 1, pFile );
	fclose( pFile );

	// Get the header
	memcpy( &sHeader, pabFileData, sizeof(sHeader) );

	// Each scan line MUST have a size that can be divided by a 'long' data type
	int iScanLineSize = sHeader.NumPlanes * sHeader.BPL;
	ldiv_t sDivResult = ldiv( iScanLineSize, sizeof(long) );
	if ( sDivResult.rem > 0 )
		iScanLineSize = (iScanLineSize/sizeof(long)+1) * sizeof(long);

	// Set the bitmap size data member
	BitmapSize = CSize( sHeader.X2-sHeader.X1+1, sHeader.Y2-sHeader.Y1+1 );
	const long clImageSize = iScanLineSize * BitmapSize.cy;

	// Set the bitmap information
	pBmpInfo = (LPBITMAPINFO) new char[sizeof(BITMAPINFO)+(sizeof(RGBQUAD)*256)];
	pBmpInfo->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth          = BitmapSize.cx;
	pBmpInfo->bmiHeader.biHeight         = BitmapSize.cy;
	pBmpInfo->bmiHeader.biPlanes         = sHeader.NumPlanes;
	pBmpInfo->bmiHeader.biBitCount       = sHeader.BitPerPixel;
	pBmpInfo->bmiHeader.biCompression    = BI_RGB;
	pBmpInfo->bmiHeader.biSizeImage      = 0;
	pBmpInfo->bmiHeader.biXPelsPerMeter  = 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter  = 0;
	pBmpInfo->bmiHeader.biClrUsed        = 0;
	pBmpInfo->bmiHeader.biClrImportant   = 0;

   // Prepare a buffer large enough to hold the image
   pabRawBitmap = (BYTE *)new BYTE[ clImageSize ];
   if ( !pabRawBitmap )
   {
      AfxMessageBox( "Can't allocate memory for the image" );
      delete [] pabFileData;
      return FALSE;
   }

   // Get the compressed image
   long lDataPos = 0;
   long lPos = 128;     // That's where the data begins

   int iX,iY;
   for ( iY=0; iY < BitmapSize.cy; iY++ )
   {
      // Decompress the scan line
      for ( iX=0; iX < sHeader.BPL; )
      {
         UINT uiValue = pabFileData[lPos++];
         if ( uiValue > 192 ) {  // Two high bits are set = Repeat
            uiValue -= 192;                  // Repeat how many times?
            BYTE Color = pabFileData[lPos++];  // What color?

            if ( iX <= BitmapSize.cx )
            {  // Image data.  Place in the raw bitmap.
               for ( BYTE bRepeat=0; bRepeat < uiValue; bRepeat++ )
               {
                  pabRawBitmap[lDataPos++] = Color;
                  iX++;
               }
            }
            else
               iX += uiValue; // Outside the image.  Skip.
         }
         else
         {
            if ( iX <= BitmapSize.cx )
               pabRawBitmap[lDataPos++] = uiValue;
            iX++;
         }
      }

      // Pad the rest with zeros
      if ( iX < iScanLineSize )
      {
         for ( ;iX < iScanLineSize; iX++ )
            pabRawBitmap[lDataPos++] = 0;
      }
   }

   if ( pabFileData[lPos++] == 12 )          // Simple validation
      // Get the palette
      for ( short Entry=0; Entry < 256; Entry++ )
      {
         pBmpInfo->bmiColors[Entry].rgbRed       = pabFileData[lPos++];
         pBmpInfo->bmiColors[Entry].rgbGreen     = pabFileData[lPos++];
         pBmpInfo->bmiColors[Entry].rgbBlue      = pabFileData[lPos++];
         pBmpInfo->bmiColors[Entry].rgbReserved  = 0;
      }
CString test;
//test.Format("%d %d %d %d",BitmapSize.cx, BitmapSize.cy,sHeader.NumPlanes,sHeader.BitPerPixel);
//AfxMessageBox(test);
	int m_ndumy=(BitmapSize.cx)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	pBmpInfo->bmiHeader.biSizeImage=(BitmapSize.cx+m_ndumy)*BitmapSize.cy;

	DWORD size,len;
	BITMAPFILEHEADER bmpHeader;
	bmpHeader.bfType=DIB_HEADER_MARKER;
	bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+((256) * sizeof(RGBQUAD));
	bmpHeader.bfSize=sizeof(bmpHeader)+((255) * sizeof(RGBQUAD))
			+sizeof(BITMAPINFO)+pBmpInfo->bmiHeader.biSizeImage;
	bmpHeader.bfReserved1=0;
	bmpHeader.bfReserved2=0;
	HANDLE fd=CreateFile(bmpname,GENERIC_READ|GENERIC_WRITE,
			0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(!WriteFile(fd,(LPSTR)&bmpHeader,sizeof(bmpHeader),&len,NULL))
		return FALSE;
	size = ((255) * sizeof(RGBQUAD))+sizeof(BITMAPINFO);
	if(!WriteFile(fd,pBmpInfo,size,&len,NULL))
		return FALSE;

	flipY(pabRawBitmap,BitmapSize.cx,BitmapSize.cy,1);//이미지를 뒤집는다.

	if(!WriteFile(fd,pabRawBitmap,pBmpInfo->bmiHeader.biSizeImage,&len,NULL))
		return FALSE;

	CloseHandle(fd);

    delete [] pabFileData;
	delete [] pabRawBitmap;
	delete pBmpInfo;
	return TRUE;

}
