// BaseImage.cpp: implementation of the CBaseImage class.
//
//////////////////////////////////////////////////////////////////////
#include "Baseimage.h"
#include "../base/xdmath.h"
//#include "tiff_util.h"
//#include "defaultPalette.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseImage::CBaseImage()
{
	m_pDib=NULL;
	lpLogPalette=NULL;
	pBitmapInfo=NULL;
}

CBaseImage::~CBaseImage()
{
	if(m_pDib) {
		delete [] m_pDib;
		m_pDib=NULL;
	} 

	if(pBitmapInfo){
		delete pBitmapInfo;
		pBitmapInfo=NULL;
	}

	if(lpLogPalette){
		delete lpLogPalette;
		lpLogPalette=NULL;
	}

	m_palette.clear();
}

//Create : size 크기의 영상을 colornum의 색으로 배경을 backcolor로 만든다.
bool CBaseImage::Create(SIZE size, int colornum,COLORREF backcolor)
{
	int r,g,b,i,j,a;
	if(!SetBmpInfo(size,colornum)) return false;

	//jsyun 20171208 사용 안함.
//	if(colornum)
//		if(!CreatePalette(false)) return false;

	if(m_pDib) { delete [] m_pDib;	m_pDib=NULL; }
	m_pDib= new u8 [pBitmapInfo->bmiHeader.biSizeImage];

	switch(m_nColor)
	{
	case 0://트루칼라를 사용할 경우( 24 bit )
			b=GetBValue(backcolor);
			g=GetGValue(backcolor);
			r=GetRValue(backcolor);
			for(i=0;i<size.cy;i++)
				for(j=0;j<size.cx;j++)
			{
				m_pDib[i*(size.cx*3+m_ndumy)+j*3]=(u8)b;
				m_pDib[i*(size.cx*3+m_ndumy)+j*3+1]=(u8)g;
				m_pDib[i*(size.cx*3+m_ndumy)+j*3+2]=(u8)r;
			}
			break;
	case 32://트루칼라를 사용할 경우( 32 bit )
			b=GetBValue(backcolor);
			g=GetGValue(backcolor);
			r=GetRValue(backcolor);
			a=GetAValue(backcolor);
			for(i=0;i<size.cy;i++)
				for(j=0;j<size.cx;j++)
			{
				m_pDib[i*(size.cx*4+m_ndumy)+j*4]=(u8)b;
				m_pDib[i*(size.cx*4+m_ndumy)+j*4+1]=(u8)g;
				m_pDib[i*(size.cx*4+m_ndumy)+j*4+2]=(u8)r;
				m_pDib[i*(size.cx*4+m_ndumy)+j*4+3]=(u8)a;
			}
			break;
	case 256:
			i=FindColorInPalette(backcolor);
			if(i<0)i=0;
			r=(u8)i;
			memset(m_pDib,r,pBitmapInfo->bmiHeader.biSizeImage);
			break;
	case 16:
			i=FindColorInPalette(backcolor);
			if(i<0)i=0;
			r=i;r=r<<4;g=r>>4;b=r|g;
			memset(m_pDib,b,pBitmapInfo->bmiHeader.biSizeImage);
			break;
	case 1:
			memset(m_pDib,0,pBitmapInfo->bmiHeader.biSizeImage);
			break;
	}
	return true;
}
bool CBaseImage::CreateByDIB(SIZE size, int colornum,u8* dib)
{
	int r,g,b,i,j;
//	printf("CreateByDIB - 0,size w,h = %d,%d\n",size.cx,size.cy);
	if(!SetBmpInfo(size,colornum)) return false;
	if(colornum!=0 && colornum!=32)
		if(!CreatePalette(false)) return false;
	if(m_pDib!=NULL) { delete [] m_pDib;	m_pDib=NULL; }
	m_pDib= new u8 [pBitmapInfo->bmiHeader.biSizeImage];
	memcpy(m_pDib,dib,pBitmapInfo->bmiHeader.biSizeImage);

	return true;
}

bool CBaseImage::LoadImageMemory( u8* data, u32 isize )
{
	if(isize==0) return false;
	u8* offset=data;

	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmiHeader;
	//bmp file header 메모리 복사
	memcpy(&bmpHeader,offset,sizeof(BITMAPFILEHEADER));	offset+=sizeof(BITMAPFILEHEADER);
	DibReadBitmapInfo(&offset, &bmiHeader);//이미지 정보를 읽는다.

	u32 dwCompression=bmiHeader.biCompression;
	u32 dwBitCount=bmiHeader.biBitCount; //preserve for BI_BITFIELDS compression <Thomas Ernst>
	bool bIsOS2 = bmiHeader.biSize != sizeof(BITMAPINFOHEADER);
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

//_debug("dwBitCount %d",dwBitCount);

	SIZE size;
	size.cx = bmiHeader.biWidth; 
	size.cy = bmiHeader.biHeight;
	int nColor =0;
	if(bmiHeader.biBitCount==24) nColor=0;
	else if(bmiHeader.biBitCount==32) nColor=32;
	else nColor=0;
	if(!SetBmpInfo(size,nColor)) return false;
	if(m_pDib) { delete [] m_pDib;	m_pDib=NULL; }
//	fprintf(stderr,"LoadImageMemory - 1 , %d - %d - %d - m_nColor %d\n",pBitmapInfo->bmiHeader.biSizeImage,isize,bmiHeader.biSizeImage,m_nColor);
	m_pDib= new u8 [pBitmapInfo->bmiHeader.biSizeImage];
//	fprintf(stderr,"LoadImageMemory - 2 , bmpHeader.biBitCount %d\n",bmiHeader.biBitCount);

	switch (dwBitCount) {
		/*
		case 32 ://32 비트 이미지 불러오기
			m_nColor=32;
			if (bmpHeader.bfOffBits != 0L) {
				offset=data+bmpHeader.bfOffBits;//SetFilePointer(fh, bmpHeader.bfOffBits,NULL,FILE_BEGIN);
			fprintf(stderr,"bmpHeader.bfOffBits != 0L - bmpHeader.bfOffBits %d\n",bmpHeader.bfOffBits);				
			}
			if (dwCompression == BI_BITFIELDS || dwCompression == BI_RGB){
				long imagesize=4*bmiHeader.biHeight*bmiHeader.biWidth;
fprintf(stderr,"imagesize 0- %d\n",imagesize);
				u8* buff32=new u8[imagesize];
				memcpy(buff32,offset,imagesize);	offset+=imagesize;
				fprintf(stderr,"imagesize 1- %d\n",imagesize);
				Bitfield2RGB(buff32,0,0,0,32);
				fprintf(stderr,"imagesize 2- %d\n",imagesize);
				delete buff32;
				fprintf(stderr,"imagesize 3- %d\n",imagesize);
				buff32=NULL;
				fprintf(stderr,"imagesize 4- %d\n",imagesize);
			} else return false;
			break; */
		case 32 :
			m_nColor=32;
			if (bmpHeader.bfOffBits != 0L) offset=data+bmpHeader.bfOffBits;//SetFilePointer(fh, bmpHeader.bfOffBits,NULL,FILE_BEGIN);
			if (dwCompression == BI_RGB){
				memcpy(m_pDib,offset,bmiHeader.biSizeImage);	offset+=bmiHeader.biSizeImage;
//				ReadFile(fh,m_pDib,bmiHeader.biSizeImage,&dwTemp,NULL);
			} else return false;
			break;
		case 24 :
			m_nColor=0;
			if (bmpHeader.bfOffBits != 0L) offset=data+bmpHeader.bfOffBits;//SetFilePointer(fh, bmpHeader.bfOffBits,NULL,FILE_BEGIN);
			if (dwCompression == BI_RGB){
				memcpy(m_pDib,offset,bmiHeader.biSizeImage);	offset+=bmiHeader.biSizeImage;
//				ReadFile(fh,m_pDib,bmiHeader.biSizeImage,&dwTemp,NULL);
			} else return false;
			break;
	}

	return true;
}

bool CBaseImage::LoadImage(const char* file)
{
	if(file) strcpy(m_strFileName,file);
	u8* data=0;
	u32 fsize=0;
	FILE *fp = fopen(file,"rb");
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	rewind (fp);
	data=new u8[fsize];
	fread(data,1,fsize,fp);
	fclose(fp);

	bool success=LoadImageMemory( data, fsize);
	delete [] data;
	data=0;

	return success;
}

void CBaseImage::Bitfield2RGB(u8 *src, u16 redmask, u16 greenmask, u16 bluemask, u8 bpp)
{
	u32 dwWidth=pBitmapInfo->bmiHeader.biWidth;
	// 세로크기
	u32 dwHeight=pBitmapInfo->bmiHeader.biHeight;

	u32 dwEffWidth = ((((bpp * dwWidth) + 31) / 32) * 4);

	switch (bpp){
	case 16:
	{
		u32 ns[3]={0,0,0};
		// compute the number of shift for each mask
		for (int i=0;i<16;i++){
			if ((redmask>>i)&0x01) ns[0]++;
			if ((greenmask>>i)&0x01) ns[1]++;
			if ((bluemask>>i)&0x01) ns[2]++;
		}
		ns[1]+=ns[0]; ns[2]+=ns[1];	ns[0]=8-ns[0]; ns[1]-=8; ns[2]-=8;
		// dword aligned width for 16 bit image
		long effwidth2=(((dwWidth + 1) / 2) * 4);
		u16 w;
		long y2,y3,x2,x3;
		u8 *p=(u8*)m_pDib;
		// scan the buffer in reverse direction to avoid reallocations
		for (long y=dwHeight-1; y>=0; y--){
			y2=effwidth2*y;
			y3=dwEffWidth*y;
			for (long x=dwWidth-1; x>=0; x--){
				x2 = 2*x+y2;
				x3 = 3*x+y3;
				w = (u16)(src[x2]+256*src[1+x2]);
				p[  x3]=(u8)((w & bluemask)<<ns[0]);
				p[1+x3]=(u8)((w & greenmask)>>ns[1]);
				p[2+x3]=(u8)((w & redmask)>>ns[2]);
			}
		}
		break;
	}
	/*case 32:
	{
		// dword aligned width for 32 bit image
		long effwidth4 = dwWidth * 4;
		long y4,y3,x4,x3;
		u8 *p=(u8*)m_pDib;
		// scan the buffer in reverse direction to avoid reallocations
		for (long y=dwHeight-1; y>=0; y--){
			y4=effwidth4*y;
			y3=dwEffWidth*y;
			for (long x=dwWidth-1; x>=0; x--){
				x4 = 4*x+y4;
				x3 = 3*x+y3;
				p[  x3]=src[  x4];
				p[1+x3]=src[1+x4];
				p[2+x3]=src[2+x4];
			}
		}
	}*/

	}
	return;
}

bool CBaseImage::DibReadBitmapInfo(u8** offset, BITMAPINFOHEADER *pdib)
{
	if ( offset==NULL || pdib==NULL ) return false;

//	u32 dwTemp;
//	ReadFile(fh,pdib,sizeof(BITMAPINFOHEADER),&dwTemp,NULL);
	memcpy(pdib,*offset,sizeof(BITMAPINFOHEADER));  *offset += sizeof(BITMAPINFOHEADER);

    BITMAPCOREHEADER   bc;

    switch (pdib->biSize) // what type of bitmap info is this?
    {
        case sizeof(BITMAPINFOHEADER):
            break;

        case sizeof(BITMAPCOREHEADER):
            bc = *(BITMAPCOREHEADER*)pdib;
            pdib->biSize               = bc.bcSize;
            pdib->biWidth              = (u32)bc.bcWidth;
            pdib->biHeight             = (u32)bc.bcHeight;
            pdib->biPlanes             =  bc.bcPlanes;
            pdib->biBitCount           =  bc.bcBitCount;
            pdib->biCompression        = BI_RGB;
            pdib->biSizeImage          = 0;
            pdib->biXPelsPerMeter      = 0;
            pdib->biYPelsPerMeter      = 0;
            pdib->biClrUsed            = 0;
            pdib->biClrImportant       = 0;

			//SetFilePointer(fh,sizeof(BITMAPCOREHEADER)-sizeof(BITMAPINFOHEADER),NULL,FILE_CURRENT);
			*offset+=sizeof(BITMAPCOREHEADER)-sizeof(BITMAPINFOHEADER);

            break;
        default:
			return false;
    }

//    FixBitmapInfo(pdib);

    return true;
}


//SaveImage : bmp 파일을 저장한다.
bool CBaseImage::SaveImage(const char* file)
{
	u32 size,len;

	//f(file) strcpy(m_strFileName,file);
	u8* data=SaveImageToMemory(size);
	if(data){
		FILE *fp=fopen(file,"wb");
		fwrite(data , 1, size, fp);
		fclose(fp);
		delete [] data;
		return true;
	}
	else return false;
	
}
u8* CBaseImage::SaveImageToMemory(u32& isize)
{
	if(pBitmapInfo==NULL || m_pDib==NULL) return NULL;

	u32 size;
	bmpHeader.bfType=DIB_HEADER_MARKER;
	if(m_nColor==0 ||m_nColor==32)
		bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	else
		bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+((m_nColor) * sizeof(RGBQUAD));
	int nColor=0;
	if(m_nColor==0 ||m_nColor==32) nColor = 0;
	else nColor = m_nColor;
	bmpHeader.bfSize=sizeof(bmpHeader)+((nColor-1) * sizeof(RGBQUAD)) +sizeof(BITMAPINFO)+pBitmapInfo->bmiHeader.biSizeImage;
	bmpHeader.bfReserved1=0;
	bmpHeader.bfReserved2=0;

	//size = ((m_nColor-1) * sizeof(RGBQUAD))+sizeof(BITMAPINFO);
	size = ((nColor-1) * sizeof(RGBQUAD))+sizeof(BITMAPINFO);
	isize=sizeof(bmpHeader)+size+pBitmapInfo->bmiHeader.biSizeImage;
//fprintf(stderr,"size of SaveImageToMemory : %d\n",isize);
	u8* data=new u8[isize];
	u8* offset = data;
	memcpy(offset,&bmpHeader,sizeof(bmpHeader)); offset+=(sizeof(bmpHeader));
	memcpy(offset,pBitmapInfo,size); offset+=size;
	memcpy(offset,m_pDib,pBitmapInfo->bmiHeader.biSizeImage); offset+=pBitmapInfo->bmiHeader.biSizeImage;

	return data;

}
/*
void CBaseImage::SaveImageToMemory(u32& isize,u8* data)
{
	if(pBitmapInfo==NULL || m_pDib==NULL) return NULL;

	u32 size;
	bmpHeader.bfType=DIB_HEADER_MARKER;
	if(m_nColor==0 ||m_nColor==32)
		bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	else
		bmpHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+((m_nColor) * sizeof(RGBQUAD));
	int nColor=0;
	if(m_nColor==0 ||m_nColor==32) nColor = 0;
	else nColor = m_nColor;
	bmpHeader.bfSize=sizeof(bmpHeader)+((nColor-1) * sizeof(RGBQUAD)) +sizeof(BITMAPINFO)+pBitmapInfo->bmiHeader.biSizeImage;
	bmpHeader.bfReserved1=0;
	bmpHeader.bfReserved2=0;

	//size = ((m_nColor-1) * sizeof(RGBQUAD))+sizeof(BITMAPINFO);
	size = ((nColor-1) * sizeof(RGBQUAD))+sizeof(BITMAPINFO);
	isize=sizeof(bmpHeader)+size+pBitmapInfo->bmiHeader.biSizeImage;
//fprintf(stderr,"size of SaveImageToMemory : %d\n",isize);
	//data=new u8[isize];
	memset(data,0,isize);
	u8* offset = data;
	memcpy(offset,&bmpHeader,sizeof(bmpHeader)); offset+=(sizeof(bmpHeader));
	memcpy(offset,pBitmapInfo,size); offset+=size;
	memcpy(offset,m_pDib,pBitmapInfo->bmiHeader.biSizeImage); offset+=pBitmapInfo->bmiHeader.biSizeImage;

	
}*/
//BMP파일을 리사이즈 하는 함수
void CBaseImage::ImageResize(int w,int h, int flag) //24비트 이미지 일 경우만 적용
{
	if(GetSize().cx == w && GetSize().cy == h) return;

	if(pBitmapInfo==NULL || m_pDib==NULL) return;
	// TODO: Add your command handler code here
	int width=pBitmapInfo->bmiHeader.biWidth;
	int height=pBitmapInfo->bmiHeader.biHeight;

	u8* DIB= new u8[pBitmapInfo->bmiHeader.biSizeImage];
	memcpy(DIB,m_pDib,pBitmapInfo->bmiHeader.biSizeImage);

	SIZE size; size.cx=w; size.cy=h;
	int nColor=0;
	if(pBitmapInfo->bmiHeader.biBitCount==24) nColor=0;
	else if(pBitmapInfo->bmiHeader.biBitCount==32) nColor=32;
	else nColor=0;
	
	if(!SetBmpInfo(size,nColor))	return;

	if(m_pDib) { delete [] m_pDib;	m_pDib=NULL; }
	m_pDib= new u8 [pBitmapInfo->bmiHeader.biSizeImage];

	ImageRsize(DIB,width,height,(u8*)m_pDib,w,h,flag);
//	ImageResample(DIB,width,height,(u8*)m_pDib,w,h);

	delete [] DIB;
}

//GetSize : 이미지의 크기를 알아낸다.
SIZE CBaseImage::GetSize()
{
	SIZE size;
	size.cx=pBitmapInfo->bmiHeader.biWidth;
	size.cy=pBitmapInfo->bmiHeader.biHeight;
	return size;
}

//SetBmpInfo : szBitmapInfo에 값을 설정하는 함수
//버전업: 16칼라 2칼라 각각 가로 4와 8보다 작은 이미지 설정안됨
bool CBaseImage::SetBmpInfo(SIZE size, int colornum)
{
	if(!AllocBmpInfo(colornum)) return false;

	//bmiHeader의 구조체 크기 설정
	pBitmapInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	//이미지 가로크기
	pBitmapInfo->bmiHeader.biWidth=size.cx;
	// 세로크기
	pBitmapInfo->bmiHeader.biHeight=size.cy;
	//비트플렌은 1
	pBitmapInfo->bmiHeader.biPlanes=1;

	switch(colornum)
	{
	case 0:{
			pBitmapInfo->bmiHeader.biBitCount=24;//한 픽셀길이가 24 bit 
			//가로길이 4배수 나머지 값 얻기
			m_ndumy=(size.cx*3)%4;
			((4-m_ndumy)==4)? m_ndumy=0:m_ndumy=4-m_ndumy;
			pBitmapInfo->bmiHeader.biSizeImage=((size.cx*3)+m_ndumy)*size.cy;
			m_nColor=0;
			break;
		   }
	case 32:{
			pBitmapInfo->bmiHeader.biBitCount=32;//한 픽셀길이가 32 bit 
			//가로길이 4배수 나머지 값 얻기
			m_ndumy=(size.cx*4)%4;
			((4-m_ndumy)==4)? m_ndumy=0:m_ndumy=4-m_ndumy;
			pBitmapInfo->bmiHeader.biSizeImage=((size.cx*4)+m_ndumy)*size.cy;
			m_nColor=32;
			break;
		   }
	case 256:
			pBitmapInfo->bmiHeader.biBitCount=8;
			//가로길이 4배수 나머지 값 얻기
			m_ndumy=(size.cx)%4;
			((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
			pBitmapInfo->bmiHeader.biSizeImage=(size.cx+m_ndumy)*size.cy;
			m_nColor=256;
			break;
	case 16:
			pBitmapInfo->bmiHeader.biBitCount=4;
			//가로길이 4배수 나머지 값 얻기
			//가로크기가 1이면 에러 
			if(size.cx<2)
				return false;
			m_ndumy=(size.cx/2)%4;
			((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
			pBitmapInfo->bmiHeader.biSizeImage=((size.cx/2)+m_ndumy)*size.cy;
			m_nColor=16;
			break;
	case 2:
			pBitmapInfo->bmiHeader.biBitCount=1;
			//가로길이 4배수 나머지 값 얻기
			if(size.cy<8) return false;
			m_ndumy=(size.cx/8)%4;
			((4-m_ndumy)==4)?m_ndumy=0:m_ndumy=4-m_ndumy;
			pBitmapInfo->bmiHeader.biSizeImage=((size.cx/8)+m_ndumy)*size.cy;
			m_nColor=2;
			break;
	default:
			return false;
	}
	pBitmapInfo->bmiHeader.biCompression=BI_RGB;
	//차후에 기록할 예정
	pBitmapInfo->bmiHeader.biXPelsPerMeter=0;
	pBitmapInfo->bmiHeader.biYPelsPerMeter=0;
	pBitmapInfo->bmiHeader.biClrUsed =0;
	pBitmapInfo->bmiHeader.biClrImportant =0;

	return true;
}

//AllocBmpInfo : pBitmapInfo를 메모리에 설정하는 함수
bool CBaseImage::AllocBmpInfo(int colornum)
{
	//if(colornum==32) colornum=0;
	int size;
	if(pBitmapInfo){
		delete pBitmapInfo;
		pBitmapInfo=NULL;
	}
	//size=sizeof(BITMAPINFO)+(sizeof(RGBQUAD)*colornum);
	size=sizeof(BITMAPINFO)+(sizeof(RGBQUAD)*0);
	pBitmapInfo =(LPBITMAPINFO) new char[size];
	if(pBitmapInfo==NULL)
		return false;
	return true;
}

void CBaseImage::makeTransColor()
{
	int w=pBitmapInfo->bmiHeader.biWidth;
	int h=pBitmapInfo->bmiHeader.biHeight;

	int i,j;
	COLORREF color;
	for(i=0;i<h;i++){
		for(j=0;j<w;j++){
			color=GetPixelColor(j,i);
			u8 r=GetRValue(color);
			u8 g=GetGValue(color);
			u8 b=GetBValue(color);

			if( (r==255 && g==255 && b==255) || (r<10 && g>240 && b>240))
				SetPixelColor(j,i,RGB(0,0,0));
		}
	}
}


void CBaseImage::SetPaletteInfo()
{
	if(pBitmapInfo==NULL) return;

    u8* lpPal = (u8*) pBitmapInfo;

    u32 offset = sizeof(BITMAPINFOHEADER);
	int idx;
	std::map<COLORREF, u8>::iterator it = m_palette.begin();
	while (it != m_palette.end()){
		idx=(*it).second;
		offset = sizeof(BITMAPINFOHEADER)+idx*4;
        lpPal[offset++] = GetBValue((*it).first);
        lpPal[offset++] = GetGValue((*it).first);
        lpPal[offset++] = GetRValue((*it).first);
        lpPal[offset++] = 0x00;
		it++;
	}
}

//CreatePalette : mode가 1이면 pBitmapInfo 에서 불러오고 그렇지 않으면 생성한다.
bool CBaseImage::CreatePalette(bool mode)
{
//	int r,g,b;
	if(lpLogPalette) 
		delete lpLogPalette;
	lpLogPalette=NULL;

	lpLogPalette=(LPLOGPALETTE) new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*m_nColor];
	if(lpLogPalette == NULL)
		return false;
	lpLogPalette->palVersion=PALVERSION;
	lpLogPalette->palNumEntries=m_nColor;

	if(mode)
	{
		for(int i=0;i<m_nColor;i++)
		{
			lpLogPalette->palPalEntry[i].peRed=pBitmapInfo->bmiColors[i].rgbRed;
			lpLogPalette->palPalEntry[i].peGreen=pBitmapInfo->bmiColors[i].rgbGreen;
			lpLogPalette->palPalEntry[i].peBlue=pBitmapInfo->bmiColors[i].rgbBlue;
		}
	}
	else
	{
/*
		int i=0;
		for(i=0;i<256;i++)
		{
			lpLogPalette->palPalEntry[i].peRed=gDefaultPalette[i][0];
			pBitmapInfo->bmiColors[i].rgbRed=gDefaultPalette[i][0];
			lpLogPalette->palPalEntry[i].peGreen=gDefaultPalette[i][1];
			pBitmapInfo->bmiColors[i].rgbGreen=gDefaultPalette[i][1];
			lpLogPalette->palPalEntry[i].peBlue=gDefaultPalette[i][2];
			pBitmapInfo->bmiColors[i].rgbBlue=gDefaultPalette[i][2];
			if(i>m_nColor)
				return true;
		}
*/
	}
	return true;
}

//FindColorInPalette : 팔레트 안에서 fi 의 색이 있는가를 찾는다.
int CBaseImage::FindColorInPalette(COLORREF fi)
{
	for(int i=0;i<m_nColor;i++)
	{
		if( (lpLogPalette->palPalEntry[i].peRed == GetRValue(fi)) &&
			(lpLogPalette->palPalEntry[i].peGreen == GetGValue(fi)) &&
			(lpLogPalette->palPalEntry[i].peBlue == GetBValue(fi)) )
				return i;
	}
	return -1;
}



COLORREF CBaseImage::GetPixelColor(int cx,int cy)
{
	if(m_pDib==NULL) return RGB(0,0,0);
	
	SIZE size=GetSize();
	if(m_nColor==0){
		u8 r=m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+2];
		u8 g=m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+1];
		u8 b=m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+0];
		return RGB(r,g,b);
	}
	else if(m_nColor==32){
		u8 a=m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+3];
		u8 r=m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+2];
		u8 g=m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+1];
		u8 b=m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+0];
		return RGBA(r,g,b,a);
	}
	return RGBA(0,0,0,0);
}

void CBaseImage::SetPixelColor(int cx,int cy,COLORREF color)
{
	if(m_pDib==NULL) return;
	
	SIZE size=GetSize();
	if(m_nColor==0){
		u8 r=GetRValue(color);
		u8 g=GetGValue(color);
		u8 b=GetBValue(color);
		m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+0]=b;
		m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+1]=g;
		m_pDib[cy*(size.cx*3+m_ndumy)+cx*3+2]=r;
	}
	else if(m_nColor==32){
		u8 r=GetRValue(color);
		u8 g=GetGValue(color);
		u8 b=GetBValue(color);
		u8 a=GetAValue(color);
		m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+0]=b;
		m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+1]=g;
		m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+2]=r;
		m_pDib[cy*(size.cx*4+m_ndumy)+cx*4+3]=a;
	}
	else if(m_nColor==256){
		int idx=0;
		std::map<COLORREF, u8>::iterator it = m_palette.find(color);
		if (it != m_palette.end()) idx=(*it).second;
		m_pDib[cy*(size.cx+m_ndumy)+cx]=idx;
	}
}

// 32bit 이미지를 tga 포맷으로 변경한다. soo 20100701
u8* CBaseImage::Save32bitTGAMemory( u32& isize, COLORREF colorkey )
{
    struct TargaHeader
    {
        u8 IDLength;
        u8 ColormapType;
        u8 ImageType;
        u8 ColormapSpecification[5];
        u16 XOrigin;
        u16 YOrigin;
        u16 ImageWidth;
        u16 ImageHeight;
        u8 PixelDepth;
        u8 ImageDescriptor;
    } tga;
	
	memset(&tga,0,sizeof(TargaHeader));
	
	SIZE size = GetSize();

	tga.IDLength = 0;
	tga.ColormapType = 0;
	tga.ImageType = 2;
	tga.ImageDescriptor = (1 << 5);
	
	tga.ImageWidth = size.cx;
	tga.ImageHeight = size.cy;
	tga.PixelDepth = 32;
	
	isize = sizeof(TargaHeader) + size.cx*size.cy*sizeof(u32);
	u8* data = new u8[isize];
	
	u8* offset = data;
	// tga header 저장
    memcpy( offset, &tga, sizeof(TargaHeader) );  offset+=sizeof(TargaHeader);
	
	u32* m_pRGBAData; // 이미지 정보
    m_pRGBAData = new u32[size.cx*size.cy];
	
	int x,y;
    for( y=0; y<size.cy; y++ )
    {
        u32 dwOffset = y*size.cy;
		
        if( 0 == ( tga.ImageDescriptor & 0x0010 ) )
            dwOffset = (size.cy-y-1)*size.cx;
		
		for( x=0; x<size.cx; x )
        {
			COLORREF scolor=GetPixelColor(x,y);
			int m=((size.cy-y-1)*size.cx+x)*4;
            u8 b = GetBValue(scolor);
            u8 g = GetGValue(scolor);
            u8 r = GetRValue(scolor);
            u8 a = 255;
			if( scolor == colorkey ) a=0;
			if( scolor == RGB(0,0,0) ) a=0;
//          m_pRGBAData[dwOffset+x] = (r<<24L)+(g<<16L)+(b<<8L)+(a);
			m_pRGBAData[dwOffset+x] = (a<<24L)+(r<<16L)+(g<<8L)+(b);
            x++;
        }
    }
	memcpy(offset,m_pRGBAData,sizeof(u32)*size.cx*size.cy);
	
	delete [] m_pRGBAData;

	return data;
}

float b3spline(float x)
{
	float a, b, c, d;

	if((x + 2.0f) <= 0.0f) a = 0.0f; else a = (float)pow((x + 2.0f), 3.0f);
	if((x + 1.0f) <= 0.0f) b = 0.0f; else b = (float)pow((x + 1.0f), 3.0f);
	if(x <= 0) c = 0.0f; else c = (float)pow(x, 3.0f);  
	if((x - 1.0f) <= 0.0f) d = 0.0f; else d = (float)pow((x - 1.0f), 3.0f);

	return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));
}
///////////////////////////////////////////////////////////////////
//이미지 크기조정 함수(24bit 이미지)
///////////////////////////////////////////////////////////////////
//soo 20100701 수정
void CBaseImage::ImageRsize(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH, int flag) 
{
	int nResultRowBytes = ((((SW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	int nResultRowBytes_DW = ((((DW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	if(m_nColor==0) {
		nResultRowBytes = ((((SW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
		nResultRowBytes_DW = ((((DW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	}
	else if(m_nColor==32) {
		nResultRowBytes = ((((SW * 32) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
		nResultRowBytes_DW = ((((DW * 32) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	}
		

	float xScale = (float)SW / (float)DW;
	float yScale = (float)SH / (float)DH;
//	str.Format("xScale %f,yScale %f",xScale,yScale);
//	AfxMessageBox(str);
	u8 R,G,B,A;
//flag 2 는 원본 < 대상 즉, 확대시 사용 , 안그러면 분홍색 선 같은 것이 보인다.
//flag 0,1은 원본, 대상 크기 관계 없이 사용가능 그러나 확대에 사용할 시 보기 안 좋음 
//	if(SW < DW) flag=2;
//	else flag=0; //1 은 속도 느림
	//if(SW < DW && SH < DH ) flag=0; // jsyun 무조건 확대는 0번 --> 퀄리티 구려서 0번 은 안함, 대신 속도 느려도 1번으로 하는게 좋음. jsyun 20170803
	if( flag == 0 )  // nearest pixel 제일 간단(fastest)	, 정말 퀄리티 구리다!				
	{
		long fX,fY;
		for(long y=0; y<DH; y++){
			fY = y * yScale;
			for(long x=0; x<DW; x++){
				fX = x * xScale;
				if(m_nColor==0) {
					B = sDIB[fY*nResultRowBytes+fX*3];
					G = sDIB[fY*nResultRowBytes+fX*3+1];
					R = sDIB[fY*nResultRowBytes+fX*3+2];
					dDIB[y*nResultRowBytes_DW+x*3] = B;
					dDIB[y*nResultRowBytes_DW+x*3+1] = G;
					dDIB[y*nResultRowBytes_DW+x*3+2] = R;
				}
				else if(m_nColor==32) {
					B = sDIB[fY*nResultRowBytes+fX*4];
					G = sDIB[fY*nResultRowBytes+fX*4+1];
					R = sDIB[fY*nResultRowBytes+fX*4+2];
					A = sDIB[fY*nResultRowBytes+fX*4+3];
					dDIB[y*nResultRowBytes_DW+x*4] = B;
					dDIB[y*nResultRowBytes_DW+x*4+1] = G;
					dDIB[y*nResultRowBytes_DW+x*4+2] = R;
					dDIB[y*nResultRowBytes_DW+x*4+3] = A;
				}
			}
		}
	}
	else if( flag == 1 )// bicubic interpolation by Blake L. Carlson <blake-carlson(at)uiowa(dot)edu // 속도가 왜이렇게 느리지?
	{ // 시간 너무 오래 걸린다.
		float f_x, f_y, a, b, rr, gg, bb,aa, r1, r2;
		int   i_x, i_y, xx, yy;

		for(long y=0; y<DH; y++){
//			info.nProgress = (long)(100*y/newy);
//			if (info.nEscape) break;
			f_y = (float) y * yScale;
			i_y = (int) floor(f_y);
			a   = f_y - (float)floor(f_y);
			for(long x=0; x<DW; x++){
				f_x = (float) x * xScale;
				i_x = (int) floor(f_x);
				b   = f_x - (float)floor(f_x);

				rr = gg = bb = 0.0F;
				for(int m=-1; m<3; m++) {
					r1 = b3spline((float) m - a);
					for(int n=-1; n<3; n++) {
						r2 = b3spline(-1.0F*((float)n - b)); 
//						xx = i_x+n+2;
//						yy = i_y+m+2;
						xx = i_x+n;
						yy = i_y+m;
						if (xx<0) xx=0;
						if (yy<0) yy=0;
						if (xx>=SW) xx=SW-1;
						if (yy>=SH) yy=SH-1;
						if(m_nColor==0) {
							B = sDIB[yy*nResultRowBytes+xx*3];
							G = sDIB[yy*nResultRowBytes+xx*3+1];
							R = sDIB[yy*nResultRowBytes+xx*3+2];
	//						rgb = GetPixelColor(xx,yy);
							rr += R * r1 * r2;
							gg += G * r1 * r2;
							bb += B * r1 * r2;
						}
						else if(m_nColor==32) {
							B = sDIB[yy*nResultRowBytes+xx*4];
							G = sDIB[yy*nResultRowBytes+xx*4+1];
							R = sDIB[yy*nResultRowBytes+xx*4+2];
							A = sDIB[yy*nResultRowBytes+xx*4+3];
							rr += R * r1 * r2;
							gg += G * r1 * r2;
							bb += B * r1 * r2;
							aa += A * r1 * r2;
						}
					}
				}
				if(m_nColor==0) {
					dDIB[y*nResultRowBytes_DW+x*3] = bb;
					dDIB[y*nResultRowBytes_DW+x*3+1] = gg;
					dDIB[y*nResultRowBytes_DW+x*3+2] = rr;
	//				newImage.SetPixelColor(x,y,RGB(rr,gg,bb));
				}
				else if(m_nColor==32) {
					dDIB[y*nResultRowBytes_DW+x*4] = bb;
					dDIB[y*nResultRowBytes_DW+x*4+1] = gg;
					dDIB[y*nResultRowBytes_DW+x*4+2] = rr;
					dDIB[y*nResultRowBytes_DW+x*4+3] = aa;
				}
			}
		}
	}else{
		ImageResample(sDIB,SW,SH,dDIB,DW,DH);
	}
}

void CBaseImage::ImageResample(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH) 
{
	int nResultRowBytes = ((((SW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	int nResultRowBytes_DW = ((((DW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	
	if(m_nColor==0) {
		nResultRowBytes = ((((SW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
		nResultRowBytes_DW = ((((DW * 24) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	}
	else if(m_nColor==32) {
		nResultRowBytes = ((((SW * 32) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
		nResultRowBytes_DW = ((((DW * 32) + 31) & ~31) / 8);//전체박스의 한줄의 byte사이즈
	}
		
	float xScale = (float)SW / (float)DW;
	float yScale = (float)SH / (float)DH;
//	str.Format("xScale %f,yScale %f",xScale,yScale);
//	AfxMessageBox(str);
//	u8 R,G,B;

	// bilinear interpolation
	
	if ( SW < DW && (m_nColor==0 || m_nColor==32) ) // 원본보다 클때 사용한다.
	{	//1999 Steve McMahon (steve@dogma.demon.co.uk)
//_debug("bilinear interpolation");
		float fX,fY;
		long ifX, ifY, ifX1, ifY1, xmax, ymax;
		float ir1, ir2, ig1, ig2, ib1, ib2, dx, dy,ia1, ia2;
		u8 r,g,b,a;
		RGBQUAD rgb1, rgb2, rgb3, rgb4;
		xmax = SW-1;
		ymax = SH-1;
		for(long y=0; y<DH; y++){
//			info.nProgress = (long)(100*y/newy);
//			if (info.nEscape) break;
			fY = y * yScale;
			ifY = (int)fY;
			ifY1 = min(ymax, ifY+1);
			dy = fY - ifY;
			for(long x=0; x<DW; x++){
				fX = x * xScale;
				ifX = (int)fX;
				ifX1 = min(xmax, ifX+1);
				dx = fX - ifX;
				// Interpolate using the four nearest pixels in the source
/*				if (head.biClrUsed){
					rgb1=GetPaletteColor(GetPixelIndex(ifX,ifY));
					rgb2=GetPaletteColor(GetPixelIndex(ifX1,ifY));
					rgb3=GetPaletteColor(GetPixelIndex(ifX,ifY1));
					rgb4=GetPaletteColor(GetPixelIndex(ifX1,ifY1));
				}
				else {*/
			//pSource = info.pImage + yy * info.dwEffWidth;
			//pSource = (sDIB+yy*nResultRowBytes);
					u8* iDst;
					if(m_nColor==0 ) {
						iDst = sDIB + ifY*nResultRowBytes + ifX*3; //(x, y) = (x, y) + (0, 0)
						rgb1.rgbBlue = *iDst++;	rgb1.rgbGreen= *iDst++;	rgb1.rgbRed =*iDst;
						iDst = sDIB + ifY*nResultRowBytes + ifX1*3;//(x + 1, y) = (x, y) + (1, 0)
						rgb2.rgbBlue = *iDst++;	rgb2.rgbGreen= *iDst++;	rgb2.rgbRed =*iDst;
						iDst = sDIB + ifY1*nResultRowBytes + ifX*3;//(x , y + 1) = (x, y) + (0, 1)
						rgb3.rgbBlue = *iDst++;	rgb3.rgbGreen= *iDst++;	rgb3.rgbRed =*iDst;
						iDst = sDIB + ifY1*nResultRowBytes + ifX1*3;//(x + 1, y + 1) = (x, y) + (1, 1)
						rgb4.rgbBlue = *iDst++;	rgb4.rgbGreen= *iDst++;	rgb4.rgbRed =*iDst;
					}
					else if(m_nColor==32) {
						iDst = sDIB + ifY*nResultRowBytes + ifX*4; //(x, y) = (x, y) + (0, 0)
						rgb1.rgbBlue = *iDst++;	rgb1.rgbGreen= *iDst++;	rgb1.rgbRed =*iDst++; rgb1.rgbReserved =*iDst;
						iDst = sDIB + ifY*nResultRowBytes + ifX1*4;//(x + 1, y) = (x, y) + (1, 0)
						rgb2.rgbBlue = *iDst++;	rgb2.rgbGreen= *iDst++;	rgb2.rgbRed =*iDst++; rgb1.rgbReserved =*iDst;
						iDst = sDIB + ifY1*nResultRowBytes + ifX*4;//(x , y + 1) = (x, y) + (0, 1)
						rgb3.rgbBlue = *iDst++;	rgb3.rgbGreen= *iDst++;	rgb3.rgbRed =*iDst++; rgb1.rgbReserved =*iDst;
						iDst = sDIB + ifY1*nResultRowBytes + ifX1*4;//(x + 1, y + 1) = (x, y) + (1, 1)
						rgb4.rgbBlue = *iDst++;	rgb4.rgbGreen= *iDst++;	rgb4.rgbRed =*iDst++; rgb1.rgbReserved =*iDst;					
					}
//				}
				// Interplate in x direction:
				ir1 = rgb1.rgbRed   * (1 - dy) + rgb3.rgbRed   * dy;
				ig1 = rgb1.rgbGreen * (1 - dy) + rgb3.rgbGreen * dy;
				ib1 = rgb1.rgbBlue  * (1 - dy) + rgb3.rgbBlue  * dy;
				if(m_nColor==32) ia1 = rgb1.rgbReserved  * (1 - dy) + rgb3.rgbReserved  * dy;
				
				ir2 = rgb2.rgbRed   * (1 - dy) + rgb4.rgbRed   * dy;
				ig2 = rgb2.rgbGreen * (1 - dy) + rgb4.rgbGreen * dy;
				ib2 = rgb2.rgbBlue  * (1 - dy) + rgb4.rgbBlue  * dy;
				if(m_nColor==32) ia2 = rgb2.rgbReserved  * (1 - dy) + rgb4.rgbReserved  * dy;
				// Interpolate in y:
				r = (u8)(ir1 * (1 - dx) + ir2 * dx);
				g = (u8)(ig1 * (1 - dx) + ig2 * dx);
				b = (u8)(ib1 * (1 - dx) + ib2 * dx);
				if(m_nColor==32) a = (u8)(ia1 * (1 - dx) + ia2 * dx);

/*
				r = (u8)( BOUND( ((1 - dx) * (1 - dy) * ((float)rgb1.rgbRed) + 
				      (1 - dx) * dy * ((float)rgb3.rgbRed) + 
					  dx * (1 - dy) * ((float)rgb2.rgbRed) +
					  dx * dy * ((float)rgb4.rgbRed)),0,255) );
				g = (u8)( BOUND( ((1 - dx) * (1 - dy) * ((float)rgb1.rgbGreen) + 
				      (1 - dx) * dy * ((float)rgb3.rgbGreen) + 
					  dx * (1 - dy) * ((float)rgb2.rgbGreen) +
					  dx * dy * ((float)rgb4.rgbGreen)),0,255) );
				b = (u8)( BOUND( ((1 - dx) * (1 - dy) * ((float)rgb1.rgbBlue) + 
				      (1 - dx) * dy * ((float)rgb3.rgbBlue) + 
					  dx * (1 - dy) * ((float)rgb2.rgbBlue) +
					  dx * dy * ((float)rgb4.rgbBlue)),0,255) );
*/				
				// Set output
				if(m_nColor==0 ) {
					dDIB[y*nResultRowBytes_DW+x*3] = b;
					dDIB[y*nResultRowBytes_DW+x*3+1] = g;
					dDIB[y*nResultRowBytes_DW+x*3+2] = r;
				}
				else if(m_nColor==32) {
					dDIB[y*nResultRowBytes_DW+x*4] = b;
					dDIB[y*nResultRowBytes_DW+x*4+1] = g;
					dDIB[y*nResultRowBytes_DW+x*4+2] = r;
					dDIB[y*nResultRowBytes_DW+x*4+3] = a;
				}
			}
		} 
	} else { // 원본보다 작을 때 사용한다.
//_debug("high resolution shrink");
		/////////////////////////////////////////////
		//high resolution shrink, thanks to Henrik Stellmann <henrik.stellmann@volleynet.de>
		/////////////////////////////////////////////
		const long ACCURACY = 1000;
		long i_x,j_y; // index for faValue
		long xx,yy; // coordinates in  source image
		u8* pSource;
		u8* pDest = dDIB;
		long* naAccu  = 0;
		long* naCarry = 0;
		if(m_nColor==0) {
			naAccu  =  new long[3 * DW + 3];
			naCarry = new long[3 * DW + 3];
			memset(naAccu,  0, sizeof(long) * 3 * DW);
			memset(naCarry, 0, sizeof(long) * 3 * DW);
		}
		else if(m_nColor==32){
			naAccu  =  new long[4 * DW + 4];
			naCarry = new long[4 * DW + 4];
			memset(naAccu,  0, sizeof(long) * 4 * DW);
			memset(naCarry, 0, sizeof(long) * 4 * DW);
		}
		long* naTemp;
		long  nWeightX,nWeightY;
		float fEndX;
		float xScale, yScale;
		xScale = (float)(SW) / (float)DW;
		yScale = (float)(SH) / (float)DH;
		long nScale = (long)(ACCURACY * xScale * yScale);


		int u, v = 0; // coordinates in dest image

		float fEndY = yScale - 1.0f;
		u8 colorRGB;
		if(m_nColor==0) {
			for (yy = 0; yy < SH; yy++){
				//pSource = info.pImage + yy * info.dwEffWidth;
				pSource = (sDIB+yy*nResultRowBytes);
				u = i_x = 0;
				fEndX = xScale - 1.0f;
				//s.Format("fEndX=%f,xScale=%f,yy=%d",fEndX,xScale,yy);
				//AfxMessageBox(s);
				if ((float)yy < fEndY) { // complete source row goes into dest row
					for (xx = 0; xx < SW; xx++){
						if ((float)xx < fEndX){ // complete source pixel goes into dest pixel
							for (j_y = 0; j_y < 3; j_y++)	naAccu[i_x + j_y] += (*pSource++) * ACCURACY;
						} else {       // source pixel is splitted for 2 dest pixels
							nWeightX = (long)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 3; j_y++){
								naAccu[i_x] += (ACCURACY - nWeightX) * (*pSource);
								naAccu[3 + i_x++] += nWeightX * (*pSource++);
							}
							fEndX += xScale;
							u++;
						}
					}
				} else {       // source row is splitted for 2 dest rows       
					nWeightY = (long)(((float)yy - fEndY) * ACCURACY);
					for (xx = 0; xx < SW; xx++){
						if ((float)xx < fEndX){       // complete source pixel goes into 2 pixel
							for (j_y = 0; j_y < 3; j_y++){
								naAccu[i_x + j_y] += ((ACCURACY - nWeightY) * (*pSource));
								naCarry[i_x + j_y] += nWeightY * (*pSource++);
							}
						} else {       // source pixel is splitted for 4 dest pixels
							nWeightX = (int)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 3; j_y++) {
								naAccu[i_x] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
								colorRGB = (u8)(naAccu[i_x] / nScale);
								//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
								*pDest++ = colorRGB;
								naCarry[i_x] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
								naAccu[i_x + 3] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
								naCarry[i_x + 3] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
								i_x++;
								pSource++;
							}
							fEndX += xScale;
							u++;
						}
					}
					if (u < DW){ // possibly not completed due to rounding errors
						for (j_y = 0; j_y < 3; j_y++) {
							colorRGB = (u8)(naAccu[i_x++] / nScale);
							//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
							*pDest++ = colorRGB;
						}
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					memset(naCarry, 0, sizeof(int) * 3);    // need only to set first pixel zero
					pDest = dDIB + (++v * nResultRowBytes_DW);
					fEndY += yScale;
				}
			}
			if (v < DH){	// possibly not completed due to rounding errors
				for (i_x = 0; i_x < 3 * DW; i_x++) {
					colorRGB = (u8)(naAccu[i_x] / nScale);
					//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
					*pDest++ = colorRGB;
				}
			}
		}
		else if(m_nColor==32){
			for (yy = 0; yy < SH; yy++){
				//pSource = info.pImage + yy * info.dwEffWidth;
				pSource = (sDIB+yy*nResultRowBytes);
				u = i_x = 0;
				fEndX = xScale - 1.0f;
				//s.Format("fEndX=%f,xScale=%f,yy=%d",fEndX,xScale,yy);
				//AfxMessageBox(s);
				if ((float)yy < fEndY) { // complete source row goes into dest row
					for (xx = 0; xx < SW; xx++){
						if ((float)xx < fEndX){ // complete source pixel goes into dest pixel
							for (j_y = 0; j_y < 4; j_y++)	naAccu[i_x + j_y] += (*pSource++) * ACCURACY;
						} else {       // source pixel is splitted for 2 dest pixels
							nWeightX = (long)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 4; j_y++){
								naAccu[i_x] += (ACCURACY - nWeightX) * (*pSource);
								naAccu[4 + i_x++] += nWeightX * (*pSource++);
							}
							fEndX += xScale;
							u++;
						}
					}
				} else {       // source row is splitted for 2 dest rows       
					nWeightY = (long)(((float)yy - fEndY) * ACCURACY);
					for (xx = 0; xx < SW; xx++){
						if ((float)xx < fEndX){       // complete source pixel goes into 2 pixel
							for (j_y = 0; j_y < 4; j_y++){
								naAccu[i_x + j_y] += ((ACCURACY - nWeightY) * (*pSource));
								naCarry[i_x + j_y] += nWeightY * (*pSource++);
							}
						} else {       // source pixel is splitted for 4 dest pixels
							nWeightX = (int)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 4; j_y++) {
								naAccu[i_x] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
								colorRGB = (u8)(naAccu[i_x] / nScale);
								//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
								*pDest++ = colorRGB;
								naCarry[i_x] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
								naAccu[i_x + 4] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
								naCarry[i_x + 4] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
								i_x++;
								pSource++;
							}
							fEndX += xScale;
							u++;
						}
					}
					if (u < DW){ // possibly not completed due to rounding errors
						for (j_y = 0; j_y < 4; j_y++) {
							colorRGB = (u8)(naAccu[i_x++] / nScale);
							//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
							*pDest++ = colorRGB;
						}
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					memset(naCarry, 0, sizeof(int) * 4);    // need only to set first pixel zero
					pDest = dDIB + (++v * nResultRowBytes_DW);
					fEndY += yScale;
				}
			}
			if (v < DH){	// possibly not completed due to rounding errors
				for (i_x = 0; i_x < 4 * DW; i_x++) {
					colorRGB = (u8)(naAccu[i_x] / nScale);
					//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
					*pDest++ = colorRGB;
				}
			}			
		}
		delete [] naAccu;
		delete [] naCarry;
	}

}
