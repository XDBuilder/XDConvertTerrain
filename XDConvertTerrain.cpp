#include "common.h"
#include <string>
#include <algorithm> 
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

//fork 용
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "base/rect2d.h"
#include "XDRasterAPI.h"
#include "XDProjAPI.h"
#include "gdalManage.h"
#include "images/geotiff/geotiffio.h"
#include "images/geotiff/xtiffio.h"
#include "images/geotiff/geo_normalize.h"
#include "images/geotiff/geo_tiffp.h"
#include "images/geotiff/geo_keyp.h"
#include "images/Baseimage.h"
#include "images/CImage.h"
#include "images/CImagePNG.h"


#include "gdal/include/ogrsf_frmts.h"
#include "gdal/include/gdal_alg.h"
#include "gdal/include/cpl_string.h"
#include "gdal/include/cpl_conv.h"
#include "gdal/include/ogr_srs_api.h"
#include "gdal/include/gdal_utils.h"
#include "gdal/include/gdalwarper.h"
#include "gdal/include/frmts/hfa/hfa.h" //img format reading

#include "gdal/include/gdal_utils_priv.h"
#include "gdal/include/commonutils.h"

//default 500, 작을 수록 자주 메세지 가져온다.
const int CONSUMER_POLL_BLOCK_INTERVAL_TIME = 500; 
#define ACKS "all" //0,1,all --> 메세지 던지기, 던지고 리턴 확인, 던지고 리턴 자세히 확인
time_t start_t, end_t;
void 	getEPSGString(int epsgno,char *srcCrd);
int 	getImageKind(char *filename);
int 	getImageLevel(string filePath,int epsgno);
double 	getImageResolution(string filePath);
int 	getDEMTIles(GDALDatasetH *pahSrcDS,string src, int epsg,int level, int idx,int idy,float noValue,string dest,string filename,rect2dd rectSRC);

bool 	isLonLat(double minx,double miny,double maxx,double maxy);
int produce_tilelists(string connect,string topicConvert,string topicConvertCtrl,string topicProcess, int level,rect2dd imageRect,bool bOriginLevel);
int consume_tilelists(string connect,string topicConvert,string topicConvertCtrl,string topicProcess,string groupID,string src,int epsg,string dest,rect2dd rectSRC);
int consume_tilelists_Bottom2Top(string connect,string topicConvert,string topicConvertCtrl,string topicProcess,string groupID,string src,int epsg,string dest,int fromLevel,int toLevel);

static volatile sig_atomic_t g_run = 1;
static void stop(int sig) {
    g_run = 0;
}
void LoadLibrary()
{
	XDLoadXDRaster(); // 
	XDLoadXDProj(); 
}
void UnloadLibrary()
{
	XDUnloadXDRaster(); //XDRaster unloading
	XDUnloadXDProj(); //XDProj unloading
}
void getEPSGString(int epsgno,char *srcCrd)
{
	switch(epsgno) {
		case 4326:
		sprintf(srcCrd,"%s","+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
		break;
		case 14: //보정된 오래된 지리원 표준 - 서부원점 epsg:5173
		case 5173:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=125.0028902777778 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 15: //보정된 오래된 지리원 표준 - 중부원점 epsg:5174
		case 5174:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127.0028902777778 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 16: //보정된 오래된 지리원 표준 - 동부원점 epsg:5176
		case 5176:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=129.0028902777778 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 17: //타원제 바꾼 지리원 표준 - 중부원점 epsg:5181
		case 5181:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127 +k=1 +x_0=200000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
//		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127 +k=1 +x_0=200000 +y_0=600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs");
		break;
		case 18: //타원제 바꾼 지리원 표준 - 동부원점 epsg:5183
		case 5183:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=129 +k=1 +x_0=200000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
//		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=129 +k=1 +x_0=200000 +y_0=600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs");
		break;
		case 19: //보정된 오래된 지리원 표준 - 제주원점 epsg:5175
		case 5175:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127.0028902777778 +k=1 +x_0=200000 +y_0=550000 +ellps=bessel +units=m +no_defs  +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 20: // 2017년 국토지리정보원 표준 - 중부원점 epsg:5186
		case 5186:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127 +k=1 +x_0=200000 +y_0=600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs");
		break;
		case 21: // 2017년 국토지리정보원 표준 - 동부원점 epsg:5187
		case 5187:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=129 +k=1 +x_0=200000 +y_0=600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs");
		break;
		case 24: // UTM-K (Bessel): 새주소지도에서 사용 중 epsg:5178
		case 5178:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127.5 +k=0.9996 +x_0=1000000 +y_0=2000000 +ellps=bessel +units=m +no_defs +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 25: // 네비게이션용 KATEC 좌표계(KOTI-KATEC) 
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=128 +k=0.9999 +x_0=400000 +y_0=600000 +ellps=bessel +units=m +no_defs +towgs84=-115.80,474.99,674.11,1.16,-2.31,-1.63,6.43");
		break;
		case 26: // UTM-K (GRS80): 네이버지도에서 사용중인 좌표계  epsg:5179
		case 5179:
		sprintf(srcCrd,"%s","+proj=tmerc +lat_0=38 +lon_0=127.5 +k=0.9996 +x_0=1000000 +y_0=2000000 +ellps=GRS80 +units=m +no_defs");
		break;
		case 34: // UTM Zone 51 Northern(Bessel) epsg:32651
		case 32651:
		sprintf(srcCrd,"%s","+proj=utm +zone=51 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
		break;
		case 35: // UTM Zone 52 Northern(Bessel)  epsg:32652
		case 32652:
		sprintf(srcCrd,"%s","+proj=utm +zone=52 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
		break;
	}
	//utm zone
	if(epsgno >= 36 && epsgno <= 155 ) {
		int no = (epsgno - 36) / 2 + 1;
		char zone[16];
		memset(zone,0,10);
		if(epsgno%2==0) { //Northern 
			sprintf(zone,"+zone=%d",no);
			sprintf(srcCrd,"+proj=utm %s +ellps=WGS84 +datum=WGS84 +units=m +no_defs",zone);
		}
		else { //south
			sprintf(zone,"+zone=%d +south",no);
			sprintf(srcCrd,"+proj=utm %s +ellps=WGS84 +datum=WGS84 +units=m +no_defs",zone);
		}
	}		
}

int getImageKind(char *filename)
{
	short ret=0;//0:img, 1; tif, 2: ecw , -1: no
	
	char name[512];
	memset(name,0,512);
	int len = strlen(filename);
	int i;
	for(i=0;i<len;i++) 
		name[i] = tolower(filename[i]);
	if(name[i-3]=='i' && name[i-2]=='m' && name[i-1]=='g') 
		ret=0;
	else if( (name[i-3]=='t' && name[i-2]=='i' && name[i-1]=='f') 
		||   (name[i-4]=='t' && name[i-3]=='i' && name[i-2]=='f' && name[i-1]=='f')) 
		ret=1;
	else if(name[i-3]=='e' && name[i-2]=='c' && name[i-1]=='w') 
		ret=2;
	else 
		ret =1;
	/*
	if(ret==0){
		// 확장자가 img이지만 tiff일경우
		//m_hHFA = HFAOpen( filename, "r" );
		//if(m_hHFA!=NULL) return 0;
		void *tif = pXDXTIFFOpen((char*)filename);
		if(tif!=NULL) return 1;
		
	}else if (ret==1){
		// 확장자가 tiff이지만 img일경우 TIFF체크후 IMG체크
		void *tif = pXDXTIFFOpen((char*)filename);
		if(tif!=NULL) return 1;
		//m_hHFA = HFAOpen( filename, "r" );
		//if(m_hHFA!=NULL) return 0;

	}
	*/
	return ret;
}
bool isLonLat(double minx,double miny,double maxx,double maxy) {
	if( (minx >= -180 && maxx <= 180) 
	 && (miny >= -90  && maxy <= 90 ) ) return true;
	else return false;
}
rect2dd getImageBoundary(string filePath)
{
	rect2dd imgRect(0,0,0,0);
	int kind = getImageKind((char*)filePath.c_str());
	//0:img, 1; tif, 2: ecw , -1: no
	switch(kind) {
		case 0: 
		{
			void *hdl = pXDHFAOpen(filePath.c_str(),"r");
			Eprj_MapInfo *info = pXDHFAGetMapInfo(hdl);
			imgRect.ix = info->upperLeftCenter.x ;
			imgRect.iy = info->lowerRightCenter.y;
			imgRect.ax = info->lowerRightCenter.x;
			imgRect.ay = info->upperLeftCenter.y;
			//printf("%f,%f ~ %f,%f\n",(char*)info);
		}
		break;
		case 1: {
			void *tiff = pXDXTIFFOpen((char*)filePath.c_str());
			int				dataBits;		// 데이터 크기
			int				dataType;		// 데이터 아입
			int				band;			// 밴드수  r,g,b,...
			unsigned int	imagetype;	// E_GEO_IMAGE_LOAD_TYPE
			double			resolution;	// 한픽셀 의 크기
			double			minx,miny,maxx,maxy;			// 이미지 위상학적 영역
			int				width,height;			
			pXDTIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &band );
			//1 밴드의 비트수 
			pXDTIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &dataBits );
			//원본 영상 가로세로 픽셀 수 
			pXDTIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
			pXDTIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
			void* geotiff = pXDGTIFNew( tiff );
			double x,y;
			//원본 영상 경계
			x=0.0;		y=(double)height;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Lower Left"
				minx = x;	miny = y;
			}
			
			x=(double)width;	y=0.0;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Upper Right"
				maxx = x;	maxy = y;
			}
			imgRect.ix = minx;imgRect.iy = miny;
			imgRect.ax = maxx;imgRect.ay = maxy;
			pXDXTIFFClose(tiff);
		}
		break;
		case 2:
		break;

		case -1:
		default:
		break;
	}
	printf("Boundary : (%f,%f)~(%f,%F)\n",imgRect.ix,imgRect.iy,imgRect.ax,imgRect.ay);
	return imgRect;
}
int getImageLevel(string filePath,int epsgno)
{
	int level=0;
	double resolutionDEG=0.0;
	int kind = getImageKind((char*)filePath.c_str());
	//0:img, 1; tif, 2: ecw , -1: no
	switch(kind) {
		case 0: 
		{
			void *hdl = pXDHFAOpen(filePath.c_str(),"r");
			int nXSize, nYSize,nBands;
			pXDHFAGetRasterInfo(hdl,&nXSize, &nYSize,&nBands);
			Eprj_MapInfo *info = pXDHFAGetMapInfo(hdl);
			double ix,iy,ax,ay,minx,miny,maxx,maxy;			// 이미지 위상학적 영역
			ix = info->upperLeftCenter.x ;
			iy = info->lowerRightCenter.y;
			ax = info->lowerRightCenter.x;
			ay = info->upperLeftCenter.y;
			bool bLonLat = isLonLat(ix,iy,ax,ay);
			if(bLonLat || epsgno == 4326) {  //lonlat
				resolutionDEG = (ax-ix)/nXSize;
				//resolutionDEG = info->pixelSize.width; //둘다 같음.
				//printf( "resolutionDEG 1 - %f , %f %f %d\n",resolutionDEG,ax,ix,nXSize);//info->pixelSize.width);
			}
			else { //TM-->lonlat
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(epsgno,srcCrd); 
				getEPSGString(4326,destCrd);
				double x,y;
				//minx~~ , lonlat! LL,UR,UR,LR 
				//LL
				int er = pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				minx = maxx = x;
				miny = maxy = y;
				//UL
				er = pXDProjTransform(srcCrd,destCrd,ix,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//UR
				er = pXDProjTransform(srcCrd,destCrd,ax,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//LR
				er = pXDProjTransform(srcCrd,destCrd,ax,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				resolutionDEG = (maxx-minx)/nXSize;
				//printf( "resolutionDEG 2 - %f , %f %f %d\n",resolutionDEG,ax,ix,nXSize);
			}
		}
		break;
		case 1: {
			void *tiff = pXDXTIFFOpen((char*)filePath.c_str());
			int				dataBits;		// 데이터 크기
			int				dataType;		// 데이터 아입
			int				band;			// 밴드수  r,g,b,...
			unsigned int	imagetype;	// E_GEO_IMAGE_LOAD_TYPE
			double			ix,iy,ax,ay,minx,miny,maxx,maxy;			// 이미지 위상학적 영역
			int				width,height;			
			pXDTIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &band );
			//1 밴드의 비트수 
			pXDTIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &dataBits );
			//원본 영상 가로세로 픽셀 수 
			pXDTIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
			pXDTIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
			void* geotiff = pXDGTIFNew( tiff );
			double x,y;
			//원본 영상 경계
			x=0.0;		y=(double)height;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Lower Left"
				ix = x;	iy = y;
			}
			
			x=(double)width;	y=0.0;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Upper Right"
				ax = x;	ay = y;
			}
			bool bLonLat = isLonLat(ix,iy,ax,ay);
			//printf("bound : (%f,%f)~(%f,%f)\n",ix,iy,ax,ay);
			if(bLonLat || epsgno == 4326) {  //lonlat
				int count;
				double *data;
				tiff_t *t_tif = ((GTIF*)geotiff)->gt_tif;
				if ((((GTIF*)geotiff)->gt_methods.get)(t_tif, GTIFF_PIXELSCALE, &count, &data )) {
					resolutionDEG = *data;
					//printf( "m_resolution 1 - %e , %f, %f, %d\n",resolutionDEG,ax,ix,width);
				}else {
					resolutionDEG = (ax-ix)/width;
					//printf( "m_resolution 2 - %f , %f %f %d\n",resolutionDEG,ax,ix,width);
				}
			}
			else { //TM-->lonlat
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(epsgno,srcCrd); 
				getEPSGString(4326,destCrd);
				//minx~~ , lonlat! LL,UR,UR,LR 
				//LL
				int er = pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				minx = maxx = x;
				miny = maxy = y;
				//UL
				er = pXDProjTransform(srcCrd,destCrd,ix,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//UR
				er = pXDProjTransform(srcCrd,destCrd,ax,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//LR
				er = pXDProjTransform(srcCrd,destCrd,ax,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				resolutionDEG = (maxx-minx)/width;
			}

			pXDXTIFFClose(tiff);
		}
		break;
		case 2:
		break;

		case -1:
		default:
		break;
	}	
	int permesh = 256; //image 256, dem 64
	double rootres = 36.0 / permesh; 

	double curres = rootres;
	level = 0; 
	bool realBreak = false;
	bool absoluteOK = false;
	while (true) 
	{
		curres = rootres / pow(2, level);

		if (curres < resolutionDEG) { 
		
			level++;
			break;
		}
		else if (curres == resolutionDEG) { 
		//printf("break ! level : %d\n",level);
			break;
		}
		level++;
		//printf("level : %d = %f rootres %f\n",level,curres,rootres);
	}
	return level;
}

// TM 좌표계로 변환해서 처리
double getImageResolution(string filePath)
{
	double resolution=0.0; //meter / pixel
	int kind = getImageKind((char*)filePath.c_str());
	//0:img, 1; tif, 2: ecw , -1: no
	switch(kind) {
		case 0: 
		{
			void *hdl = pXDHFAOpen(filePath.c_str(),"r");
			int nXSize, nYSize,nBands;
			pXDHFAGetRasterInfo(hdl,&nXSize, &nYSize,&nBands);
			Eprj_MapInfo *info = pXDHFAGetMapInfo(hdl);
			double ix,iy,ax,ay,minx,miny,maxx,maxy,x,y;			// 이미지 위상학적 영역
			ix = info->upperLeftCenter.x ;
			iy = info->lowerRightCenter.y;
			ax = info->lowerRightCenter.x;
			ay = info->upperLeftCenter.y;
			bool bLonLat = isLonLat(ix,iy,ax,ay);
			if(bLonLat) {  //lonlat
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(4326,srcCrd); // lonlat
				getEPSGString(5186,destCrd); //TM 
				//minx~~ , lonlat! LL,UR,UR,LR 
				//LL
				int er = pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				minx = maxx = x;
				miny = maxy = y;
				//UL
				er = pXDProjTransform(srcCrd,destCrd,ix,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//UR
				er = pXDProjTransform(srcCrd,destCrd,ax,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//LR
				er = pXDProjTransform(srcCrd,destCrd,ax,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				resolution = (maxx-minx)/nXSize;
				//printf( "m_resolution 1 - %f , %f %f %d\n",resolution,maxx,minx,nXSize);
			}
			else { //TM
				//ori resolution
				resolution = (ax-ix)/nXSize;
				//resolution = info->pixelSize.width;
				//printf( "m_resolution 2 - %f , %d %d %d\n",resolution,ax,ix,info->pixelSize.width);
				
				/* //test
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(5186,srcCrd); //TM 
				getEPSGString(4326,destCrd);
				pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				*/
			}
		}
		break;
		case 1: {
			void *tiff = pXDXTIFFOpen((char*)filePath.c_str());
			int				dataBits;		// 데이터 크기
			int				dataType;		// 데이터 아입
			int				band;			// 밴드수  r,g,b,...
			unsigned int	imagetype;	// E_GEO_IMAGE_LOAD_TYPE
			double			ix,iy,ax,ay,minx,miny,maxx,maxy;			// 이미지 위상학적 영역
			int				width,height;			
			pXDTIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &band );
			//1 밴드의 비트수 
			pXDTIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &dataBits );
			//원본 영상 가로세로 픽셀 수 
			pXDTIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
			pXDTIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
			void* geotiff = pXDGTIFNew( tiff );
			double x,y;
			//원본 영상 경계
			x=0.0;		y=(double)height;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Lower Left"
				ix = x;	iy = y;
			}
			
			x=(double)width;	y=0.0;
			if(pXDGTIFImageToPCS( geotiff, &x, &y )){//"Upper Right"
				ax = x;	ay = y;
			}
			bool bLonLat = isLonLat(ix,iy,ax,ay);
			//printf("bound : (%f,%f)~(%f,%f)\n",ix,iy,ax,ay);
			if(bLonLat) {  //lonlat
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(4326,srcCrd); // lonlat
				getEPSGString(5186,destCrd); //TM 
				//minx~~ , lonlat! LL,UR,UR,LR 
				//LL
				int er = pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				minx = maxx = x;
				miny = maxy = y;
				//UL
				er = pXDProjTransform(srcCrd,destCrd,ix,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//UR
				er = pXDProjTransform(srcCrd,destCrd,ax,ay,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				//LR
				er = pXDProjTransform(srcCrd,destCrd,ax,iy,&x,&y);
				if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
				if(x < minx) minx = x; if(y < miny) miny = y;
				if(x > maxx) maxx = x; if(y > maxy) maxy = y;
				resolution = (maxx-minx)/width;
			}
			else { //TM
				//ori resolution
				int count;
				double *data;
				tiff_t *t_tif = ((GTIF*)geotiff)->gt_tif;
				if ((((GTIF*)geotiff)->gt_methods.get)(t_tif, GTIFF_PIXELSCALE, &count, &data )) {
					resolution = *data;
					//printf( "m_resolution 1 - %e , %f, %f, %d\n",resolution,ax,ix,width);
				}else {
					resolution = (ax-ix)/width;
					//printf( "m_resolution 2 - %f , %d %d %d",resolution,ax,ix,width);
				}
				/* //test
				char srcCrd[128]={0,};
				char destCrd[128]={0,};
				getEPSGString(5186,srcCrd); //TM 
				getEPSGString(4326,destCrd);
				pXDProjTransform(srcCrd,destCrd,ix,iy,&x,&y);
				*/
			}

			pXDXTIFFClose(tiff);
		}
		break;
		case 2:
		break;

		case -1:
		default:
		break;
	}
	return resolution;
}
rd_kafka_t *getTopicProducer(string connect)
{
	rd_kafka_t *producer;
	rd_kafka_conf_t *conf;
	char errstr[512];
	// Create client configuration
	conf = rd_kafka_conf_new();
	// User-specific properties that you must set
	//set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	
	set_config(conf, "bootstrap.servers",connect.c_str());
	
	// Fixed properties
	// 0 이라면, 프로듀서는 카프카에게 메시지를 전송하고 leader가 메시지를 잘 받았는지 확인하지 않는다
	// 1 이라면, 프로듀서는 메시지를 전송하고 leader 파티션이 메시지를 잘 받았는지 기다린다
	// all 이라면, leader 파티션이 정상적으로 수신했고 follower 파티션도 복제가 완료됨을 보장할 수 있다. 데이터 손실율은 없지만 기다리는 시간이 길어지기 때문에 가장 느리다. 
	set_config(conf, "acks",              ACKS); //0: 
	// Install a delivery-error callback.
	rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);
	// Create the Producer instance.
	producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
	if (!producer) {
		g_error("Failed to create new producer: %s", errstr);
		return NULL;
	}
	
	// Configuration object is now owned, and freed, by the rd_kafka_t instance.
	conf = NULL;
	return producer;
	
}
//모든 레벨의 가공 갯수를 조사해서 프로듀스에 최초로 보낸다.
//웹 UI 에서 사용하기 위함 - 명달이가 제공하는 프로세스 카운팅 토픽에 제공
void sendTotalCountMsg(string connect,string topic,unsigned long total_count)
{
		rd_kafka_t *producer;
	rd_kafka_conf_t *conf;
	char errstr[512];
	// Create client configuration
	conf = rd_kafka_conf_new();
	// User-specific properties that you must set
	//set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	
	set_config(conf, "bootstrap.servers",connect.c_str());
	
	// Fixed properties
	// 0 이라면, 프로듀서는 카프카에게 메시지를 전송하고 leader가 메시지를 잘 받았는지 확인하지 않는다
	// 1 이라면, 프로듀서는 메시지를 전송하고 leader 파티션이 메시지를 잘 받았는지 기다린다
	// all 이라면, leader 파티션이 정상적으로 수신했고 follower 파티션도 복제가 완료됨을 보장할 수 있다. 데이터 손실율은 없지만 기다리는 시간이 길어지기 때문에 가장 느리다. 
	set_config(conf, "acks",              ACKS); //0: 
	// Install a delivery-error callback.
	rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);
	// Create the Producer instance.
	producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
	if (!producer) {
		g_error("Failed to create new producer: %s", errstr);
		return 1;
	}
	
	// Configuration object is now owned, and freed, by the rd_kafka_t instance.
	conf = NULL;
		
    // Produce data by selecting random values from these lists.
	unsigned int i;
    unsigned int message_count = 10;

    string key,value;
    size_t key_len,value_len;
	if(total_count!=0) {
		key = stdformat("TC");
		key_len = key.size(); 
		value = stdformat("%d",total_count);
		value_len = value.size();
	}
	else {
		key = stdformat("C");
		key_len = key.size(); 
		value = stdformat("1");
		value_len = value.size();
	}
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer,
                            RD_KAFKA_V_TOPIC(topic.c_str()),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_KEY((void*)key.c_str(), key_len),
                            RD_KAFKA_V_VALUE((void*)value.c_str(), value_len),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic.c_str(), rd_kafka_err2str(err));
        return 1;
    } else {
        //g_message("Produced event to topic %s: key = %12s value = %12s", topic.c_str(), key.c_str(), value.c_str());
    }

    rd_kafka_poll(producer, 0);
		
	

    // Block until the messages are all sent.
    //g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    //g_message("%d events were produced to topic %s.", message_count, topic.c_str());

    rd_kafka_destroy(producer);	
	return 0;
}
//가공 컨슈머에서 가공 될때마다 1개씩 메세지 보낸다.
//웹 UI 에서 사용하기 위함 - 명달이가 제공하는 프로세스 카운팅 토픽에 제공 
int sendCountMsg(rd_kafka_t *producer,string topic)
{

	if (!producer) {
		g_error("Failed to create new producer: NULL");
		return 1;
	}
	
		
    // Produce data by selecting random values from these lists.
	unsigned int i;
    unsigned int message_count = 10;

    string key,value;
    size_t key_len,value_len;
	key = stdformat("C");
	key_len = key.size(); 
	value = stdformat("1");
	value_len = value.size();
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer,
                            RD_KAFKA_V_TOPIC(topic.c_str()),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_KEY((void*)key.c_str(), key_len),
                            RD_KAFKA_V_VALUE((void*)value.c_str(), value_len),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic.c_str(), rd_kafka_err2str(err));
        return 1;
    } else {
        //g_message("Produced event to topic %s: key = %12s value = %12s", topic.c_str(), key.c_str(), value.c_str());
    }

    rd_kafka_poll(producer, 0);
		
	

    // Block until the messages are all sent.
    //g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    //g_message("%d events were produced to topic %s.", message_count, topic.c_str());
	return 0;
}

//레벨 카운트로 각 레벨 가공을 제어하고 레벨 카운트에 도달해야지 다음 레벨로 가공한다.
void sendLevelCountMsg(string connect,string topic,int level,unsigned long level_count)
{
	rd_kafka_t *producer;
	rd_kafka_conf_t *conf;
	char errstr[512];
	// Create client configuration
	conf = rd_kafka_conf_new();
	// User-specific properties that you must set
	//set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	
	set_config(conf, "bootstrap.servers",connect.c_str());
	
	// Fixed properties
	// 0 이라면, 프로듀서는 카프카에게 메시지를 전송하고 leader가 메시지를 잘 받았는지 확인하지 않는다
	// 1 이라면, 프로듀서는 메시지를 전송하고 leader 파티션이 메시지를 잘 받았는지 기다린다
	// all 이라면, leader 파티션이 정상적으로 수신했고 follower 파티션도 복제가 완료됨을 보장할 수 있다. 데이터 손실율은 없지만 기다리는 시간이 길어지기 때문에 가장 느리다. 
	set_config(conf, "acks",              ACKS); //0: 
	// Install a delivery-error callback.
	rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);
	// Create the Producer instance.
	producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
	if (!producer) {
		g_error("Failed to create new producer: %s", errstr);
		return 1;
	}
	
	// Configuration object is now owned, and freed, by the rd_kafka_t instance.
	conf = NULL;
		
    // Produce data by selecting random values from these lists.
	unsigned int i;
    unsigned int message_count = 10;

    string key,value;
    size_t key_len,value_len;
	if(level_count!=0) {
		key = stdformat("LC");
		key_len = key.size(); 
		//value = stdformat("%d#%d",level,level_count);
		value = stdformat("%d",level_count);
		value_len = value.size();
	}
	else {
		key = stdformat("C");
		key_len = key.size(); 
		value = stdformat("1");
		value_len = value.size();
	}
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer,
                            RD_KAFKA_V_TOPIC(topic.c_str()),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_KEY((void*)key.c_str(), key_len),
                            RD_KAFKA_V_VALUE((void*)value.c_str(), value_len),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic.c_str(), rd_kafka_err2str(err));
        return 1;
    } else {
        //g_message("Produced event to topic %s: key = %12s value = %12s", topic.c_str(), key.c_str(), value.c_str());
    }

    rd_kafka_poll(producer, 0);
		
	

    // Block until the messages are all sent.
    //g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    //g_message("%d events were produced to topic %s.", message_count, topic.c_str());

    rd_kafka_destroy(producer);	
	return 0;
}

int sendConvertCtrlMsg(rd_kafka_t *producer,string topic,string key, string value)
{

	if (!producer) {
		g_error("Failed to create new producer: NULL");
		return 1;
	}
	
		
    // Produce data by selecting random values from these lists.
	unsigned int i;
    unsigned int message_count = 10;

    //string key,value;
    size_t key_len,value_len;
	//key = stdformat("C");
	key_len = key.size(); 
	//value = stdformat("1");
	value_len = value.size();
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(producer,
                            RD_KAFKA_V_TOPIC(topic.c_str()),
                            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                            RD_KAFKA_V_KEY((void*)key.c_str(), key_len),
                            RD_KAFKA_V_VALUE((void*)value.c_str(), value_len),
                            RD_KAFKA_V_OPAQUE(NULL),
                            RD_KAFKA_V_END);

    if (err) {
        g_error("Failed to produce to topic %s: %s", topic.c_str(), rd_kafka_err2str(err));
        return 1;
    } else {
        //g_message("Produced event to topic %s: key = %12s value = %12s", topic.c_str(), key.c_str(), value.c_str());
    }

    rd_kafka_poll(producer, 0);
		
	

    // Block until the messages are all sent.
    // g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    //g_message("%d events were produced to topic %s.", message_count, topic.c_str());
	return 0;
}
int produce_tilelists(string connect,string topicConvert,string topicConvertCtrl,string topicProcess, int level,rect2dd imageRect,bool bOriginLevel)
{
	//printf("produce_tilelists : (%f,%f)~(%f,%f)\n",imageRect.ix,imageRect.iy,imageRect.ax,imageRect.ay);
	rd_kafka_t *producer;
	rd_kafka_conf_t *conf;
	char errstr[512];
	// Create client configuration
	conf = rd_kafka_conf_new();
	// User-specific properties that you must set
	//set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	
	set_config(conf, "bootstrap.servers",connect.c_str());
	
	// Fixed properties
	// 0 이라면, 프로듀서는 카프카에게 메시지를 전송하고 leader가 메시지를 잘 받았는지 확인하지 않는다
	// 1 이라면, 프로듀서는 메시지를 전송하고 leader 파티션이 메시지를 잘 받았는지 기다린다
	// all 이라면, leader 파티션이 정상적으로 수신했고 follower 파티션도 복제가 완료됨을 보장할 수 있다. 데이터 손실율은 없지만 기다리는 시간이 길어지기 때문에 가장 느리다. 
	set_config(conf, "acks",              ACKS); //0: 
	// Install a delivery-error callback.
	rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);
	// Create the Producer instance.
	producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
	if (!producer) {
		g_error("Failed to create new producer: %s", errstr);
		return 1;
	}
	
	// Configuration object is now owned, and freed, by the rd_kafka_t instance.
	conf = NULL;
		
    // Produce data by selecting random values from these lists.
	unsigned int i;
	char stridy[9],stridx[9];
	unsigned long total_count = 0;
	unsigned long count=0;
	unsigned long level_count=0;
	//진행률 토픽에 보낸다.
	//최초 1회 전체 가공 갯수를 웹(명달) 토픽으로 보낸다.
	if(bOriginLevel==true) {
		for(i=0;i<=level;i++) {
			count= getMeshWithImagesCount( i ,imageRect);
			total_count += count;
			if(i==level) level_count=count; 
			//printf("total %d / count %d - Level %d\n",total_count,count,i);
		}
		sendTotalCountMsg(connect,topicProcess,total_count); //가공 갯수 카운팅 메세지(명달)
	}
	else {
		level_count= getMeshWithImagesCount( level ,imageRect);
	}
	//첫번째 하위레벨 가공 갯수를 topicConvertCtrl 토픽으로 보낸다. --> 레벨의 가공처리 완료 점검용(가공에서 사용)
	sendLevelCountMsg(connect,topicConvertCtrl,level,level_count); //레벨 가공 상태 완료 확인용
	
	std::vector<TMIdentify> meshlist; 
	getMeshWithImages( level ,imageRect, meshlist );
	//printf("%d - %d\n",level_count,meshlist.size());
	for( i=0;i<meshlist.size();i++ )
	{
		getStringInteger(meshlist[i].idy,stridy); 
		getStringInteger(meshlist[i].idx,stridx); 

        string key,value;
		//key = stdformat("%d_%s_%s",level,stridx,stridy);
		key = stdformat("%d_%d_%d",level,meshlist[i].idx,meshlist[i].idy);
        size_t key_len = key.size(); 
		//value = stdformat("%d_%s_%s",level,stridx,stridy);
		value = stdformat("%d_%d_%d",level,meshlist[i].idx,meshlist[i].idy);
        size_t value_len = value.size();

        rd_kafka_resp_err_t err;

        err = rd_kafka_producev(producer,
                                RD_KAFKA_V_TOPIC(topicConvert.c_str()),
                                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                                RD_KAFKA_V_KEY((void*)key.c_str(), key_len),
                                RD_KAFKA_V_VALUE((void*)value.c_str(), value_len),
                                RD_KAFKA_V_OPAQUE(NULL),
                                RD_KAFKA_V_END);

        if (err) {
            g_error("Failed to produce to topic %s: %s", topicConvert.c_str(), rd_kafka_err2str(err));
            return 1;
        } else {
            //g_message("Produced event to topic %s: key = %12s value = %12s", topicConvert.c_str(), key.c_str(), value.c_str());
        }
		printf("produce : %s - %s\n",key.c_str(),value.c_str());
        rd_kafka_poll(producer, 0); //즉시 보냄
			
	}

    // Block until the messages are all sent.
    //g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    //g_message("%d events were produced to topic %s.", message_count, topic.c_str());

    rd_kafka_destroy(producer);	
	return 0;
}
int consume_tilelists(string connect,string topicConvert,string topicConvertCtrl,string topicProcess,string groupID,string src,int epsg,string dest,rect2dd rectSRC)
{
	//printf("adfa\n");
	LoadLibrary();
	/////////////////////////////////////
	// 영상가공 토픽 컨슈머
	/////////////////////////////////////	
    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();

    // User-specific properties that you must set
    //set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	set_config(conf, "bootstrap.servers",connect.c_str());
    set_config(conf, "group.id",groupID.c_str()); //컨슈머 그룹 ID"group_p6r1"
	printf("gid : %s\n",groupID.c_str());
    set_config(conf, "auto.offset.reset", "earliest"); //earliest:먼저 들어온 메세지 부터 처리,latest : 마지막 메세지 부터 처리

    // Create the Consumer instance.
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

	//특정 토픽 제목을 정해서 해당 토픽 메세지만 받아온다.
    // Convert the list of topics to a format suitable for librdkafka.
    //const char *topic = "image.process";
	//const char *topic = "imagesp6r1";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topicConvert.c_str(), RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);

	//gdal start
	GDALStart();
	GDALDatasetH *pahSrcDS = GDALGetTransFormHandleDEM((char*)src.c_str());
	//rect2dd rectSRC = GDALGetImageRect((char*)src.c_str());
	if(pahSrcDS==NULL) printf("pahSrcDS==NULL\n");
	/*GDALDataset  *poDataset = (GDALDataset*)pahSrcDS;
	rect2dd rectSRC;
	printf("1\n");
	double        adfGeoTransform[6];
	printf("2\n");
	if(poDataset==NULL) printf("poDataset==NULL\n");
	poDataset->GetGeoTransform(adfGeoTransform);
	printf("3\n");
	dimension2di imagesize;
	double	resolution;
	printf("4\n");

	imagesize.Width = GDALGetRasterXSize(pahSrcDS);
	imagesize.Height = GDALGetRasterYSize(pahSrcDS);
	printf("w %d,h %d\n",imagesize.Width,imagesize.Height);
	rectSRC.ix = adfGeoTransform[0];
	rectSRC.iy = adfGeoTransform[3] + GDALGetRasterYSize(pahSrcDS) * adfGeoTransform[5];
	rectSRC.ax = adfGeoTransform[0] + GDALGetRasterYSize(pahSrcDS) * adfGeoTransform[1];
	rectSRC.ay = adfGeoTransform[3];*/
	printf("consume_tilelists - DEM rect %f,%f ~ %f,%f\n\n\n",rectSRC.ix,rectSRC.iy,rectSRC.ax,rectSRC.ay);
	rd_kafka_t *producerProcess = getTopicProducer(connect); //진행 상태 전달(명달)
	rd_kafka_t *producerConvertCtrl = getTopicProducer(connect); //레벨 작업 제어(가공)
	/*
	//list image driver loaded.
	GDALAllRegister();
    for( int iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
    {
        GDALDriverH hDriver= GDALGetDriver(iDr);

        if( GDALGetMetadataItem( hDriver, GDAL_DCAP_RASTER, NULL) != NULL &&
            GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL) != NULL )
        {
            printf( "  %s: %s\n",
                    GDALGetDriverShortName( hDriver  ),
                    GDALGetDriverLongName( hDriver ) );
        }
    }
	//check a driver of images
	if( GDALGetDriverByName( "PNG" ) == NULL ) {
		printf("BMP NULL!!!\n");
		return 1;
	}
	else {
		printf("BMP NOT NULL!!!\n");
		
	}
	return 0; */
    // 컨슈머 동작 지점 - 무한 대기 하면서 가공 메세지 오면 처리한다.
	// Start polling for messages.
	unsigned long count=0;
    while (g_run) {
        rd_kafka_message_t *consumer_message;
		//CONSUMER_POLL_BLOCK_INTERVAL_TIME 시간만큼 대기했다가 메세지 들고옴
		//printf("111\n");
        consumer_message = rd_kafka_consumer_poll(consumer, CONSUMER_POLL_BLOCK_INTERVAL_TIME);
        if (!consumer_message) {
			//printf("111-1\n");
            // 흐른 시간 기록
			time(&end_t);
			// 시간 차이 계산
			unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			printf("No Message : %d min %d sec\n",sec/60, sec%60);
            continue;
        }
		//printf("222\n");
        if (consumer_message->err) {
			//printf("222-1\n");
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
			//printf("333\n");
			//여러 메세지 중에 영상 가공 메세지만 처리함. 
			/*if(strcmp(rd_kafka_topic_name(consumer_message->rkt),topicConvert.c_str())!=0) {
				time(&end_t);
				// 시간 차이 계산
				unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
				printf("Another Topic  : %s -- %d min %d sec\n",rd_kafka_topic_name(consumer_message->rkt),sec/60, sec%60);
				continue;
				
			} */
            g_message("Consumed event from topic %s: key = %.*s value = %s - %d",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload,
                      ++count);
			//1. get index from message by split
			string retStr = stdformat("%s",consumer_message->payload);
			vector<string> result = split(retStr, '_');
			int level = atoi(result[0].c_str());
			int idx = atoi(result[1].c_str());
			int idy = atoi(result[2].c_str());
			if(level==1000) { //병렬
				rd_kafka_message_destroy(consumer_message);
				g_run=0;
				break;
			}
			char stridy[9],stridx[9];
			getStringInteger(idx,stridx); 
			getStringInteger(idy,stridy); 
			//2. create folder
			string path = dest;
			//printf("%s\n",path.c_str());
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%d",level);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%s",stridy);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);


			//3. get tiles data ,using xdraster lib, xdproj
			short colorkey[6];
			colorkey[0]=0;colorkey[1]=0;colorkey[2]=0;colorkey[3]=0;colorkey[4]=0;colorkey[5]=0;
			string fname = stdformat("%s/%s_%s.bil",path.c_str(),stridy,stridx);
			float noValue = 255;
			int ret = getDEMTIles(pahSrcDS,src,epsg,level,idx,idy,noValue,dest,fname,rectSRC);
			////////////////
			// 흐른 시간 기록
			////////////////
			//time(&end_t);
			// 시간 차이 계산
			//unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
			///////////////////////////////////////
			
			// Free the message when we're done.
			rd_kafka_message_destroy(consumer_message);
			//전체 진행 상황 메세지 처리 (명달 관리, 웹 UI 용)
			sendConvertCtrlMsg(producerProcess,topicProcess.c_str(),"C","1");
			//가공 레벨 진행 상황 처리,단일 레벨 가공 후 상위 레벨로 이동
			sendConvertCtrlMsg(producerConvertCtrl,topicConvertCtrl.c_str(),"C","1"); 
			
        }
    }
	GDALFreeHandleDEM(pahSrcDS, 1);
	GDALEnd();
	
    // Close the consumer: commit final offsets and leave the group.
    g_message( "Closing consumer - Lowest Level");
    rd_kafka_consumer_close(consumer);

    // Destroy the consumer.
    rd_kafka_destroy(consumer);
	UnloadLibrary();
    return 0;
}
void getDEMBottom2Top(string folder,int level, int idx, int idy,int transR,int transG, int transB,string pngfname) {
	int SLEEP_TIME=1;
	int EXIT_COUNT = 100;
	int blevel = level + 1;
	char *archivedNamePNG=(char*)pngfname.c_str();
	int imageSize=256;
	string blevelFolderPNG,byFolderPNG,barchivedNamePNG;
	char bstridy[9], bstridx[9];
	int i,j,imageWidth,imageHeight;
	imageWidth=imageHeight=512;
	//모두 투명으로 초기화 
	SColor color = SColor(0,transR,transG,transB);
	CImage *image = new CImage(ECF_A8R8G8B8, dimension2d<s32>(imageWidth, imageHeight));
	for (j = 0; j < 512; j++) {
		for (i = 0; i < 512; i++) {
			image->setPixel(i, j, color);
		}
	}
	//bottom level에서 png가 있으면 512*512 위치에 맞게 가져온다.
	bool bExits = false;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			string str;
			int bidx = idx * 2 + i;
			int bidy = idy * 2 + j;
			int xoff = i * imageSize;
			int yoff = (2 - j - 1)*imageSize;
			//int yoff= j*imageSize;
			position2di pos;
			pos.x = xoff;
			pos.y = yoff;
			rect2di rect = rect2di(0, 0, 256, 256);
			rect2di cliprect = rect2di(xoff, yoff, xoff + 256, yoff + 256);


			getStringInteger(bidy, bstridy);
			getStringInteger(bidx, bstridx);
			blevelFolderPNG=stdformat("%s/%d", folder.c_str(), blevel);
			byFolderPNG=stdformat("%s/%s", blevelFolderPNG.c_str(), bstridy);
			if (!filesystem::exists(byFolderPNG)) filesystem::create_directory(byFolderPNG);
			
			barchivedNamePNG=stdformat("%s/%s_%s.png", byFolderPNG.c_str(),bstridy,bstridx);
			// bottom 타일이 있으면 png -> bmp
			if (filesystem::exists(barchivedNamePNG)) {

				CImagePNG bimg;
				FILE *f = fopen(barchivedNamePNG.c_str(), "rb");
				int wcount = 0;
				bool bContinue = false;
				while (f == NULL) {
					sleep(SLEEP_TIME);
					f = fopen(barchivedNamePNG.c_str(), "rb");
					wcount++;
					if (wcount > EXIT_COUNT) {
						printf(" getDEMBottom2Top - barchivedNamePNG  - %s\n", barchivedNamePNG.c_str());
						bContinue = true;
						break;
					}
				}
				if (bContinue) {
					if (f) fclose(f);
					continue;
				}
				CImage *bImage = bimg.loadImage(f);
				fclose(f);
				bImage->copyTo(image, pos, rect, &cliprect);
				bExits = true;
				delete bImage;
			}
			else {

				//if(idy==222 && idx==543)  {
				//	str.Format("child %d_%d_%d",clevel,cidy,cidx);
				//	AfxMessageBox(str);
				//}

			}

		} //for j end
	}//for i end
	
	if (bExits) { //4개 bottom 중에 하나라도 있으면 무조건 현재 레벨은 만든다. 없으면 안 만든다.
		SColor src_color, dest_color;

		if (filesystem::exists(archivedNamePNG)) { //현재 만들고자 하는 png 타일이 있으면 자식으로 만들어진 512 * 512 크리의 타일과 비교 처리해야한다.
			//자식으로 부터 만들어진 512 * 512 타일에 현재 존재하고 있는 256 * 256 크기의 타일을 
			//512*512 로 리사이즈 해서 자식으로 부터 만들어진 타일의 투명 영역에 기존 타일 영상의 정보를 갱신해야한다.
			//왜 256 -> 512로 하냐면 512-> 256으로 하면 메모리도 줄어들고 속도도 빠르지만 결정적으로 투명처리가 좀 지저분 해지는 경향이 있다.
			CImage *exImage = NULL;
			CImagePNG existpng;
			int imageWidth, p, q;
			imageWidth = 512;
			FILE *existfp = fopen(archivedNamePNG, "rb");
			int wcount = 0;
			bool bContinue = false;
			while (existfp == NULL) {
				sleep(SLEEP_TIME);
				existfp = fopen(archivedNamePNG, "rb");
				wcount++;
				if (wcount > EXIT_COUNT) {
					printf(" getDEMBottom2Top - archivedNamePNG  - %s\n", archivedNamePNG);
					bContinue = true;
					break;
				}
			}
			if (bContinue == true) {
				if (existfp) fclose(existfp);
			}
			else {
				exImage = existpng.loadImage(existfp); //existpng 는 256 * 256
				//int pWidth = existpng.Width;//-> 256
				//int pHeight = existpng.Height;//-> 256
				fclose(existfp);

				SColor scol, tscol;
				short r, g, b, a, tr, tg, tb, ta;
				for (q = 0; q < imageWidth; q++) { //imageWidth = 512
					for (p = 0; p < imageWidth; p++) {
						scol = image->getPixel(p, q); //child에서 4개 붙인 이미지 image 512 * 512
						r = scol.getRed();
						g = scol.getGreen();
						b = scol.getBlue();
						a = scol.getAlpha();
						if ((transR == r && transG == g && transB == b) || a == 0) { //child에서 가져온 생성 이미지에 투명값색상 혹은 알파가 있으면 
							tscol = exImage->getPixel(p / 2.0f, q / 2.0f);//imageWidth-q-1); --> 기존 256이미지를 512로 확대한거
							/*tr = tscol.getRed();
							tg = tscol.getGreen();
							tb = tscol.getBlue();
							ta = tscol.getAlpha();
							SColor tscol2= SColor(ta,tr,tg,tb);*/
							image->setPixel(p, q, tscol);
						}
					}
				}
				//if(pImage) { delete pImage;	pImage=0; }
				if (exImage) { delete exImage;	exImage = 0; }
			}
		}
		
		CImagePNG iimg;
		CImage * tarImage = new CImage(ECF_A8R8G8B8, dimension2d<s32>(256, 256));

		u32 param = 0;
		FILE *fp = NULL;
		fp = fopen(archivedNamePNG, "wb");
		int wcount = 0;
		bool bContinue = false;
		while (fp == NULL) {
			sleep(SLEEP_TIME);
			fp = fopen(archivedNamePNG, "wb");
			wcount++;
			if (wcount > EXIT_COUNT) {
				printf(" CreateParentTileFromChild - archivedNamePNG 2  - %s\n", archivedNamePNG);
				bContinue = true;
				break;
			}
		}
		if (bContinue == true) {
			if (fp) fclose(fp);
		}
		else {
			image->copyToScaling(tarImage); //512,512 --> 256,256 

			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			iimg.writeImage(fp, tarImage, param);
			fclose(fp);
		}
		if (tarImage) {
			delete tarImage;
			tarImage = 0;
		}
	}
	if (image) {
		delete image;
		image = 0;
	}
}
int consume_tilelists_Bottom2Top(string connect,string topicConvert,string topicConvertCtrl,string topicProcess,string groupID,string src,int epsg,string dest,int fromLevel,int toLevel)
{
	LoadLibrary();
	/////////////////////////////////////
	// 영상가공 토픽 컨슈머
	/////////////////////////////////////	
    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();
	printf("gid : %s\n",groupID.c_str());
    // User-specific properties that you must set
    //set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	set_config(conf, "bootstrap.servers",connect.c_str());
    set_config(conf, "group.id",groupID.c_str()); //컨슈머 그룹 ID "group_p6r1"
    set_config(conf, "auto.offset.reset", "earliest"); //earliest:먼저 들어온 메세지 부터 처리,latest : 마지막 메세지 부터 처리

    // Create the Consumer instance.
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

    // Convert the list of topics to a format suitable for librdkafka.
    //const char *topic = "image.process";
	//const char *topic = "imagesp6r1";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topicConvert.c_str(), RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);

	//gdal start
	rd_kafka_t *producerProcess = getTopicProducer(connect); //진행 상태 전달(명달)
	rd_kafka_t *producerConvertCtrl = getTopicProducer(connect); //레벨 작업 제어(가공)
    // 컨슈머 동작 지점 - 무한 대기 하면서 가공 메세지 오면 처리한다.
	// Start polling for messages.
    while (g_run) {
        rd_kafka_message_t *consumer_message;
		//CONSUMER_POLL_BLOCK_INTERVAL_TIME 시간만큼 대기했다가 메세지 들고옴
        consumer_message = rd_kafka_consumer_poll(consumer, CONSUMER_POLL_BLOCK_INTERVAL_TIME);
        if (!consumer_message) {
            // 흐른 시간 기록
			time(&end_t);
			// 시간 차이 계산
			unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
            continue;
        }
		
        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
			//여러 메세지 중에 영상 가공 메세지만 처리함. 
			/*if(strcmp(rd_kafka_topic_name(consumer_message->rkt),topicConvert.c_str())!=0) {
				time(&end_t);
				// 시간 차이 계산
				unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
				printf("Another Topic  : %s -- %d min %d sec\n",rd_kafka_topic_name(consumer_message->rkt),sec/60, sec%60);
				continue;
				
			}*/
            g_message("Consumed event from topic %s: key = %.*s value = %s",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload
                      );
			//1. get index from message by split
			string retStr = stdformat("%s",consumer_message->payload);
			vector<string> result = split(retStr, '_');
			int level = atoi(result[0].c_str());
			int idx = atoi(result[1].c_str());
			int idy = atoi(result[2].c_str());
			if(level != toLevel) {
				g_message("Consumer Level Diff: %d_%d_%d is not %d level",level,idx,idy,toLevel );
				rd_kafka_message_destroy(consumer_message);
				continue;
			}
			if(level==1000) { //병렬
				rd_kafka_message_destroy(consumer_message);
				g_run=false;
				break;
			}
			char stridy[9],stridx[9];
			getStringInteger(idx,stridx); 
			getStringInteger(idy,stridy); 
			//2. create folder
			string path = dest;
			//printf("%s\n",path.c_str());
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%d",level);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%s",stridy);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);


			//3. get tiles data ,using xdraster lib, xdproj
			short colorkey[6];
			colorkey[0]=0;colorkey[1]=0;colorkey[2]=0;colorkey[3]=0;colorkey[4]=0;colorkey[5]=0;
			string fname = stdformat("%s/%s_%s.png",path.c_str(),stridy,stridx);
			getDEMBottom2Top(dest,level,idx,idy,colorkey[0],colorkey[1],colorkey[2],fname);
			////////////////
			// 흐른 시간 기록
			////////////////
			//time(&end_t);
			// 시간 차이 계산
			//unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
			///////////////////////////////////////
			
			// Free the message when we're done.
			rd_kafka_message_destroy(consumer_message);
			//sendCountMsg(producerProcess,topicProcess); // 전체 진행 상황 메세지 처리 (명달 관리, 웹 UI 용)
			
			// 전체 진행 상황 메세지 처리 (명달 관리, 웹 UI 용)
			sendConvertCtrlMsg(producerProcess,topicProcess.c_str(),"C","1"); 
			//가공 레벨 진행 상황 처리,단일 레벨 가공 후 상위 레벨로 이동
			sendConvertCtrlMsg(producerConvertCtrl,topicConvertCtrl.c_str(),"C","1"); 
			
        }
    }
	
    // Close the consumer: commit final offsets and leave the group.
    g_message( "Closing consumer - Convert Level %d",toLevel);
    rd_kafka_consumer_close(consumer);

    // Destroy the consumer.
    rd_kafka_destroy(consumer);
	UnloadLibrary();
    return 0;
}
typedef struct consParam {
	string connect;
	string topic;
	string topicProcess;
	string topicProcessCtrl;
	string src;
	int epsg;
	string dest;
} CONSPARAM;
void *consume_tilelists_thread(void *arg)
{	
	CONSPARAM *param = (CONSPARAM*)arg;
	string connect=param->connect;
	string topic=param->topic;
	string topicProcess=param->topicProcess;
	string topicProcessCtrl=param->topicProcessCtrl;
	string src=param->src;
	int epsg=param->epsg;
	string dest=param->dest;	
	printf("consume_tilelists_thread\n");
	printf("%s - %s - %s - %s\n",connect.c_str(),topic.c_str(),src.c_str(),dest.c_str());
	/////////////////////////////////////
	// 영상가공 토픽 컨슈머
	/////////////////////////////////////	
    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();

    // User-specific properties that you must set
    //set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	set_config(conf, "bootstrap.servers",connect.c_str());
    set_config(conf, "group.id",          "group_p6r1"); //컨슈머 그룹 ID
    set_config(conf, "auto.offset.reset", "earliest"); //earliest:먼저 들어온 메세지 부터 처리,latest : 마지막 메세지 부터 처리

    // Create the Consumer instance.
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

    // Convert the list of topics to a format suitable for librdkafka.
    //const char *topic = "image.process";
	//const char *topic = "imagesp6r1";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topic.c_str(), RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);

	//gdal start
	GDALStart();
	GDALDatasetH *pahSrcDS = GDALGetTransFormHandleDEM((char*)src.c_str());
	rect2dd rectSRC = GDALGetImageRect((char*)src.c_str());
	//rd_kafka_t *producerProcess = getTopicProducer(connect); //진행 상태 전달(명달)
	//rd_kafka_t *producerProcessCtrl = getTopicProducer(connect); //레벨 작업 제어(가공)
	/*
	//list image driver loaded.
	GDALAllRegister();
    for( int iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
    {
        GDALDriverH hDriver= GDALGetDriver(iDr);

        if( GDALGetMetadataItem( hDriver, GDAL_DCAP_RASTER, NULL) != NULL &&
            GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL) != NULL )
        {
            printf( "  %s: %s\n",
                    GDALGetDriverShortName( hDriver  ),
                    GDALGetDriverLongName( hDriver ) );
        }
    }
	//check a driver of images
	if( GDALGetDriverByName( "PNG" ) == NULL ) {
		printf("BMP NULL!!!\n");
		return 1;
	}
	else {
		printf("BMP NOT NULL!!!\n");
		
	}
	return 0; */
    // 컨슈머 동작 지점 - 무한 대기 하면서 가공 메세지 오면 처리한다.
	// Start polling for messages.
	unsigned long count=0;
    while (g_run) {
        rd_kafka_message_t *consumer_message;
		//CONSUMER_POLL_BLOCK_INTERVAL_TIME 시간만큼 대기했다가 메세지 들고옴
        consumer_message = rd_kafka_consumer_poll(consumer, CONSUMER_POLL_BLOCK_INTERVAL_TIME);
        if (!consumer_message) {
            g_message("Waiting...");
			// 흐른 시간 기록
			time(&end_t);
			// 시간 차이 계산
			unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
            continue;
        }

        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
            g_message("Consumed event from topic %s: key = %.*s value = %s",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload
                      );
			//1. get index from message by split
			string retStr = stdformat("%s",consumer_message->payload);
			vector<string> result = split(retStr, '_');
			int level = atoi(result[0].c_str());
			int idx = atoi(result[1].c_str());
			int idy = atoi(result[2].c_str());
			char stridy[9],stridx[9];
			getStringInteger(idx,stridx); 
			getStringInteger(idy,stridy); 
			//2. create folder
			string path = dest;
			//printf("%s\n",path.c_str());
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%d",level);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);

			path = path + stdformat("/%s",stridy);
			if (!filesystem::exists(path)) filesystem::create_directory(path);
			//filesystem::create_directory(path);


			//3. get tiles data ,using xdraster lib, xdproj
			float noValue = 255;
			string fname = stdformat("%s/%s_%s.bil",path.c_str(),stridy,stridx);
			getDEMTIles(pahSrcDS,src,epsg,level,idx,idy,noValue,dest,fname,rectSRC);
			
			//// 흐른 시간 기록
			//time(&end_t);
			// 시간 차이 계산
			//unsigned long sec = (unsigned long)(double)difftime(//end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
			
			// Free the message when we're done.
			rd_kafka_message_destroy(consumer_message);
			//sendCountMsg(producerProcess,topicProcess); // 전체 진행 상황 메세지 처리 (명달 관리, 웹 UI 용)
			//sendConvertCtrlMsg(producerProcessCtrl,topicProcessCtrl,"C","1"); //가공 레벨 진행 상황 처리,단일 레벨 가공 후 상위 레벨로 이동
			count++;
        }
    }
	GDALFreeHandleIMG(pahSrcDS, 1);
	GDALEnd();
	
    // Close the consumer: commit final offsets and leave the group.
    //g_message( "Closing consumer");
    rd_kafka_consumer_close(consumer);

    // Destroy the consumer.
    rd_kafka_destroy(consumer);

    return 0;
}
const int SLEEP_TIME = 1;
const int EXIT_COUNT = 10;
void CreateChildDEMFromParentf32(string dataPath, int level,int idx,int idy)
{
	if (level == 0) return;
	string filename,pfilename, archivedName, archivedNamebk;
	char stridy[9], stridx[9];
	char pstridy[9], pstridx[9];

	getStringInteger(idx, stridx);
	getStringInteger(idy, stridy);
	filename=stdformat("%s/%d/%s/%s_%s.bil", dataPath.c_str(), level, stridy, stridy,stridx);
	if (filesystem::exists(filename))  {
		return; // 기존 이미지가 있으면..
	}
	int plevel = level - 1;
	int pix = (int)(idx / 2);
	int piy = (int)(idy / 2);

	getStringInteger(pix, pstridx);
	getStringInteger(piy, pstridy);
	//string dataPath = GetCVTPath(filename);
	pfilename=stdformat("%s/%d/%s/%s_%s.bil", dataPath.c_str(), plevel, pstridy, pstridy,pstridx);
	printf("check parent : %s\n",pfilename.c_str());
	if (!filesystem::exists(pfilename)) {// 부모 bil 이미지가 없으면..
		CreateChildDEMFromParentf32(dataPath, plevel, pix, piy);
	}

	int i, j, l, m;

	int zSize = 64 + 1;

	int dsize = zSize * zSize;//해당 메쉬 높이값을 불러온다

	// 부모 메쉬 읽어 온다.
	float* phmap = new float[dsize];
	printf("exist parent : %s\n",pfilename.c_str());
	FILE *fp = fopen(pfilename.c_str(), "rb");
	int wcount = 0;
	bool bContinue = false;
	while (!fp) {
		fp = fopen(pfilename.c_str(), "rb");
		sleep(SLEEP_TIME);
		wcount++;
		if (wcount > EXIT_COUNT) {
			printf("CreateChildDEMFromParentf32  A - %s - %d\n", pfilename.c_str(), wcount);
			bContinue = true;
			break;
		}
	}
	if (bContinue) {
		if (fp) fclose(fp);
		printf("can't CreateChildDEMFromParentf32  B - %s - %d\n", pfilename.c_str(),wcount);
		return;
	}
	fread(phmap, sizeof(float), dsize, fp);
	fclose(fp);

	//자식메쉬로 만들 데이터를 부모메쉬 데이터에서 보간해서 생성한다.
	int permesh = 64;
	int doublesize = permesh * 2 + 1;
	float* hmap = new float[doublesize*doublesize];

	for (i = 0; i < permesh; i++)
	{
		for (j = 0; j < permesh; j++)
		{
			hmap[(i * 2)   *doublesize + (j * 2)] = phmap[i*zSize + j];
			hmap[(i * 2)   *doublesize + (j * 2) + 1] = (phmap[i*zSize + j] + phmap[i*zSize + j + 1]) / 2;
			hmap[(i * 2)   *doublesize + (j * 2) + 2] = phmap[i*zSize + j + 1];
			hmap[((i * 2) + 1)*doublesize + (j * 2)] = (phmap[i*zSize + j] + phmap[(i + 1)*zSize + j]) / 2;
			hmap[((i * 2) + 1)*doublesize + (j * 2) + 1] = (phmap[i*zSize + j] + phmap[(i + 1)*zSize + j] + phmap[i*zSize + j + 1] + phmap[(i + 1)*zSize + j + 1]) / 4;
			hmap[((i * 2) + 1)*doublesize + (j * 2) + 2] = (phmap[i*zSize + j + 1] + phmap[(i + 1)*zSize + j + 1]) / 2;
			hmap[((i * 2) + 2)*doublesize + (j * 2)] = phmap[(i + 1)*zSize + j];
			hmap[((i * 2) + 2)*doublesize + (j * 2) + 1] = (phmap[(i + 1)*zSize + j] + phmap[(i + 1)*zSize + j + 1]) / 2;
			hmap[((i * 2) + 2)*doublesize + (j * 2) + 2] = phmap[(i + 1)*zSize + j + 1];
		}
	}

	SAFE_DELETE_ARRAY(phmap);

	string levelFolder, yFolder, levelFolderbk, yFolderbk;
	levelFolder=stdformat("%s/%d", dataPath.c_str(), level);
	if (!filesystem::exists(levelFolder)) filesystem::create_directory(levelFolder);

	//자식 메쉬 데이터 저장
	float* chmap = new float[dsize];
	//2 * 2 형태로 저장
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			int idxx = pix * 2 + j;
			int idyy = piy * 2 + i;

			getStringInteger(idyy, stridy);
			getStringInteger(idxx, stridx);

			yFolder=stdformat("%s/%s", levelFolder.c_str(), stridy);
			if (!filesystem::exists(yFolder)) filesystem::create_directory(yFolder);
			archivedName=stdformat("%s/%s_%s.bil", yFolder.c_str(), stridy, stridx);

			if (filesystem::exists(archivedName)) {
				continue; // 기존 이미지가 있으면..
			}

			for (l = 0; l < zSize; l++)
			{
				int yi = (1 - i)*permesh + l;

				for (m = 0; m < zSize; m++)
				{
					chmap[l*zSize + m] = hmap[yi*doublesize + j * permesh + m];
				}
			}

			// 오버레이 하는 것을 체크 한다.

			FILE *fp = fopen(archivedName.c_str(), "wb");
			int wcount = 0;
			bool bContinue = false;
			while (!fp) {
				fp = fopen(archivedName.c_str(), "wb");
				sleep(SLEEP_TIME);
				wcount++;
				if (wcount > EXIT_COUNT) {
					printf("CreateChildDEMFromParentf32  A 2 - %s - %d\n", archivedName.c_str(), wcount);
					bContinue = true;
					break;
				}
			}
			if (bContinue) {
				if (fp) fclose(fp);
				printf("CreateChildDEMFromParentf32 B 2  - %s - %d\n", archivedName.c_str(),wcount);
				continue;
			}
			fwrite(chmap, sizeof(float), dsize, fp);
			fclose(fp);
		}//for( j 
	}//for( i

	SAFE_DELETE_ARRAY(hmap);
	SAFE_DELETE_ARRAY(chmap);

}

void HeightsOverlayNoValue(float* ddata, u32 dsize,string filename, int level, int idx, int idy, rect2dd rect, double delta, float noValue)

{
	//printf("HeightsOverlayNoValue - 1\n");
	string _filename;
	_filename=stdformat("%s", filename.c_str());
	if (!filesystem::exists(_filename)) return;// 기존 이미지가 없으면..

	int zSize = 65 + 1;

	int dwTemp;//해당 메쉬 높이값을 불러온다

	float* sdata = new float[dsize];
	//printf("HeightsOverlayNoValue - 2\n");
	//HANDLE fh = CreateFile(_filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//만들어진 xds 데이터
	//ReadFile(fh, sdata, dsize * sizeof(float), &dwTemp, NULL);
	//CloseHandle(fh);
	FILE *fp = fopen(_filename.c_str(), "rb");
	int wcount = 0;
	bool bContinue = false;
	while (!fp) {
		fp = fopen(_filename.c_str(), "rb");
		sleep(SLEEP_TIME);
		wcount++;
			printf("HeightsOverlayNoValue  - %s - %d\n", _filename.c_str(),wcount);
		if (wcount > EXIT_COUNT) {
			printf("HeightsOverlayNoValue  - %s\n", _filename.c_str());
			bContinue = true;
			break;
		}
	}
	if (bContinue) {
		if (fp) fclose(fp);
		printf("HeightsOverlayNoValue  - %s\n", _filename.c_str());
		return;
	}
	fread(sdata, sizeof(float), dsize, fp);
	fclose(fp);
	//printf("HeightsOverlayNoValue - 3\n");
	int k, j;

	for (k = 0; k < zSize; k++)
	{
		for (j = 0; j < zSize; j++)
		{
			f64 xx = rect.ix + j * delta;//이미지 픽셀의 실제 좌표
			f64 yy = rect.ay - k * delta;//
			//if( ddata[k*zSize+j]==0 ){
			if (ddata[k*zSize + j] == noValue) {//jsyun NODATA
				ddata[k*zSize + j] = sdata[k*zSize + j];
				// 원래 있던 지형도 있으니
			}
			else {
			}

		}
	}
	//printf("HeightsOverlayNoValue - 4\n");
	SAFE_DELETE_ARRAY(sdata);
	//printf("HeightsOverlayNoValue - 4-1\n");

}
int getDEMTIles(GDALDatasetH *pahSrcDS,string src, int epsg,int level, int idx,int idy,float noValue,string dest,string filename,rect2dd rectSRC)
{
	int imageType=0; //0:dds, 1:jpg, 2:png
	double meshsize = double(36.0) / pow(2.0, (double)level);
	double meshsx = (double)((double)-180.0 + meshsize * double(idx));
	double meshsz = (double)((double)-90.0 + meshsize * double(idy));
	double maxx = (double)(meshsx + meshsize);
	double maxy = (double)(meshsz + meshsize);
	int images_in = 0;
	// 구하고자 하는 이미지 영역..
	rect2dd rect(meshsx, meshsz, meshsx + meshsize, meshsz + meshsize);
	if (rectSRC.isRectCollided(rect)==true) {
		images_in = 1; //오버레이 되는 경우 추가 작업함.
		if (rectSRC.isRectContained(rect)==true) images_in = 2; //포함되는 경우
	}
	else return -1;// DEM 원본을 벗어남
	float* heights = NULL;
	int permesh = 64;
	int zSize = permesh + 1;// 메쉬 셀 갯수  32+1 or 64+1
	int dsize = zSize * zSize;//dem 조각이 담길 메모리 65*65*2 = 8,450 바이트
	double delta = meshsize / (zSize - 1);
	
	char srcCrd[128]={0,};
	char destCrd[128]={0,};
	getEPSGString(epsg,srcCrd);
	getEPSGString(4326,destCrd);
	float *tmpHeights = NULL;
	int resWidth;
	GDALWarpAppOptions *psOptions = GDALSetTransFormOptionDEM((char*)src.c_str(), (char*)srcCrd, (char*)destCrd, meshsx, meshsz, meshsx + meshsize, meshsz + meshsize, "MEM", resWidth, noValue);
	tmpHeights = GDALTransFormImageByHandleDEM(pahSrcDS, 1, psOptions, (char*)srcCrd, (char*)destCrd, meshsx, meshsz, meshsx + meshsize, meshsz + meshsize, "MEM", resWidth, noValue);
	GDALWarpAppOptionsFree(psOptions);
	//return 0;
	if (tmpHeights == NULL) {
		return -1;
	}
	heights = new float[zSize*zSize];
	if (resWidth != zSize) {
		Resampleing(tmpHeights, resWidth, resWidth, heights, zSize, zSize, 0);
	}
	else {
		memcpy(heights, tmpHeights, zSize * zSize * sizeof(float));
	}

	SAFE_DELETE_ARRAY(tmpHeights);
	if (heights && images_in > 0)
	{
		//printf("images_in : %d\n",images_in);
		//images_in=0 : 전혀 없음
		//images_in=1 : 일부만 있음, images_in=2 : 다 있음
		//구하는 지형 타일 영역에 일부만 데이터가 있는 경우는 상위 레벨에서 데이터를 가져와서 오버레이함.
		if (images_in == 1) //overlay
		{
			CreateChildDEMFromParentf32(dest.c_str(), level, idx,idy);
			HeightsOverlayNoValue(heights, dsize, filename.c_str(), level,idx,idy,rect,delta, noValue);
		}
		FILE *fp = fopen(filename.c_str(), "wb");
		fwrite(heights, sizeof(float), dsize, fp);
		fclose(fp);
		SAFE_DELETE_ARRAY(heights);
	}
	return 0;
}
void Usage()
{
	//./XDConvertImage 0 ./yeudo_25cm_4326.tif 4326 ./tmp 255 255 255
	printf("usage) ./XDConvertImage 0 src 4326 dest r g b\n");
	printf("ex) ./XDConvertImage 0 /home/hadoop/project/jsyun/kafka/XDConvertImage/yeudo_25cm_4326.tif 4326 /home/hadoop/project/jsyun/kafka/XDConvertImage/out 255 128 255\n");
}
bool check_topic_exists(const char *brokers, const char *topic_name) {
    rd_kafka_t *rk;          // Producer instance
    rd_kafka_conf_t *conf;   // Temporary configuration object
    rd_kafka_topic_t *rkt;   // Topic object
    rd_kafka_metadata_t *metadata;
    rd_kafka_resp_err_t err;

    // Kafka 프로듀서 설정
    conf = rd_kafka_conf_new();
    rd_kafka_conf_set(conf, "bootstrap.servers", brokers, NULL, 0);

    // Kafka 프로듀서 생성
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, NULL, NULL);
    if (!rk) {
        fprintf(stderr, "Failed to create Kafka producer\n");
        return false;
    }

    // 메타데이터 요청
    err = rd_kafka_metadata(rk, 1, NULL, &metadata, 5000);
    if (err) {
        fprintf(stderr, "Failed to get metadata: %s\n", rd_kafka_err2str(err));
        rd_kafka_destroy(rk);
        return false;
    }

    // 토픽 존재 여부 확인
    int topic_exists = 0;
    for (size_t i = 0; i < metadata->topic_cnt; i++) {
		printf("[%d/%d] - %s\n",i,metadata->topic_cnt,metadata->topics[i].topic);
        if (strcmp(metadata->topics[i].topic, topic_name) == 0) {
			printf("=======\n");
			printf("name : %s\n",metadata->topics[i].topic);
			printf("partition cnt : %d\n",metadata->topics[i].partition_cnt);
			for(int j=0;j<metadata->topics[i].partition_cnt;j++) {
				printf("partitin [%d/%d] : replica cnt : %d\n",j+1,metadata->topics[i].partition_cnt, metadata->topics[i].partitions[j].replica_cnt);
			}
            topic_exists = 1;
			printf("=======\n");
			break;
        }
    }

    // 결과 출력
    if (topic_exists) {
        printf("토픽 '%s'이(가) 존재합니다.\n", topic_name);
    } else {
        printf("토픽 '%s'이(가) 존재하지 않습니다.\n", topic_name);
    }

    // 메모리 해제
    rd_kafka_metadata_destroy(metadata);
    rd_kafka_destroy(rk);
	if(topic_exists) return true;
	else return false;
}
void runCommand(const char *msg) {
	int result = system(msg);
	// 결과 확인
    if (result == -1) {
        printf("명령어 실행 실패");
        return ;
    }
	else printf("명령어 '%s'이(가) 성공적으로 실행되었습니다.\n", msg);
	
}
void creTopic(const char *msg) {
	int result = system(msg);
	// 결과 확인
    if (result == -1) {
        printf("명령어 실행 실패");
        return ;
    }
	else printf("명령어 '%s'이(가) 성공적으로 실행되었습니다.\n", msg);
	
}
void delTopic(const char *msg) {
	int result = system(msg);
	// 결과 확인
    if (result == -1) {
        printf("명령어 실행 실패");
        return ;
    }
	else printf("명령어 '%s'이(가) 성공적으로 실행되었습니다.\n", msg);
	
}
/**
 * Kafka logger callback (optional)
 */
static void
logger(const rd_kafka_t *rk, int level, const char *fac, const char *buf) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        fprintf(stderr, "%u.%03u RDKAFKA-%i-%s: %s: %s\n", (int)tv.tv_sec,
                (int)(tv.tv_usec / 1000), level, fac,
                rk ? rd_kafka_name(rk) : NULL, buf);
}
void err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque) {
    fprintf(stderr, "Error: %s: %s\n", rd_kafka_err2str(err), reason);
}
void creatTopicsByCommand(string connect,string topicConvert,int partition,int replica,string topicProcess)
{
	string strCre = stdformat("ssh hadoop@master \"/home/hadoop/kafka_2.13-3.9.0/bin/kafka-topics.sh --bootstrap-server %s --create --topic %s --partitions %d --replication-factor %d\"",connect.c_str(),topicConvert.c_str(),partition,replica);
	creTopic(strCre.c_str());
	while(1) {
		bool bExist = check_topic_exists(connect.c_str(),topicConvert.c_str());
		if(bExist) break;
		sleep(1);
	}	
}
void deleteTopicsByCommand(string connect,string topicConvert,string topicProcess)
{
	bool bExist = check_topic_exists(connect.c_str(),topicConvert.c_str());
	string strDel = stdformat("ssh hadoop@master \"/home/hadoop/kafka_2.13-3.9.0/bin/kafka-topics.sh --bootstrap-server %s --delete --topic %s\"",connect.c_str(),topicConvert.c_str());
	if(bExist==true) {
		delTopic(strDel.c_str());
		while(1) {
			bool bExist = check_topic_exists(connect.c_str(),topicConvert.c_str());
			if(bExist==false) break;
			sleep(1);
		}
	}	
	/*
	bExist = check_topic_exists(connect.c_str(),topicProcess.c_str());
	//delete_topic(connect.c_str(),topic.c_str());
	strDel = stdformat("ssh hadoop@master \"/home/hadoop/kafka_2.13-3.9.0/bin/kafka-topics.sh --bootstrap-server %s --delete --topic %s\"",connect.c_str(),topicProcess.c_str());
	if(bExist==true) {
		delTopic(strDel.c_str());
		while(1) {
			bool bExist = check_topic_exists(connect.c_str(),topicProcess.c_str());
			if(bExist==false) break;
			sleep(1);
		}
	}
*/	
}
void runTopicProcessByCommand(string connect,string topicProcess)
{
	string strCre = stdformat("ssh hadoop@master \"/home/hadoop/project/jsyun/kafka/XDConvertMsg/cons/ConvertMsgCons %s %s\"",connect.c_str(),topicProcess.c_str());
	creTopic(strCre.c_str());
}
void delete_topic(string connect, const char *topicname) {

	rd_kafka_t *rk;  
    rd_kafka_conf_t *conf;
    // Kafka 설정 생성
    conf = rd_kafka_conf_new();
    
    // 브로커 설정
    if (rd_kafka_conf_set(conf, "bootstrap.servers", connect.c_str(), NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Error setting brokers: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        return 1;
    }
    // Kafka Admin 클라이언트 생성
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf,NULL, 0);
    if (!rk) {
        fprintf(stderr, "Failed to create Kafka admin client: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        return ;
    }


    rd_kafka_DeleteTopic_t *delt[1];
    const size_t delt_cnt = 1;
    rd_kafka_AdminOptions_t *options;
    rd_kafka_queue_t *rkqu;
    rd_kafka_event_t *rkev;
    const rd_kafka_DeleteTopics_result_t *res;
    const rd_kafka_topic_result_t **terr;
    int timeout_ms = 1000;
    size_t res_cnt;
    rd_kafka_resp_err_t err;
    char errstr[512];
    rkqu = rd_kafka_queue_new(rk);

    delt[0] = rd_kafka_DeleteTopic_new(topicname);
    options = rd_kafka_AdminOptions_new(rk, RD_KAFKA_ADMIN_OP_DELETETOPICS);
    err     = rd_kafka_AdminOptions_set_operation_timeout(
        options, timeout_ms, errstr, sizeof(errstr));
    //TEST_ASSERT(!err, "%s", errstr);

    printf("Deleting topic \"%s\" ""(timeout=%d)\n",topicname, timeout_ms);

    //TIMING_START(&t_create, "DeleteTopics");
    rd_kafka_DeleteTopics(rk, delt, delt_cnt, options, rkqu);
    /* Wait for result */
    rkev = rd_kafka_queue_poll(rkqu, timeout_ms + 2000);
        //TEST_ASSERT(rkev, "Timed out waiting for DeleteTopics result");

    //TIMING_STOP(&t_create);

    res = rd_kafka_event_DeleteTopics_result(rkev);
    //TEST_ASSERT(res, "Expected DeleteTopics_result, not %s",rd_kafka_event_name(rkev));
    terr = rd_kafka_DeleteTopics_result_topics(res, &res_cnt);
    //TEST_ASSERT(terr, "DeleteTopics_result_topics returned NULL");
    //TEST_ASSERT(res_cnt == delt_cnt,
    //"DeleteTopics_result_topics returned %" PRIusz
    //            " topics, "
    //            "not the expected %" PRIusz,
    //            res_cnt, delt_cnt);

    //TEST_ASSERT(!rd_kafka_topic_result_error(terr[0]),
    //            "Topic %s result error: %s",
    //            rd_kafka_topic_result_name(terr[0]),
    //            rd_kafka_topic_result_error_string(terr[0]));

    rd_kafka_event_destroy(rkev);

    rd_kafka_queue_destroy(rkqu);

    rd_kafka_AdminOptions_destroy(options);

    rd_kafka_DeleteTopic_destroy(delt[0]);
    if (rk) {
        rd_kafka_destroy(rk);
    }
}
void create_topic(string connect,
                             const char *topicname,
                             int partition_cnt,
                             int replication_factor,
                             const char **configs)
{
	rd_kafka_t *rk;
    rd_kafka_conf_t *conf;
    // Kafka 설정 생성
    conf = rd_kafka_conf_new();
    
    // 브로커 설정
    if (rd_kafka_conf_set(conf, "bootstrap.servers", connect.c_str(), NULL, 0) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Error setting brokers: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        return 1;
    }
    // Kafka Admin 클라이언트 생성
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf,NULL, 0);
    if (!rk) {
        fprintf(stderr, "Failed to create Kafka admin client: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        return ;
    }

    rd_kafka_NewTopic_t *newt[1];
    const size_t newt_cnt = 1;
    rd_kafka_AdminOptions_t *options;
    rd_kafka_queue_t *rkqu;
    rd_kafka_event_t *rkev;
    const rd_kafka_CreateTopics_result_t *res;
    const rd_kafka_topic_result_t **terr;
    int timeout_ms = 1000;//1초
    size_t res_cnt;
    rd_kafka_resp_err_t err;
    char errstr[512];
    //test_timing_t t_create;

    //if (!(rk = use_rk)) rk = test_create_producer();

    rkqu = rd_kafka_queue_new(rk);

    newt[0] = rd_kafka_NewTopic_new(topicname, partition_cnt, replication_factor,errstr, sizeof(errstr));
		
 //   TEST_ASSERT(newt[0] != NULL, "%s", errstr);
/*
    if (configs) {
            int i;

            for (i = 0; configs[i] && configs[i + 1]; i += 2)
                    TEST_CALL_ERR__(rd_kafka_NewTopic_set_config(
                        newt[0], configs[i], configs[i + 1]));
    }
*/
    options = rd_kafka_AdminOptions_new(rk, RD_KAFKA_ADMIN_OP_CREATETOPICS);
    err     = rd_kafka_AdminOptions_set_operation_timeout(options, timeout_ms, errstr, sizeof(errstr));
	/*
    printf(
        "Creating topic \"%s\" "
        "(partitions=%d, replication_factor=%d, timeout=%d)\n",
        topicname, partition_cnt, replication_factor, timeout_ms);
	*/
    //TIMING_START(&t_create, "CreateTopics");
    rd_kafka_CreateTopics(rk, newt, newt_cnt, options, rkqu);
    /* Wait for result */
    rkev = rd_kafka_queue_poll(rkqu, timeout_ms + 2000);
    //TEST_ASSERT(rkev, "Timed out waiting for CreateTopics result");

    //TIMING_STOP(&t_create);

    //TEST_ASSERT(!rd_kafka_event_error(rkev), "CreateTopics failed: %s",rd_kafka_event_error_string(rkev));
    res = rd_kafka_event_CreateTopics_result(rkev);
    //TEST_ASSERT(res, "Expected CreateTopics_result, not %s",rd_kafka_event_name(rkev));
    terr = rd_kafka_CreateTopics_result_topics(res, &res_cnt);
	/*
    TEST_ASSERT(terr, "CreateTopics_result_topics returned NULL");
    TEST_ASSERT(res_cnt == newt_cnt,
                "CreateTopics_result_topics returned %" PRIusz
                " topics, "
                "not the expected %" PRIusz,
                res_cnt, newt_cnt);

    TEST_ASSERT(!rd_kafka_topic_result_error(terr[0]) ||
                    rd_kafka_topic_result_error(terr[0]) ==
                        RD_KAFKA_RESP_ERR_TOPIC_ALREADY_EXISTS,
                "Topic %s result error: %s",
                rd_kafka_topic_result_name(terr[0]),
                rd_kafka_topic_result_error_string(terr[0]));
*/
    rd_kafka_event_destroy(rkev);
    rd_kafka_queue_destroy(rkqu);
    rd_kafka_AdminOptions_destroy(options);
    rd_kafka_NewTopic_destroy(newt[0]);
}
int sendKillMessageEndLevelConvert(vector<int>pids) {
	unsigned int i;
	for( i=0;i<pids.size();i++ ) {
		kill(pids[i], SIGKILL); //pid에 해당되는 프로세스에 crtl + c 날림,SIGINT,SIGKILL,SIGTERM
		printf("[%d/%d] kill pid : %d\n",i+1,pids.size(),pids[i]);
		//sleep(1);
	}
}
int runConvertControl(string connect,string topicConvertCtrl)
{
	// 시작 시간 기록
    time(&start_t);
	//string topic = stdformat("DT.Convert.KAKAO.Works");
	/////////////////////////////////////
	// 진행상태 토픽 컨슈머
	/////////////////////////////////////	
    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
	vector<int> pids;
    char errstr[512];
	// ./ConvertControlMsg "218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092" convertControl
	
    // Create client configuration
    conf = rd_kafka_conf_new();

    // User-specific properties that you must set
    //set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	//set_config(conf, "bootstrap.servers", "218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092");
	set_config(conf, "bootstrap.servers",(char*)connect.c_str());
    set_config(conf, "group.id",          "group_ConvertControl"); //컨슈머 그룹 ID
    set_config(conf, "auto.offset.reset", "earliest"); //earliest:먼저 들어온 메세지 부터 처리,latest : 마지막 메세지 부터 처리

    // Create the Consumer instance.
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

	//특정 토픽 제목을 정해서 해당 토픽 메세지만 받아온다.
    // Convert the list of topics to a format suitable for librdkafka.
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topicConvertCtrl.c_str(), RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        //g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);
	
    // Start polling for messages.

	unsigned long level_count=0;
	unsigned long level_counting=0;
    while (g_run) {
        rd_kafka_message_t *consumer_message;

		//CONSUMER_POLL_BLOCK_INTERVAL_TIME 시간만큼 대기했다가 메세지 들고옴
        consumer_message = rd_kafka_consumer_poll(consumer, CONSUMER_POLL_BLOCK_INTERVAL_TIME);
        if (!consumer_message) {
            //g_message("Waiting...");
			// 흐른 시간 기록
			time(&end_t);
			// 시간 차이 계산
			unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			//printf("%d min %d sec\n",sec/60, sec%60);
            continue;
        }
		
        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
			/*if(strcmp(rd_kafka_topic_name(consumer_message->rkt),topicConvertCtrl.c_str())!=0) {
				time(&end_t);
				// 시간 차이 계산
				unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
				printf("Another Topic  : %s -- %d min %d sec\n",rd_kafka_topic_name(consumer_message->rkt),sec/60, sec%60);
				continue;
				
			}*/
            g_message("Consumed event from topic %s: key = %.*s value = %s",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload
                      );
			//0. get option
			string retStr = stdformat("%s",consumer_message->payload);
			//printf("\n\n retStr=|%s|\n",retStr.c_str());
			//1. key
			((char*)consumer_message->key)[consumer_message->key_len]='\0';
			string key = stdformat("%s",consumer_message->key);
			//printf("\n\n key=|%s|\n",(char*)consumer_message->key);
			if(key.compare("LC")==0) {
				level_count = atoi(retStr.c_str());
				level_counting=0;
			}
			else if(key.compare("C")==0) {
				level_counting++;
				if(level_count!=0) {
					printf("Level Process : %5.1f% = (L : %d/%d) \n",(float)(level_counting*100./level_count),level_counting,level_count);
				}
				if(level_count==level_counting) {
					printf("Send Level End  KILL Message\n");
					//메세지 처리하면 1개 컨슈머당 3초 시간 소요, 100개면 300초를 컨슈머 종료에 소요. 배보다 배꼽이 더 커짐
					//string connect="192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092";
					//string topicConvert="imagesp6r1";
					//sendMessageEndLevelConvert(connect,topicConvert); 
					//가공 토픽에 강제 종료 kill 메세지 보내어 바로 종료하게 처리함. 속도개선
					sendKillMessageEndLevelConvert(pids);
					pids.clear();
				}
			}
			else if(key.compare("P")==0) {
				int pid = atoi(retStr.c_str());
				pids.push_back(pid);
				printf("start pid : %d - %d\n",pid,pids.size());
				//sleep(1);
			}
			// 흐른 시간 기록
			time(&end_t);
			// 시간 차이 계산
			unsigned long sec = (unsigned long)(double)difftime(end_t, start_t);
			printf("%d min %d sec\n\n",sec/60, sec%60);
			// Free the message when we're done.
			rd_kafka_message_destroy(consumer_message);
        }
    }//while end
	
    // Close the consumer: commit final offsets and leave the group.
    g_message( "Closing consumer - Convert Control");
    rd_kafka_consumer_close(consumer);

    // Destroy the consumer.
    rd_kafka_destroy(consumer);	
	return 0;
}
string g_connect="";
string g_topicConvert=""; //가공 토픽명
string g_topicConvertCtrl="";//가공관리 토픽명
pid_t g_pid_control; // 가공 관리 토픽 pid, 강제 종료용
// 종료 시 호출될 함수, 무조건 호출되어서 생성된 토픽 2개는 무조건 지움. 
void cleanup() {
    printf("프로그램이 종료됩니다. 토픽 정리 작업을 수행합니다.\n");
	kill(g_pid_control, SIGKILL);  // 가공 레벨 제어 토픽 강제 프로세스 강제 종료(이거 해야 레벨 제어 토픽 삭제 가능)
    // 추가적인 정리 작업을 여기에 작성
	delete_topic(g_connect,g_topicConvert.c_str());
	while(1) {
		bool bExist = check_topic_exists(g_connect.c_str(),g_topicConvert.c_str());
		if(!bExist) break;
		else delete_topic(g_connect,g_topicConvert.c_str());
		sleep(1);
	}	
	/////////////////////////////
	//가공 작업 레벨 컨트롤 토픽 삭제
	/////////////////////////////
	delete_topic(g_connect,g_topicConvertCtrl.c_str());
	while(1) {
		bool bExist = check_topic_exists(g_connect.c_str(),g_topicConvertCtrl.c_str());
		if(!bExist) break;
		else delete_topic(g_connect,g_topicConvertCtrl.c_str());
		sleep(1);
	}		
}

// 신호 처리기
void signal_handler(int signal) {
    cleanup();
    exit(signal); // 신호에 따라 종료
}

int main (int argc, char **argv) {
	///////////////////
	//1. check parameters
	///////////////////
	//if(argc != 9 ) {
	//	Usage();
	//	return 1;// paremeter count error
	//}
	// 시작 시간 기록
    time(&start_t);
	//////////////////////
	// Load librarys
	//////////////////////
	LoadLibrary();
	GDALStart();
	//./XDConvertTerrain 218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092 convertProcess 6 ../data/yeudo_1m_dem_4326.tif 4326 ../data/out 255
	//./XDConvertTerrain 218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092 convert convertControl convertProcess 6 ../data/yeudo_1m_dem_4326.tif 4326 ../data/out 255
	int i,j,k;
	i=j=0;
	printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
	printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
	printf("\\\\\\\\\\\\ KAFKA  가공 \\\\\\\\\\\\\\\\\n");
	printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
	printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
	for(i=0;i<argc;i++) {
		printf("[%d/%d] = %s\n",i+1,argc,argv[i]);
	}
	//연구소 브로커 "218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092"
	//unsigned long jobid = atoi(argv[1]);
	string connect = string(argv[1]);
	g_connect = connect;
	string topicConvert,topicConvertCtrl;
	//topicConvert = stdformat("%s",argv[2]);
	//topicConvertCtrl = stdformat("%s",argv[3]);
	string topicProcess = stdformat("%s",argv[2]);
	int partition=atoi(argv[3]);
	string src  = stdformat("%s",argv[4]);
	int    epsg = atoi(argv[5]);
	string dest;
	float noValue;
	pid_t pid_control;
	
	dest = stdformat("%s",argv[6]);
	noValue  = atof(argv[7]);
	string groupID = stdformat("%llu",generate_unique_value());
	printf("gid : %s\n",groupID.c_str());

	int replica=1;
	
	//////////////////////
	// Create convert topic
	//////////////////////
	topicConvert = stdformat("%s_convert",groupID.c_str());
	g_topicConvert = topicConvert;
	create_topic(connect,topicConvert.c_str(),partition,replica,NULL);
	while(1) {
		bool bExist = check_topic_exists(connect.c_str(),topicConvert.c_str());
		if(bExist) break;
		sleep(1);
	}	
	topicConvertCtrl = stdformat("%s_convertControl",groupID.c_str());
	g_topicConvertCtrl = topicConvertCtrl;
	create_topic(connect,topicConvertCtrl.c_str(),1,1,NULL);
	while(1) {
		bool bExist = check_topic_exists(connect.c_str(),topicConvertCtrl.c_str());
		if(bExist) break;
		sleep(1);
	}	

	
	//deleteTopicsByCommand(connect,topic,topicProcess);
	//creatTopicsByCommand(connect,topic,partition,replica,topicProcess);

	
	//////////////////////////////////////////////////////////
	// 가공 토픽 실시간 생성시에 내부적으로 가공 레벨 관리 프로세스 생성함
	// 고정된 가공 레벨 관리 프로세스 사용할 경우는 아래 사용안함.
	//////////////////////////////////////////////////////////
	pid_control = fork();  // 새로운 자식 프로세스 생성
	g_pid_control = pid_control;
	if(pid_control > 0) { //부모 프로세스에서 실행할 것들 
		// 부모 프로세스는 아무 작업을 하지 않음 , 부모 프로세느는 자기 하는일 그대로 진행
		//pid_control 는 자식프로세스 pid 이다. 나중에 이걸 이용해서 convertControl 프로세스를 강제 종료해야한다. 
		//그래야지 convertConvtrol 토픽이 삭제가능하다.
	}
    else if (pid_control == 0) { //자식 프로세스에서 실행할 것들
        // 자식 프로세스가 실행할 코드
		runConvertControl(connect,topicConvertCtrl);// 자식 프로세스 작업
		exit(0);  // 자식 프로세스 종료
    }
    else if (pid_control < 0) {
        // fork() 실패 시
        perror("fork 실패");
        exit(1);
    }
	
	//////////////////
	//0. 종료 신호 처리기 설정, 강제/정상/비정상 종료 모두 작동, 레벨 제어 프로세스 종료 및 2개 토픽 삭제
	// topicConvert,topicConvertCtrl 은 무조건 삭제 목표
	///////////////////
	signal(SIGINT, signal_handler); // Ctrl+C
	signal(SIGTERM, signal_handler); // kill 명령어 등
	signal(SIGQUIT, signal_handler); // Ctrl+\	

	///////////////////
	//2. get convert imagea list
	///////////////////
	//lonlat
	rect2dd rectImage = getImageBoundary(src);
	int level = getImageLevel(src,epsg);
	double resolution = getImageResolution(src);
	printf("level : %d\n",level);
	printf("resolution : %f\n",resolution);
	if(resolution==-1.0) {
		printf("Error : Get Resolution.\n");
		return 1;
	}
	//change to lonlat
	if(epsg!=4326) { //TM Rect --> lonlat Rect
		char srcCrd[128]={0,};
		char destCrd[128]={0,};
		double lon,lat,minx,miny,maxx,maxy;
		getEPSGString(epsg,srcCrd);
		getEPSGString(4326,destCrd);

		rect2dd tmpRect;
		//minx~~ , lonlat! LL,UR,UR,LR 
		//LL
		int er = pXDProjTransform(srcCrd,destCrd,rectImage.ix,rectImage.iy,&lon,&lat);
		if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
		minx = maxx = lon;
		miny = maxy = lat;
		//UL
		er = pXDProjTransform(srcCrd,destCrd,rectImage.ix,rectImage.ay,&lon,&lat);
		if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
		if(lon < minx) minx = lon; if(lat < miny) miny = lat;
		if(lon > maxx) maxx = lon; if(lat > maxy) maxy = lat;
		//UR
		er = pXDProjTransform(srcCrd,destCrd,rectImage.ax,rectImage.ay,&lon,&lat);
		if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
		if(lon < minx) minx = lon; if(lat < miny) miny = lat;
		if(lon > maxx) maxx = lon; if(lat > maxy) maxy = lat;
		//LR
		er = pXDProjTransform(srcCrd,destCrd,rectImage.ax,rectImage.iy,&lon,&lat);
		if(er!=0) { printf("Error : XDProjTransform %d \n",er); return -1.0;}
		if(lon < minx) minx = lon; if(lat < miny) miny = lat;
		if(lon > maxx) maxx = lon; if(lat > maxy) maxy = lat;
		tmpRect.ix = minx;		tmpRect.iy = miny;
		tmpRect.ax = maxx;		tmpRect.ay = maxy;
				
		rectImage = tmpRect;
	}
	GDALEnd();
	UnloadLibrary();
	//GDAL은 병렬 처리 내부에서 각각 초기화 해야함. 외부에서 초기화해서 인자로 관련된 자원으로 넘기면 GDAL은 오류남, thread 기반 병렬처리가 아직 unsafe ,process 기반 병렬 처리 사용 jsyun 20250224
	///////////////////
	//3. convert image using kafka
	///////////////////

	// 상위 DEM이 있어야지 경계가 절벽이 안 되고 그나마 부드럽게 연결됨. 12레벨 정도 하면 됨.
	// 0~12 레벨까지 이미 대한민국 만든 DEM을 먼저 복사한다.
	string strRun = stdformat("rsync -ah ./dem/* %s" ,dest.c_str());
	runCommand(strRun.c_str());
	//for(i=0;i<=12;i++) {
	//	string strRun = stdformat("cp \"./dem/%d\" \"%s\" -r",i,dest.c_str());
	//	runCommand(strRun.c_str());
	//}
	//return 0;
	//제일 하위 레벨 가공 목록을 보낸다.
	produce_tilelists(connect,topicConvert,topicConvertCtrl,topicProcess,level,rectImage,true); //제일 고해상도 만듬
	//단일 프로세스 작동 여부 점검용
	//consume_tilelists(connect,topicConvert,topicConvertCtrl,topicProcess,groupID,src,epsg,dest,rectImage); //가공 목록을 받아서 가공 한다.
	//가공 목록을 받아서 가공 한다.
	int num_children = partition;  // 자식 프로세스 수 ,Kafka 토픽의 partition 수에 맞추는 것이 효율적이다.
    pid_t pid;
	
	rd_kafka_t *producerConvertCtrl = getTopicProducer(connect); //레벨 작업
	
    for (i = 0; i < num_children; i++) {
        pid = fork();  // 새로운 자식 프로세스 생성

		if(pid > 0) { //부모 프로세스에서 실행할 것들 
			// 부모 프로세스는 아무 작업을 하지 않음 (자식이 끝나길 기다림)
			//자식 pid들을 모두 가공 컨트롤 컨슈머에 보내어서 등록시켜서 강제 종료에 사용한다.
			sendConvertCtrlMsg(producerConvertCtrl,topicConvertCtrl.c_str(),"P",stdformat("%d",pid)); 

		}
        else if (pid == 0) { //자식 프로세스에서 실행할 것들
            // 자식 프로세스가 실행할 코드
            //do_work(i + 1);  // 자식 프로세스 작업
			printf("최하위 레벨을 자식 프로세스[%d/%d]\n",i+1,num_children);
			consume_tilelists(connect,topicConvert,topicConvertCtrl,topicProcess,groupID,src,epsg,dest,rectImage); //가공 목록을 받아서 가공 한다.
            exit(0);  // 자식 프로세스 종료
        }
        else if (pid < 0) {
            // fork() 실패 시
            perror("fork 실패");
            exit(1);
        }
    }
	printf("최하위 레벨을 가공하는 모든 자식 프로세스가 완료 대기중.\n");
    // 부모는 모든 자식 프로세스가 종료될 때까지 기다림
    for (i = 0; i < num_children; i++) {
		
        wait(NULL);  // 자식 프로세스의 종료를 기다림
		printf("최하위 레벨을 가공하는 자식 프로세스가 완료함. [%d/%d]\n" , i+1,num_children);
    }
	
	
    printf("최하위 레벨을 가공하는 모든 자식 프로세스가 완료되었습니다.\n");
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	for(j=level-1;j>=12;j--) {
		produce_tilelists(connect,topicConvert,topicConvertCtrl,topicProcess,j,rectImage,false);
		//num_children 갯수 조절
		//if(j==12) { 
		//	if(num_children != 1 && num_children > 1 ) num_children = num_children/2;
		//}
		//consume_tilelists_Bottom2Top(connect,topicConvert,topicConvertCtrl,topicProcess,groupID,src,epsg,dest,j+1,j); //가공 목록을 받아서 가공 한다.
		
		for (i = 0; i < num_children; i++) {
			pid = fork();  // 새로운 자식 프로세스 생성
			if(pid > 0) { //부모 프로세스에서 실행할 것들 
				// 부모 프로세스는 아무 작업을 하지 않음 (자식이 끝나길 기다림)
				//자식 pid들을 모두 가공 컨트롤 컨슈머에 보내어서 등록시켜서 강제 종료에 사용한다.
				sendConvertCtrlMsg(producerConvertCtrl,topicConvertCtrl.c_str(),"P",stdformat("%d",pid)); 

			}
			else if (pid == 0) {
				// 자식 프로세스가 실행할 코드
				//do_work(i + 1);  // 자식 프로세스 작업
				consume_tilelists_Bottom2Top(connect,topicConvert,topicConvertCtrl,topicProcess,groupID,src,epsg,dest,j+1,j); //가공 목록을 받아서 가공 한다.
				exit(0);  // 자식 프로세스 종료
			}
			else if (pid < 0) {
				// fork() 실패 시
				perror("fork 실패");
				exit(1);
			}
		}

		// 부모는 모든 자식 프로세스가 종료될 때까지 기다림
		for (i = 0; i < num_children; i++) {
			wait(NULL);  // 자식 프로세스의 종료를 기다림
			printf("%d/%d 레벨을 가공하는 자식 프로세스가 완료함. [%d/%d]\n" ,j,level, i+1,num_children);
		}
		
		printf("%d/%d 레벨을 가공하는 모든 자식 프로세스가 완료되었습니다.\n",j,level);
	}
	
	
	//////////////////////
	// Delete convert topic
	//////////////////////
	//deleteTopicsByCommand(connect,topic,topicProcess);
	//convertControl 토픽 사용하는 프로세스를 강제 종료한다. 
	//안그러면 프로세스를 계속 물고 있어서 아래의 topicConvertCtrl 토픽이 삭제가 안되. 
	kill(pid_control, SIGKILL);  //가공 작업 레벨 컨트롤 프로세스 제거

	/*
	/////////////////////////////
	//가공 작업 토픽 삭제
	/////////////////////////////
	delete_topic(connect,topicConvert.c_str());
	while(1) {
		bool bExist = check_topic_exists(connect.c_str(),topicConvert.c_str());
		if(!bExist) break;
		else delete_topic(connect,topicConvert.c_str());
		sleep(1);
	}	
	/////////////////////////////
	//가공 작업 레벨 컨트롤 토픽 삭제
	/////////////////////////////
	delete_topic(connect,topicConvertCtrl.c_str());
	while(1) {
		bool bExist = check_topic_exists(connect.c_str(),topicConvertCtrl.c_str());
		if(!bExist) break;
		else delete_topic(connect,topicConvertCtrl.c_str());
		sleep(1);
	}	
	*/
    return 0;
}


	/*
	int N = 5;
 
    pid_t pid[N]; // process id 
    int childStatus;
    i;
 
    for (i = 0; i < N; i++)
    {
        pid[i] = fork();
        if (pid[i] == 0)
        {
            //printf("Now pid[%d] is die\n",i);
            //exit(100+i);
        }        
        // Create multiple child processes
		consume_tilelists(connect,topic,topicProcess,topicProcessCtrl,src,epsg,dest); //가공 목록을 받아서 가공 한다.
    }
 
    for (i = 0; i < N; i++)
    {
        pid_t terminatedChild = wait(&childStatus);
 
        if (WIFEXITED(childStatus))
        {
            // The child process has termindated normally
 
            printf("Child %d has terminated with exit status %d\n", terminatedChild, WEXITSTATUS(childStatus));
        }
 
        else
            printf("Child %d has terminated abnormally\n", terminatedChild);
    }	
	*/
	/* //fork 프로세스 방식
	pid_t pid = fork();
	if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // 자식 프로세스: 다른 프로그램 실행
        printf("Child process (PID: %d) executing 'ls -l'\n", getpid());
        //execlp("ls", "ls", "-l", NULL); // 'ls -l' 실행
		consume_tilelists(connect,topic,topicProcess,topicProcessCtrl,src,epsg,dest); //가공 목록을 받아서 가공 한다.
        perror("exec failed"); // exec 실패 시
        exit(1);
    } else {
		// 부모 프로세스: 자식 프로세스 종료 대기
        int status;
        pid_t child_pid = waitpid(pid, &status, 0);
        if (child_pid > 0) {
            printf("Parent process (PID: %d) detected child (PID: %d) termination\n", getpid(), child_pid);
            if (WIFEXITED(status)) {
                printf("Child exited with code: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child terminated by signal: %d\n", WTERMSIG(status));
            }
        } else {
            perror("waitpid failed");
        }
	}		
	*/
/*	
	//multi thread 방식
	CONSPARAM *param = new CONSPARAM();
	param->connect=connect;
	param->topic=topic;
	param->topicProcess=topicProcess;
	param->topicProcessCtrl=topicProcessCtrl;
	param->src=src;
	printf("src %s\n",param->src.c_str());
	param->epsg=epsg;
	param->dest=dest;
	int threadCount=1;
	pthread_t *threadID = new pthread_t[threadCount];
	for(i=0;i<threadCount;i++) {
		if(pthread_create(threadID+i, NULL, consume_tilelists_thread, (void*)param) != 0) { //1번 thread 생성
			fprintf(stderr, "thread create error\n");
			exit(1);
		}
	}
	fprintf(stderr, "thread close 1\n");
	for(i=0;i<threadCount;i++) pthread_join(threadID[i],NULL);
	fprintf(stderr, "thread close 2\n");
	///////////////////
	//3. create convert topic( partitions ) , level control topic 
	///////////////////
	string convertTopic = "convert0.topic";
	string levelCtrlTopic = "levelCtrl0.topic";*/
	//createTopic6(connect, convertTopic,6);
	//createTopic(connect, levelCtrlTopic,1);
	///////////////////
	//4. g_run convert topic , level control topic
	///////////////////

	///////////////////
	//5. delete convert topic , level control topic
	///////////////////
	