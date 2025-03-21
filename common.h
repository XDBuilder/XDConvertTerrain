#ifndef __COMMON_H_INCLUDED__
#define __COMMON_H_INCLUDED__
#include <glib.h>
#include <librdkafka/rdkafka.h>

#include <string>
#include <algorithm> 
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
#include <sys/time.h>
#include "base/rect2d.h"
using namespace std;
class TMIdentify //지형 식별 아이디....
{
	public:

		TMIdentify( char l,int x,int y ) : level(l), idx(x), idy(y) {}

		virtual ~TMIdentify() {}

		int		level;  
		int		idx;
		int		idy;
};

typedef struct tagSIZE
{
    int        cx;
    int        cy;
} SIZE;
#define SAFE_DELETE(p)		{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p){ if(p) { delete [] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)		{ if(p) { (p)->Release(); (p)=NULL; } }
// pXDCoordinateTransForm 반환값에 사용
#define RAD_TO_DEG	57.295779513082321
#define DEG_TO_RAD	.017453292519943296


typedef u32   COLORREF;
#define GetRValue(rgb)      (u8(rgb))
#define GetGValue(rgb)      (u8((rgb)>> 8))
#define GetBValue(rgb)      (u8((rgb)>>16))
#define GetAValue(rgb)      (u8((rgb)>>24))
s32 getPowerOfTwo(s32 size);
#define RGB(r,g,b)          ((COLORREF)(((u8)(r)|((u16)((u8)(g))<<8))|(((u32)(u8)(b))<<16)))
#define RGBA(r,g,b,a)        ((COLORREF)( (((u8)(a))<<24) |     RGB(r,g,b) ))       
typedef struct tagBITMAPINFOHEADER{
        u32      biSize;
        int       biWidth;
        int       biHeight;
        u16       biPlanes;
        u16       biBitCount;
        u32      biCompression;
        u32      biSizeImage;
        int       biXPelsPerMeter;
        int       biYPelsPerMeter;
        u32      biClrUsed;
        u32      biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;
typedef struct tagRGBQUAD {
        u8    rgbBlue;
        u8    rgbGreen;
        u8    rgbRed;
        u8    rgbReserved;
} __attribute__((packed)) RGBQUAD;
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} __attribute__((packed)) BITMAPINFO,__attribute__((packed)) *LPBITMAPINFO;
typedef struct tagPALETTEENTRY {
    u8        peRed;
    u8        peGreen;
    u8        peBlue;
    u8        peFlags;
} __attribute__((packed)) PALETTEENTRY;
typedef struct tagLOGPALETTE {
    u16        palVersion;
    u16        palNumEntries;
    PALETTEENTRY        palPalEntry[1];
} __attribute__((packed)) LOGPALETTE,__attribute__((packed)) *LPLOGPALETTE;
typedef struct tagDDS_PIXELFORMAT
{
    u32 dwSize;
    u32 dwFlags;
    u32 dwFourCC;
    u32 dwRGBBitCount;
    u32 dwRBitMask;
    u32 dwGBitMask;
    u32 dwBBitMask;
    u32 dwABitMask;
} __attribute__((packed)) DDS_PIXELFORMAT;
typedef struct tagDDS_HEADER
{
    u32 dwSize;			//구조체의 사이즈. 이 멤버는 반드시 124 로 설정하는 것. 
    u32 dwHeaderFlags;	//유효한 필드를 나타내는 플래그. 항상 DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH, DDSD_HEIGHT 를 포함하는 것. 
    u32 dwHeight;			//메인 이미지의 픽셀 단위의 높이
    u32 dwWidth;			//메인 이미지의 픽셀 단위의 폭
    u32 dwPitchOrLinearSize;//미압축 포맷의 경우는, 메인 이미지의 주사선 근처의 바이트수 (u32 로 정렬). 이 경우는 dwFlags 에 DDSD_PITCH 를 포함한다. 
							//압축 포맷의 경우는, 메인 이미지의 총바이트수. 이 경우는 dwFlags 에 DDSD_LINEARSIZE 를 포함한다. 
    u32 dwDepth;			//볼륨 텍스처의 경우는, 볼륨의 깊이. 이 경우는 dwFlags 에 DDSD_DEPTH 를 포함한다. 
							// only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
    u32 dwMipMapCount;	//밉맵 레벨이 정의된 밉맵의 경우는, 메인 이미지의 밉맵 체인에 포함되는 총레벨수. 이 경우는 dwFlags 에 DDSD_MIPMAPCOUNT 를 포함한다
    u32 dwReserved1[11];
    DDS_PIXELFORMAT ddspf;	//픽셀 포맷의 구조체를 지정하는 32 바이트의 값. 
    u32 dwSurfaceFlags;   //능력의 구조체를 지정하는 16 바이트의 값. 
    u32 dwCubemapFlags;
    u32 dwReserved2[3];
} __attribute__((packed)) DDS_HEADER;
typedef struct tagBITMAPCOREHEADER {
	u32   bcSize;
	u16    bcWidth;
	u16    bcHeight;
	u16    bcPlanes;
	u16    bcBitCount;
} __attribute__((packed)) BITMAPCOREHEADER;
#define ByteSwap16(n) ( ((((unsigned int) n) << 8) & 0xFF00) | ((((unsigned int) n) >> 8) & 0x00FF) ) 
#define ByteSwap32(n) ( ((((unsigned long) n) << 24) & 0xFF000000) | ((((unsigned long) n) <<  8) & 0x00FF0000) | ((((unsigned long) n) >>  8) & 0x0000FF00) | ((((unsigned long) n) >> 24) & 0x000000FF) ) 
	////////////// Endian swaping wrapper functions //////////
	inline void fwrite32(u32 blah, FILE *f){    
		u32 item = ByteSwap32(blah);    
		fwrite(&item, 4, 1, f);
	} 
	inline void fwrite16(u16 blah, FILE *f){    
		u16 item = ByteSwap16(blah);    
		fwrite(&item, 2, 1, f);
	}
char* GetFileName(char* str);
char* GetCharPath(char* str);
class JPG_DATA
{
	public:
		JPG_DATA( u8* data, u32 size ) : jpgsize(size) 	
		{
			jpgdata = new u8[jpgsize];
			memcpy(jpgdata,data,jpgsize);
		}
		JPG_DATA() : jpgdata(0), jpgsize(0) {}  	
		~JPG_DATA() { SAFE_DELETE_ARRAY(jpgdata); }

		u32			jpgsize;
		u8*			jpgdata;	

} ;
/* Wrapper to set config values and error out if needed.
 */
static void set_config(rd_kafka_conf_t *conf, char *key, char *value) {
    char errstr[512];
    rd_kafka_conf_res_t res;

    res = rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr));
    if (res != RD_KAFKA_CONF_OK) {
        g_error("Unable to set config: %s", errstr);
        exit(1);
    }
}
/* Optional per-message delivery callback (triggered by poll() or flush())
 * when a message has been successfully delivered or permanently
 * failed delivery (after retries).
 */
static void dr_msg_cb (rd_kafka_t *kafka_handle,
                       const rd_kafka_message_t *rkmessage,
                       void *opaque) {
    if (rkmessage->err) {
        g_error("Message delivery failed: %s", rd_kafka_err2str(rkmessage->err));
    }
}
void getMeshWithImages( int level,rect2dd _imageRect, std::vector<TMIdentify>& meshlist );
unsigned long getMeshWithImagesCount( int level, rect2dd _imageRect );
void getStringInteger(int value, c8* temp);
inline int ilog10(int v){
	 return (v >= 1000000000u) ? 9 : (v >= 100000000u) ? 8 :
	        (v >= 10000000u) ? 7 : (v >= 1000000u) ? 6 :
	        (v >= 100000u) ? 5 : (v >= 10000u) ? 4 :
	        (v >= 1000u) ? 3 : (v >= 100u) ? 2 : (v >= 10u) ? 1 : 0;
}

string stdformat(const char* fmt, ...);
//void mkdirs(string path);
vector<string> split(string input, char delimiter);
void enhance_edges(u8* the_image, u8* out_image, int rows, int cols, int band, int high, int rowBytes);
void ImageResample(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH,int Bits,bool bForce=false);
unsigned long long generate_unique_value();
void Resampleing(float* sDIB, int SW, int SH, float* dDIB, int DW, int DH,char flag);
string GetCVTFileName(string _str_name);	// 파일 명칭 반환
string GetCVTPath(string _str_name);		// 파일 명칭 제외한 패스 반환
string GetCVTFileExtension(string _str_name);	// 파일 확장자 반환
float b3spline2(float x);
#endif