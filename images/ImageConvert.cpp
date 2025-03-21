// ImageConvert.cpp: implementation of the CImageConvert class.
//
//////////////////////////////////////////////////////////////////////

#include "ImageConvert.h"
#include "CImageBMP.h"
#include "CImagePNG.h"
#include <io.h>            // _filelength()

bool isLoadableImageFile(char* imgname)
{
	TCHAR* strExtension = _tcsrchr( imgname, _T('.'));
	if( lstrcmpi( strExtension, _T(".bmp") ) &&
		lstrcmpi( strExtension, _T(".png") )  ) return FALSE;
	
	return TRUE;
}

HBITMAP GetImgHandle(char* imgname)
{
	HBITMAP hBmp;
	char* tmpBmp = "C:\\temp.bmp";
	BOOL success = FALSE;

	TCHAR* strExtension = _tcsrchr( imgname, _T('.'));
	// Load bitmap files
	if( !lstrcmpi( strExtension, _T(".bmp") ) ){
		hBmp = (HBITMAP)LoadImage(NULL,imgname,IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		return hBmp;
	}
	// Load jpeg files
	if( !lstrcmpi( strExtension, _T(".jpg") ) ){
		CImageJPG ijpg;
		success = ijpg.jpg2bmp(imgname,tmpBmp);
	}

	// Load gif files
	if( !lstrcmpi( strExtension, _T(".gif") ) ){
		CImageGIF igif;
		success = igif.gif2bmp(imgname,tmpBmp);
	}

	// Load pcx files
	if( !lstrcmpi( strExtension, _T(".pcx") ) ){
		CImagePCX ipcx;
		success = ipcx.pcx2bmp(imgname,tmpBmp);
	}

	if( !lstrcmpi( strExtension, _T(".tga") ) ){
		CImageTGA itga;
		success = itga.tga2bmp(imgname,tmpBmp);
	}

	if(success){
		hBmp = (HBITMAP)LoadImage(NULL,tmpBmp,IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		DeleteFile(tmpBmp);
		return hBmp;
	}else 
		return NULL;

}

CImage* GetImagePtr( char* imgname )
{
	CImage* image = NULL;

	FILE* fp=fopen(imgname,"rb");

	TCHAR* strExtension = _tcsrchr( imgname, _T('.'));
	// Load bitmap files
	if( !lstrcmpi( strExtension, _T(".bmp") ) ){
		CImageBMP iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(fp);
	}
	// Load jpeg files
	if( !lstrcmpi( strExtension, _T(".jpg") ) ){
		CImageJPG iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(fp);
	}

	// Load gif files
	if( !lstrcmpi( strExtension, _T(".gif") ) ){
		CImageGIF iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(imgname);
	}

	// Load pcx files
	if( !lstrcmpi( strExtension, _T(".pcx") ) ){
		CImagePCX iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(fp);
	}

	if( !lstrcmpi( strExtension, _T(".tga") ) ){
		CImageTGA iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(fp);
	}

	if( !lstrcmpi( strExtension, _T(".png") ) ){
		CImagePNG iimg;
//		AfxMessageBox("Getpng");
		if(iimg.isALoadableFileFormat(fp)) {
//			AfxMessageBox("GetImagePtr");
			image = iimg.loadImage(fp);
		}
	}

	if( !lstrcmpi( strExtension, _T(".psd") ) ){
		CImagePSD iimg;
		if(iimg.isALoadableFileFormat(fp))
			image = iimg.loadImage(fp);
	}

	fclose(fp);

	return image;

}


bool ChangeSaveImageFormat( char* inputname, char* outputname )
{

	TCHAR* strExtension = _tcsrchr( inputname, _T('.'));
	TCHAR* stroutExt = _tcsrchr( outputname, _T('.'));

	// 확장자가 같으면 나간다.
	if( !lstrcmpi( strExtension, stroutExt ) ) return false;

	CImage* image = GetImagePtr( inputname );

	if(image==NULL) return false;

	bool success = false;

	FILE* fp=fopen(outputname,"wb");

	u32 param = 0;
	// Load bitmap files
	if( !lstrcmpi( stroutExt, _T(".bmp") ) ){
		CImageBMP iimg;
		success = iimg.writeImage( fp, image, param );
	}
	// Load jpeg files
	if( !lstrcmpi( stroutExt, _T(".jpg") ) ){
		CImageJPG iimg;
		success = iimg.writeImage( fp, image, param );
	}

	// Load gif files
	if( !lstrcmpi( stroutExt, _T(".gif") ) ){
		CImageGIF iimg;
//		success = iimg.writeImage( fp, image, param );
	}

	// Load pcx files
	if( !lstrcmpi( stroutExt, _T(".pcx") ) ){
		CImagePCX iimg;
//		success = iimg.writeImage( fp, image, param );
	}

	if( !lstrcmpi( stroutExt, _T(".tga") ) ){
		CImageTGA iimg;
		success = iimg.writeImage( fp, image, param );
	}

	if( !lstrcmpi( stroutExt, _T(".png") ) ){
		CImagePNG iimg;
		success = iimg.writeImage( fp, image, param );
	}

	if( !lstrcmpi( stroutExt, _T(".psd") ) ){
		CImagePSD iimg;
//		success = iimg.writeImage( fp, image, param );
	}

	fclose(fp);

	return success;

}

BOOL ImageFileYN(LPCTSTR imgname)
{
	TCHAR* strExtension = _tcsrchr( imgname, _T('.'));
	if( lstrcmpi( strExtension, _T(".bmp") ) &&
		lstrcmpi( strExtension, _T(".jpg") ) &&
		lstrcmpi( strExtension, _T(".pcx") ) &&
		lstrcmpi( strExtension, _T(".tga") ) &&
		lstrcmpi( strExtension, _T(".gif") ) ) return FALSE;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
////Jpg2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BOOL Jpg2Bmp(LPCTSTR jpgname,LPCTSTR bmpname)
{
	HANDLE Heap;
	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);								// init JPEG decompressing engine
	p.JPGFile=(char*)jpgname;								// what's the filename ?
	ijlRead (&p,IJL_JFILE_READPARAMS);	// read JPEG header
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth;
	p.DIBHeight=p.JPGHeight;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
	// now I must pass a buffer large enough to the decompressor
	p.DIBBytes=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	
	ijlRead (&p,IJL_JFILE_READWHOLEIMAGE);	// finally, this reads the image data and decompresses it to my buffer

	int bx,by;		    // bitmap dimensions (x=width, y=height), in pixels
	int bl;				// the length of one scan line, in bytes
						// it is equal to 3*x rounded up to nearest number divisible by 4
						// so l can be computed as l=3*(x+1)&(~3)
						// next_scan_line_adress = previous_scan_line_adress + l (bytes)
	BYTE *bb;		    // pointer to color data
						// data are stored row by row, top to bottom
						// each row occupies l bytes
						// pixels in row are stored left to right
						// each pixel occupies 3 bytes (B,G,R)
						// values of bytes mean R,G,B intensity (0=black, 255=highest intensity)

	bx=p.DIBWidth;			// filling up the DIB structure, as described in typedef section
	by=p.DIBHeight;
	bl=3*(bx+1)&(~3);
	bb=p.DIBBytes;
	ijlFree (&p);				// close the JPEG engine

	LPBITMAPINFO pBmpInfo;
	pBmpInfo = (LPBITMAPINFO) new char[sizeof(BITMAPINFO)];

	//bmiHeader의 구조체 크기 설정
	pBmpInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth=bx;
	pBmpInfo->bmiHeader.biHeight=by;
	pBmpInfo->bmiHeader.biPlanes=1;
	pBmpInfo->bmiHeader.biBitCount=24;//한 픽셀길이가 24 bit 
	//가로길이 4배수 나머지 값 얻기
	int m_ndumy=(bx*3)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	pBmpInfo->bmiHeader.biSizeImage=((bx*3)+m_ndumy)*by;

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
		return FALSE;
	size = sizeof(BITMAPINFO)-sizeof(RGBQUAD);
	if(!WriteFile(fd,pBmpInfo,size,&len,NULL))
		return FALSE;

	FlipY(bb,CSize(bx,by),3);//이미지를 뒤집는다..

	if(!WriteFile(fd,bb,pBmpInfo->bmiHeader.biSizeImage,&len,NULL))
		return FALSE;
	CloseHandle(fd);

	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);
	delete pBmpInfo;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////
////Jpg2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
byte* JpgFile2BmpMemory(LPCTSTR jpgname,DWORD& bsize)
{
	HANDLE Heap;
	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);								// init JPEG decompressing engine
	p.JPGFile=(char*)jpgname;								// what's the filename ?
	ijlRead (&p,IJL_JFILE_READPARAMS);	// read JPEG header
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth;
	p.DIBHeight=p.JPGHeight;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
	// now I must pass a buffer large enough to the decompressor
	p.DIBBytes=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	
	ijlRead (&p,IJL_JFILE_READWHOLEIMAGE);	// finally, this reads the image data and decompresses it to my buffer

	int bx,by;		    // bitmap dimensions (x=width, y=height), in pixels
	int bl;				// the length of one scan line, in bytes
						// it is equal to 3*x rounded up to nearest number divisible by 4
						// so l can be computed as l=3*(x+1)&(~3)
						// next_scan_line_adress = previous_scan_line_adress + l (bytes)
	BYTE *bb;		    // pointer to color data
						// data are stored row by row, top to bottom
						// each row occupies l bytes
						// pixels in row are stored left to right
						// each pixel occupies 3 bytes (B,G,R)
						// values of bytes mean R,G,B intensity (0=black, 255=highest intensity)

	bx=p.DIBWidth;			// filling up the DIB structure, as described in typedef section
	by=p.DIBHeight;
	bl=3*(bx+1)&(~3);
	bb=p.DIBBytes;
	ijlFree (&p);				// close the JPEG engine

	LPBITMAPINFO pBmpInfo;
	pBmpInfo = (LPBITMAPINFO) new char[sizeof(BITMAPINFO)];

	//bmiHeader의 구조체 크기 설정
	pBmpInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth=bx;
	pBmpInfo->bmiHeader.biHeight=by;
	pBmpInfo->bmiHeader.biPlanes=1;
	pBmpInfo->bmiHeader.biBitCount=24;//한 픽셀길이가 24 bit 
	//가로길이 4배수 나머지 값 얻기
	int m_ndumy=(bx*3)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	pBmpInfo->bmiHeader.biSizeImage=((bx*3)+m_ndumy)*by;

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

	bsize=sizeof(bmpHeader)+sizeof(BITMAPINFO)-sizeof(RGBQUAD)+pBmpInfo->bmiHeader.biSizeImage;
	byte* bmpbuf=new byte[bsize];
	byte* offset=bmpbuf;

	memcpy(offset,&bmpHeader,sizeof(bmpHeader)); offset+=sizeof(bmpHeader);
	size = sizeof(BITMAPINFO)-sizeof(RGBQUAD);
	memcpy(offset,pBmpInfo,size); offset+=size;

	FlipY(bb,CSize(bx,by),3);//이미지를 뒤집는다..

	memcpy(offset,bb,pBmpInfo->bmiHeader.biSizeImage); offset+=pBmpInfo->bmiHeader.biSizeImage;


	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);
	delete pBmpInfo;

	return bmpbuf;
}
///////////////////////////////////////////////////////////////////////////
////Bmp2Jpg////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BOOL Bmp2Jpg(LPCTSTR bmpname,LPCTSTR jpgname)
{
	int bx,by;
	HANDLE fh=CreateFile(bmpname,GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==NULL) return FALSE;

	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmiHeader;

	DWORD dwTemp;
	ReadFile(fh,&bmpHeader,sizeof(BITMAPFILEHEADER),&dwTemp,NULL);
	ReadFile(fh,&bmiHeader,sizeof(BITMAPINFOHEADER),&dwTemp,NULL);
	BYTE* DIB= new BYTE[bmiHeader.biSizeImage];
	ReadFile(fh,DIB,bmiHeader.biSizeImage,&dwTemp,NULL);
	bx=bmiHeader.biWidth;
	by=bmiHeader.biHeight;
	CloseHandle(fh);
//CString str;
//str.Format("%s %d %d",jpgname,bx,by);
//AfxMessageBox(str);
//	HANDLE Heap;
//	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);						// init JPEG decompressing engine

	p.JPGFile=(char*)jpgname;
	p.JPGChannels=3;
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth=bx;
	p.DIBHeight=p.JPGHeight=by;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
	// what's the filename ?
	ijlWrite (&p,IJL_JFILE_WRITEHEADER);	// write JPEG header
	
	p.DIBBytes=DIB;
	FlipY(p.DIBBytes,CSize(bx,by),3);//이미지를 뒤집는다..
	// now I must pass a buffer large enough to the decompressor
//	=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	

	ijlWrite (&p,IJL_JFILE_WRITEWHOLEIMAGE);	

	ijlFree (&p);				// close the JPEG engine

//	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);

	delete [] DIB;

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
////Jpg2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
byte* Jpg2BmpMemory(byte* jpgbuf,DWORD jsize, DWORD& bsize)
{
	HANDLE Heap;
	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);								// init JPEG decompressing engine
//	p.JPGFile=(char*)jpgname;								// what's the filename ?
	p.JPGBytes=(BYTE*)jpgbuf;								// what's the filename ?
	p.JPGSizeBytes=jsize;	
	ijlRead (&p,IJL_JBUFF_READPARAMS);	// read JPEG header
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth;
	p.DIBHeight=p.JPGHeight;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
	// now I must pass a buffer large enough to the decompressor
	p.DIBBytes=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	
	ijlRead (&p,IJL_JBUFF_READWHOLEIMAGE);	// finally, this reads the image data and decompresses it to my buffer

	int bx,by;		    // bitmap dimensions (x=width, y=height), in pixels
	int bl;				// the length of one scan line, in bytes
						// it is equal to 3*x rounded up to nearest number divisible by 4
						// so l can be computed as l=3*(x+1)&(~3)
						// next_scan_line_adress = previous_scan_line_adress + l (bytes)
	BYTE *bb;		    // pointer to color data
						// data are stored row by row, top to bottom
						// each row occupies l bytes
						// pixels in row are stored left to right
						// each pixel occupies 3 bytes (B,G,R)
						// values of bytes mean R,G,B intensity (0=black, 255=highest intensity)

	bx=p.DIBWidth;			// filling up the DIB structure, as described in typedef section
	by=p.DIBHeight;
	bl=3*(bx+1)&(~3);
	bb=p.DIBBytes;
	ijlFree (&p);				// close the JPEG engine

	LPBITMAPINFO pBmpInfo;
	pBmpInfo = (LPBITMAPINFO) new char[sizeof(BITMAPINFO)];

	//bmiHeader의 구조체 크기 설정
	pBmpInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth=bx;
	pBmpInfo->bmiHeader.biHeight=by;
	pBmpInfo->bmiHeader.biPlanes=1;
	pBmpInfo->bmiHeader.biBitCount=24;//한 픽셀길이가 24 bit 
	//가로길이 4배수 나머지 값 얻기
	int m_ndumy=(bx*3)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	pBmpInfo->bmiHeader.biSizeImage=((bx*3)+m_ndumy)*by;

	pBmpInfo->bmiHeader.biCompression=BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter=0;
	pBmpInfo->bmiHeader.biYPelsPerMeter=0;
	pBmpInfo->bmiHeader.biClrUsed =0;
	pBmpInfo->bmiHeader.biClrImportant =0;

 //CString test;
//test.Format("%s %d %d",jpgname,bx,by);
//AfxMessageBox(test);

 	BITMAPFILEHEADER bmpHeader;
	DWORD size;
	bmpHeader.bfType=DIB_HEADER_MARKER;
	bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize=sizeof(bmpHeader)-sizeof(RGBQUAD)+sizeof(BITMAPINFO)+pBmpInfo->bmiHeader.biSizeImage;
	bmpHeader.bfReserved1=0;
	bmpHeader.bfReserved2=0;

	bsize=sizeof(bmpHeader)+sizeof(BITMAPINFO)-sizeof(RGBQUAD)+pBmpInfo->bmiHeader.biSizeImage;
	byte* bmpbuf=new byte[bsize];
	byte* offset=bmpbuf;

	memcpy(offset,&bmpHeader,sizeof(bmpHeader)); offset+=sizeof(bmpHeader);
	size = sizeof(BITMAPINFO)-sizeof(RGBQUAD);
	memcpy(offset,pBmpInfo,size); offset+=size;

	FlipY(bb,CSize(bx,by),3);//이미지를 뒤집는다..

	memcpy(offset,bb,pBmpInfo->bmiHeader.biSizeImage); offset+=pBmpInfo->bmiHeader.biSizeImage;

	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);
	delete pBmpInfo;

	return bmpbuf;
}
byte* Jpg2BmpMemoryByDIB(byte* jpgbuf,DWORD jsize, DWORD& bsize,int &bx,int &by)
{
	HANDLE Heap;
	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);								// init JPEG decompressing engine
//	p.JPGFile=(char*)jpgname;								// what's the filename ?
	p.JPGBytes=(BYTE*)jpgbuf;								// what's the filename ?
	p.JPGSizeBytes=jsize;	
	ijlRead (&p,IJL_JBUFF_READPARAMS);	// read JPEG header
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth;
	p.DIBHeight=p.JPGHeight;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
	// now I must pass a buffer large enough to the decompressor
	p.DIBBytes=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	
	ijlRead (&p,IJL_JBUFF_READWHOLEIMAGE);	// finally, this reads the image data and decompresses it to my buffer

	//int bx,by;		    // bitmap dimensions (x=width, y=height), in pixels
	int bl;				// the length of one scan line, in bytes
						// it is equal to 3*x rounded up to nearest number divisible by 4
						// so l can be computed as l=3*(x+1)&(~3)
						// next_scan_line_adress = previous_scan_line_adress + l (bytes)
	BYTE *bb;		    // pointer to color data
						// data are stored row by row, top to bottom
						// each row occupies l bytes
						// pixels in row are stored left to right
						// each pixel occupies 3 bytes (B,G,R)
						// values of bytes mean R,G,B intensity (0=black, 255=highest intensity)

	bx=p.DIBWidth;			// filling up the DIB structure, as described in typedef section
	by=p.DIBHeight;
	bl=3*(bx+1)&(~3);
	bb=p.DIBBytes;
	ijlFree (&p);				// close the JPEG engine

	//가로길이 4배수 나머지 값 얻기
	int m_ndumy=(bx*3)%4;
	((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
	int biSizeImage=((bx*3)+m_ndumy)*by;

	bsize=biSizeImage;
	byte* bmpbuf=new byte[bsize];
	byte* offset=bmpbuf;
	//FlipY(bb,CSize(bx,by),3);//이미지를 뒤집는다..
	memcpy(offset,bb,biSizeImage);
	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);

	return bmpbuf;
}
///////////////////////////////////////////////////////////////////////////
////Bmp2Jpg////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
byte* Bmp2JpgMemory( byte* bmpbuf, DWORD dsize, DWORD& jsize, DWORD quality )
{
	int bx,by;

	if(bmpbuf==NULL) return NULL;

	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmiHeader;

	byte* offset=bmpbuf;
	memcpy(&bmpHeader,offset,sizeof(BITMAPFILEHEADER)); offset+=sizeof(BITMAPFILEHEADER);
	memcpy(&bmiHeader,offset,sizeof(BITMAPINFOHEADER)); offset+=sizeof(BITMAPINFOHEADER);
	BYTE* DIB= new BYTE[bmiHeader.biSizeImage];
	memcpy(DIB,offset,bmiHeader.biSizeImage); offset+=bmiHeader.biSizeImage;

	bx=bmiHeader.biWidth;
	by=bmiHeader.biHeight;

//	HANDLE Heap;
//	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);						// init JPEG decompressing engine

	DWORD dwRgbBufferSize = bx * by * 3 * 3;
	byte* lpTemp=new byte[dwRgbBufferSize];
//	p.JPGFile=(char*)jpgname;
	p.JPGBytes       = lpTemp;
    p.JPGSizeBytes   = dwRgbBufferSize;
	p.JPGChannels=3;
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth=bx;
	p.DIBHeight=p.JPGHeight=by;
	p.DIBChannels=3;
	p.jquality = quality;
	if(quality<50) p.jquality = 75;

//	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
//	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
//	ijlWrite (&p,IJL_JBUFF_WRITEHEADER);	// write JPEG header

	p.DIBBytes=DIB;

	FlipY(p.DIBBytes,CSize(bx,by),3);//이미지를 뒤집는다..
	// now I must pass a buffer large enough to the decompressor
//	=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	

	ijlWrite (&p,IJL_JBUFF_WRITEWHOLEIMAGE);

	jsize=p.JPGSizeBytes;
//str.Format("jpg size %d ",p.JPGSizeBytes);
//AfxMessageBox(str);
	byte* jpgbuf=new byte[jsize];	
	memcpy(jpgbuf,lpTemp,jsize);
/*DWORD dwTemp;
	HANDLE fh=CreateFile("c:\\a.jpg", GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	WriteFile(fh,jpgbuf,jsize,&dwTemp,NULL);
	CloseHandle(fh);*/
	if(lpTemp) {
		delete [] lpTemp;
		lpTemp=0;
	}

	ijlFree (&p);			// close the JPEG engine

//	HeapFree(Heap,HEAP_NO_SERIALIZE,bb);

	if(DIB) {
		delete [] DIB;
		DIB=0;
	}

	return jpgbuf;
}
///////////////////////////////////////////////////////////////////////////
////Bmp2Jpg////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool BmpMemory2JpgFile( byte* bmpbuf, DWORD dsize, char * jpgname, DWORD quality )
{
	int bx,by;

	if(bmpbuf==NULL) return NULL;

	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmiHeader;

	byte* offset=bmpbuf;
	memcpy(&bmpHeader,offset,sizeof(BITMAPFILEHEADER)); offset+=sizeof(BITMAPFILEHEADER);
	memcpy(&bmiHeader,offset,sizeof(BITMAPINFOHEADER)); offset+=sizeof(BITMAPINFOHEADER);
	BYTE* DIB= new BYTE[bmiHeader.biSizeImage];
	memcpy(DIB,offset,bmiHeader.biSizeImage); offset+=bmiHeader.biSizeImage;

	bx=bmiHeader.biWidth;
	by=bmiHeader.biHeight;

//	HANDLE Heap;
//	Heap=GetProcessHeap();

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);						// init JPEG decompressing engine

	p.JPGFile=(char*)jpgname;
	p.JPGChannels=3;
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth=bx;
	p.DIBHeight=p.JPGHeight=by;
	p.DIBChannels=3;
	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);

	p.jquality = quality;
	if(quality<50) p.jquality = 75;

	ijlWrite (&p,IJL_JFILE_WRITEHEADER);	// write JPEG header
	
	p.DIBBytes=DIB;
	FlipY(p.DIBBytes,CSize(bx,by),3);//이미지를 뒤집는다..

	ijlWrite (&p,IJL_JFILE_WRITEWHOLEIMAGE);	

	ijlFree (&p);				// close the JPEG engine

	delete [] DIB;
	return true;
}

/*
byte* Bmp2JpgMemoryByDIB_ori( byte* bmpbuf,int bx,int by ,DWORD dsize, DWORD& jsize, DWORD quality )
{

	if(bmpbuf==NULL) return NULL;

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);						// init JPEG decompressing engine

	DWORD dwRgbBufferSize = bx * by * 3;
	byte* lpTemp=new byte[dwRgbBufferSize];
//	p.JPGFile=(char*)jpgname;
	p.JPGBytes       = lpTemp;
    p.JPGSizeBytes   = dwRgbBufferSize;
	p.JPGChannels=3;
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth=bx;
	p.DIBHeight=p.JPGHeight=by;
	p.DIBChannels=3;
	p.jquality = quality;
	if(quality<50) p.jquality = 75;

//	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
//	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
//	ijlWrite (&p,IJL_JBUFF_WRITEHEADER);	// write JPEG header

	p.DIBBytes=bmpbuf;

	//FlipY(p.DIBBytes,CSize(bx,by),3);//이미지를 뒤집는다..
	// now I must pass a buffer large enough to the decompressor
//	=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	

	ijlWrite (&p,IJL_JBUFF_WRITEWHOLEIMAGE);

	jsize=p.JPGSizeBytes;
//str.Format("jpg size %d ",p.JPGSizeBytes);
//AfxMessageBox(str);
	byte* jpgbuf=new byte[jsize];	
	memcpy(jpgbuf,lpTemp,jsize);
	if(lpTemp) {
		delete [] lpTemp;
		lpTemp=0;
	}

	ijlFree (&p);			// close the JPEG engine


	return jpgbuf;
}*/
byte* Bmp2JpgMemoryByDIB( byte* bmpbuf,int bx,int by ,DWORD dsize, DWORD& jsize, DWORD quality )
{

	if(bmpbuf==NULL) return NULL;

	JPEG_CORE_PROPERTIES p;				// Intel's JPEG structure
	ZeroMemory (&p,sizeof(p));			// must be cleared
	ijlInit (&p);						// init JPEG decompressing engine

	DWORD dwRgbBufferSize = bx * by * 3 * 3;
	byte* lpTemp=new byte[dwRgbBufferSize];
//	p.JPGFile=(char*)jpgname;
	p.JPGBytes       = lpTemp;
    p.JPGSizeBytes   = dwRgbBufferSize;
	p.JPGChannels=3;
	switch(p.JPGChannels) {				// some manipulations I don't understand much
		case 1: p.JPGColor=IJL_G;break;
		case 3: p.JPGColor=IJL_YCBCR;break;
		default: p.DIBColor=p.JPGColor=(IJL_COLOR)IJL_OTHER;break;
	}
	p.DIBWidth=p.JPGWidth=bx;
	p.DIBHeight=p.JPGHeight=by;
	p.DIBChannels=3;
	p.jquality = quality;
	if(quality<50) p.jquality = 75;

//	p.DIBPadBytes=IJL_DIB_PAD_BYTES(p.DIBWidth,p.DIBChannels);
//	long length = (p.DIBWidth*p.DIBChannels+p.DIBPadBytes)*p.DIBHeight;
//	ijlWrite (&p,IJL_JBUFF_WRITEHEADER);	// write JPEG header

	p.DIBBytes=bmpbuf;

	//FlipY(p.DIBBytes,CSize(bx,by),3);//이미지를 뒤집는다..
	// now I must pass a buffer large enough to the decompressor
//	=(BYTE*)HeapAlloc(Heap,HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,(length)*sizeof(BYTE));	

	ijlWrite (&p,IJL_JBUFF_WRITEWHOLEIMAGE);

	jsize=p.JPGSizeBytes;
//str.Format("jpg size %d ",p.JPGSizeBytes);
//AfxMessageBox(str);
	byte* jpgbuf=new byte[jsize];	
	memcpy(jpgbuf,lpTemp,jsize);
	if(lpTemp) {
		delete [] lpTemp;
		lpTemp=0;
	}

	ijlFree (&p);			// close the JPEG engine


	return jpgbuf;
}
///////////////////////////////////////////////////////////////////////////
////Pcx2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BOOL Pcx2Bmp(LPCTSTR pcxname,LPCTSTR bmpname)
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
	const long clFileSize = _filelength(_fileno(pFile));
	BYTE *pabFileData = (BYTE *)new BYTE[ clFileSize ];
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

   for ( int iY=0; iY < BitmapSize.cy; iY++ )
   {
      // Decompress the scan line
      for ( int iX=0; iX < sHeader.BPL; )
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

	FlipY(pabRawBitmap,BitmapSize,1);//이미지를 뒤집는다.

	if(!WriteFile(fd,pabRawBitmap,pBmpInfo->bmiHeader.biSizeImage,&len,NULL))
		return FALSE;

	CloseHandle(fd);

    delete [] pabFileData;
	delete [] pabRawBitmap;
	delete pBmpInfo;
	return TRUE;

}

///////////////////////////////////////////////////////////////////////////
////Tga2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BOOL Tga2Bmp(LPCTSTR tganame,LPCTSTR bmpname)
{
    FILE* file = fopen( tganame, "rb" );
    if( NULL == file )
        return FALSE;

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
        return FALSE;
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
        return FALSE;
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
		return FALSE;
	size = sizeof(BITMAPINFO)-sizeof(RGBQUAD);
	if(!WriteFile(fd,pBmpInfo,size,&len,NULL))
		return FALSE;

//	FlipY(pBytes,CSize(m_dwWidth,m_dwHeight),3);//이미지를 뒤집는다..

	if(!WriteFile(fd,pBytes,pBmpInfo->bmiHeader.biSizeImage,&len,NULL))
		return FALSE;
	CloseHandle(fd);

	delete [] pBytes;

    return TRUE;
}

void FlipY(BYTE* image , CSize size, int width)
{
	//한 스캔라인에 소요된 실제 바이트수 4의 배수를 구합니다.
	int RealWidth = (size.cx * width + 3)/4*4;
	//이미지의 높이를 구합니다.
	int Height = size.cy;	
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
////Gif2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BITMAPFILEHEADER hdr;
BITMAPINFOHEADER bi;
long	wib;
RGBQUAD cmap[256];
FILE   *fp;
int     showComment;
int     verbose = 0;
int     ZeroDataBlock = 0;

BOOL Gif2Bmp(LPCTSTR gifname,LPCTSTR bmpname)
{
	FILE*	in;
	int		imageNumber = 1;

	in= fopen(gifname, "rb");
	if(in==0){
		AfxMessageBox("gif file is not exist");
		return FALSE;
	}

	fp= fopen(bmpname, "wb");
	if(fp==0){
		AfxMessageBox("bmp file is not exist");
		return FALSE;
	}
	ReadGIF(in, imageNumber);
	fclose(in);
	fclose(fp);
	return 1;

}

int dib_wib(int bitcount, int wi)
{
	switch (bitcount){
	case 1: wi= (wi+31) >> 3; break;
	case 4: wi= (wi+7)  >> 1; break;
	case 8: wi=  wi+3; break;
	case 16:wi= (wi*2)+3; break;
	case 24:wi= (wi*3)+3; break;
	case 32:return wi*4;
	}
	return wi & ~3;
}
/* Output .bmp header */
int spew_header(int wi, int hi, int n, RGBQUAD *cmap)
{
	/* Writes to the global fp */
		 if(n>16) bi.biBitCount= 8;
	else if(n>2 ) bi.biBitCount= 4;
	else          bi.biBitCount= 1;
	n = 1 << ((int)bi.biBitCount);
	bi.biSize= sizeof(bi);
	bi.biWidth= wi;
	bi.biHeight= hi;
	bi.biPlanes= 1;
	bi.biCompression= 0;
	wib = dib_wib((int)bi.biBitCount, (int)bi.biWidth);
	bi.biSizeImage= bi.biHeight * wib;
	bi.biClrUsed= n;
	bi.biClrImportant= n;
	bi.biXPelsPerMeter= 0;
	bi.biYPelsPerMeter= 0;

	hdr.bfType= 0x4d42;	/* BM */
	hdr.bfReserved1= 0;
	hdr.bfReserved2= 0;
	hdr.bfOffBits = sizeof(hdr) + bi.biSize + n*sizeof(RGBQUAD);
	hdr.bfSize= hdr.bfOffBits + bi.biSizeImage;

	fwrite(&hdr,1,sizeof(hdr),fp);
	fwrite(&bi, 1, sizeof(bi), fp);
	if(n>0)
		fwrite(cmap, sizeof(RGBQUAD), n, fp);
	return 1;
}

void ReadGIF(FILE *fd, int imageNumber)
{
	unsigned char buf[16];
	unsigned char c;
	static RGBQUAD localColorMap[MAXCOLORMAPSIZE];
	int useGlobalColormap;
	int bitPixel;
	int imageCount = 0;
	char version[4];
	CString str;

	if (! ReadOK(fd,buf,6)) {
		AfxMessageBox("error reading magic number");
		return;
	}

	if (strncmp((char *)buf,"GIF",3) != 0){
		AfxMessageBox("not a GIF file");
		return;
	}

	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';

	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)){
		AfxMessageBox("bad version number, not '87a' or '89a'");
		return;
	}

	if (! ReadOK(fd,buf,7)){
		AfxMessageBox("failed to read screen descriptor");
		return;
	}

	GifScreen.Width 	  = LM_to_uint(buf[0],buf[1]);
	GifScreen.Height	  = LM_to_uint(buf[2],buf[3]);
	GifScreen.BitPixel	  = 2<<(buf[4]&0x07);
	GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
	GifScreen.Background	 = buf[5];
	GifScreen.AspectRatio	 = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
		if (ReadColorMap(fd,GifScreen.BitPixel, GifScreen.ColorMap)){
			AfxMessageBox("error reading global colormap");
			return;
		}
	}
	/*
	if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
		float	r;
		r = ( (float) GifScreen.AspectRatio + 15.0 ) / 64.0;
		fprintf(stderr,"warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'\n",
		  r < 1.0 ? 'x' : 'y',
		  r < 1.0 ? 1.0 / r : r );
	}
	*/
	for (;;) {
		if (! ReadOK(fd,&c,1)){
			AfxMessageBox("EOF / read error on image data");
			return;
		}

		if (c == ';') {	 /* GIF terminator */
			if (imageCount < imageNumber){
				str.Format("only %d image%s found in file\n",
													imageCount, imageCount>1?"s":"" );
				AfxMessageBox(str);
				return;
			}
			return;
		}

		if (c == '!') {	 /* Extension */
			if (! ReadOK(fd,&c,1)){
				AfxMessageBox("OF / read error on extention function code");
				return;
			}
			DoExtension(fd, c);
			continue;
		}

		if (c != ',') {	 /* Not a valid start character */
			str.Format("bogus character 0x%02x, ignoring\n", (int)c);
			AfxMessageBox(str);
			continue;
		}

		++imageCount;

		if (! ReadOK(fd,buf,9)){
			AfxMessageBox("couldn't read left/top/width/height");
			return;
		}

		useGlobalColormap = ! (buf[8] & LOCALCOLORMAP);

		bitPixel = 1<<((buf[8]&0x07)+1);

		if (! useGlobalColormap) {
			if (ReadColorMap(fd, bitPixel, localColorMap)){
				AfxMessageBox("error reading local colormap");
				return;
			}
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				localColorMap, bitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		} else {
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				GifScreen.ColorMap, GifScreen.BitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		}

	}
}

int ReadColorMap(FILE *fd, int number, RGBQUAD *buffer)
{
	int i;
	unsigned char rgb[3];

	for (i = 0; i < number; ++i, buffer++) {
		if (! ReadOK(fd, rgb, sizeof(rgb))){
			AfxMessageBox("bad colormap");
			return -1;
		}
		buffer->rgbRed= rgb[0];
		buffer->rgbGreen= rgb[1];
		buffer->rgbBlue= rgb[2];
		buffer->rgbReserved= 0;
	}
	return 0;
}
int DoExtension(FILE *fd, int label)
{
	static char	buf[256];
	char		*str;

	switch (label) {
	case 0x01:		/* Plain Text Extension */
		str = "Plain Text Extension";
#ifdef notdef
		if (GetDataBlock(fd, (unsigned char*) buf) == 0);

		lpos	= LM_to_uint(buf[0], buf[1]);
		tpos	= LM_to_uint(buf[2], buf[3]);
		width	= LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw	= buf[8];
		cellh	= buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			PPM_ASSIGN(image[ypos][xpos],
					cmap[CM_RED][v],
					cmap[CM_GREEN][v],
					cmap[CM_BLUE][v]);
			++index;
		}

		return FALSE;
#else
		break;
#endif
	case 0xff:		/* Application Extension */
		str = "Application Extension";
		GetDataBlock(fd, (unsigned char*) buf);
		if (showComment){
			fprintf(stderr, "Application Extension: %c%c%c%c%c%c%c%c ",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
			fprintf(stderr, " Authentication Code=)%02x %02x %02x\n",
				buf[8], buf[9], buf[10]);
		}
		break;
	case 0xfe:		/* Comment Extension */
		str = "Comment Extension";
		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			if (showComment)
				fprintf(stderr,"gif comment: %s\n", buf );
		}
		return FALSE;
	case 0xf9:		/* Graphic Control Extension */
		str = "Graphic Control Extension";
		(void) GetDataBlock(fd, (unsigned char*) buf);
		Gif89.disposal    = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
		Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
		if ((buf[0] & 0x1) != 0)
			Gif89.transparent = buf[3];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0)
			;
		return FALSE;
	default:
		str = buf;
		sprintf(buf, "UNKNOWN (0x%02x)", label);
		break;
	}

	fprintf(stderr,"got a '%s' extension\n", str );

	while (GetDataBlock(fd, (unsigned char*) buf) != 0)
		;

	return FALSE;
}

int GetDataBlock(FILE *fd, unsigned char  *buf)
{
	unsigned char	count;

	if (! ReadOK(fd,&count,1)) {
		fprintf(stderr,"error in getting DataBlock size\n" );
		return -1;
	}

	ZeroDataBlock = count == 0;

	if ((count != 0) && (! ReadOK(fd, buf, count))) {
		fprintf(stderr,"error in reading DataBlock\n" );
		return -1;
	}

	return count;
}
int GetCode(FILE *fd, int code_size, int flag)
{
	static unsigned char	buf[280];
	static int		curbit, lastbit, done, last_byte;
	int			i, j, ret;
	unsigned char		count;

	if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( (curbit+code_size) >= lastbit) {
		if (done) {
			if (curbit >= lastbit){
				AfxMessageBox("ran off the end of my bits");
				return -1;
			}
			return -1;
		}
		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		if ((count = GetDataBlock(fd, &buf[2])) == 0)
			done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2+count)*8 ;
	}

	ret = 0;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

	curbit += code_size;

	return ret;
}
int LWZReadByte(FILE *fd, int flag, int input_code_size)
{
	static int	fresh = FALSE;
	int		code, incode;
	static int	code_size, set_code_size;
	static int	max_code, max_code_size;
	static int	firstcode, oldcode;
	static int	clear_code, end_code;
	static int	table[2][(1<< MAX_LWZ_BITS)];
	static int	stack[(1<<(MAX_LWZ_BITS))*2], *sp;
	register int	i;

	if (flag) {
		set_code_size = input_code_size;
		code_size = set_code_size+1;
		clear_code = 1 << set_code_size ;
		end_code = clear_code + 1;
		max_code_size = 2*clear_code;
		max_code = clear_code+2;

		GetCode(fd, 0, TRUE);

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i) {
			table[0][i] = 0;
			table[1][i] = i;
		}
		for (; i < (1<<MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][0] = 0;

		sp = stack;

		return 0;
	} else if (fresh) {
		fresh = FALSE;
		do {
			firstcode = oldcode =
				GetCode(fd, code_size, FALSE);
		} while (firstcode == clear_code);
		return firstcode;
	}

	if (sp > stack)
		return *--sp;

	while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
			for (i = 0; i < clear_code; ++i) {
				table[0][i] = 0;
				table[1][i] = i;
			}
			for (; i < (1<<MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = 0;
			code_size = set_code_size+1;
			max_code_size = 2*clear_code;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode =
					GetCode(fd, code_size, FALSE);
			return firstcode;
		} else if (code == end_code) {
			int		count;
			unsigned char   buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = GetDataBlock(fd, buf)) > 0)
				;

			if (count != 0)
				fprintf(stderr,"missing EOD in data stream (common occurence)\n");
			return -2;
		}

		incode = code;

		if (code >= max_code) {
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code) {
			*sp++ = table[1][code];
			if (code == table[0][code]){
				AfxMessageBox("circular table entry BIG ERROR");
				return -1;
			}
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1<<MAX_LWZ_BITS))) {
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}
void ReadImage(FILE *fd, int len, int height, RGBQUAD *cmap,
									 int bpp, int interlace, int ignore)
{
	unsigned char	c;
	int		v;
	int		xpos = 0, ypos = 0, pass = 0;
	unsigned char *scanline;
	CString str;

	/*
	**  Initialize the Compression routines
	*/
	if (! ReadOK(fd,&c,1)){
		AfxMessageBox("EOF / read error on image data");
		return ;
	}

	if (LWZReadByte(fd, TRUE, c) < 0){
		AfxMessageBox("error reading image");
		return ;
	}

	/*
	**  If this is an "uninteresting picture" ignore it.
	*/
	if (ignore) {
		if (verbose)
			fprintf(stderr,"skipping image...\n" );

		while (LWZReadByte(fd, FALSE, c) >= 0)
			;
		return;
	}
	if ((scanline= (unsigned char *)malloc(len)) == NULL){
		AfxMessageBox("couldn't alloc space for image");
		return ;
	}


	if (verbose){
		str.Format("reading %d by %d%s GIF image\n",
							len, height, interlace ? " interlaced" : "" );
		AfxMessageBox(str);
	}

	spew_header(len, height, bpp, cmap);

	/* Fill the whole file with junk */
	for(v= 0; v<height; v++)
		fwrite(scanline, 1, (int)wib, fp);

	while (ypos<height && (v = LWZReadByte(fd,FALSE,c)) >= 0) {
		switch(bi.biBitCount){
		case 1:
			if(v)
				scanline[xpos>>3] |= 128 >> (xpos&7);
			else
				scanline[xpos>>3] &= 0xff7f >> (xpos&7);
			break;
		case 4:
			if(xpos&1)
				scanline[xpos>>1] |= v&15;
			else
				scanline[xpos>>1] = (v&15) << 4;
			break;
		case 8:
			scanline[xpos]= v;
			break;
		}
		++xpos;
		if (xpos == len) {
			fseek(fp, -(ypos+1)*wib, SEEK_END);
			fwrite(scanline, 1, (int)wib, fp);
			xpos = 0;
			if (interlace) {
				static int dpass[]= {8,8,4,2};
				ypos += dpass[pass];
				if (ypos >= height) {
					static int restart[]= {0,4,2,1,32767};
					ypos= restart[++pass];
				}
			} else
				++ypos;
		}
	}
	if(LWZReadByte(fd, FALSE,c) >= 0)
		AfxMessageBox("too much input data, ignoring extra...\n");

}
