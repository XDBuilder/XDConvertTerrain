// Copyright 2022 GDAL
#ifndef XDWORLDBUILDERPROB5_GDAL23_GDALMANAGE_H_
#define XDWORLDBUILDERPROB5_GDAL23_GDALMANAGE_H_

#include <string>
// 1. related header
// 2. c System library
// 3. c++ System library
// 4. external library header
#include "gdal/include/gdal_priv.h"
#include "gdal/include/ogrsf_frmts.h"
#include "gdal/include/gdal_alg.h"
#include "gdal/include/cpl_string.h"
#include "gdal/include/cpl_conv.h"
#include "gdal/include/ogr_srs_api.h"
#include "gdal/include/gdal_utils.h"
#include "gdal/include/gdalwarper.h"


#include "gdal/include/commonutils.h"
#include "gdal/include/gdal_utils_priv.h"

// 5. project header
#include "base/rect2d.h"
using namespace std;

// API 생성 규칙
// 그룹_동사_목적_분류순으로 구성
/*
EX)
	생성 : Create 	[ 레이어 기본 생성 ]
	동작 : Try		[ 분석 시작 ex) 그림자 분석 시작 ]
	변환 : Convert	[ 3ds, obj와 같은 파일로 객체 생성 시 ]
	만듬 : Make		[ 사용자 입력 값, 좌표 리스트 ]
	변경 : Edit 	[ 레이어 기본 요소 ex)타입, 명칭, 레벨 ]
	적용 : Apply 	[ 레이어 추가 요소 ex)WMS 설정 옵션 ]
	삭제
		Delete : 자기 자신 삭제
		Remove : 구성 요소 삭제
	파일 불러오기 : Import
	파일 내보내기 : Export
	단일 변경 : set	[ 변수 변경 ]
	단일 확인 : get	[ 변수 확인 ]
	모드 확인 : Is	[ 투명도, Z-Buffer 모드 ]
	검색 : Find		[ 구성요소 검색 ]
	삽입 : Add		[ 구성요소 삽입 ]
	요청 : Request	[ 레이어 요청 ]

	기존 : XDEStopAntoMoveTransparency
	변경 : XDEMoveStopTransparencyAnalysis

구글 스타일 관련 내용 정리 
참조 URL : https://docs.google.com/presentation/d/1lBLw5w01dXtPtIhyGS3CeQCEgmgGKKywba_5yglYc3U/edit#slide=id.ge11ea0fc2c_0_6
*/
// 좌표변환
// 주석 임시

string GDALTransFormWKTToProj(string _str_wkt);							// WKT TO PROJ4
bool GDALFindSRS(const char *pszInput, OGRSpatialReference &oSRS);		// check Gdal WKT
float *GDALTransFormDEM(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue);
char *GDALTransFormImage(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize,int &imageWidth,int &imageHeight,int imgType, char r=0,char g=0,char b=0,char a=0,char *method=0);
bool GDALTransFormImageTest(char* src, char* dest, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt);
rect2dd GDALGetImageRect(char *src);
float *GDALGetDEMRect(char *src, int *witdh);
void GDALStart();
void GDALEnd();
char *SanitizeSRS(const char *pszUserInput);
short GDALGetBandCount(char *src);
bool GDALIsDEM(char *src);
char *GDALGetCityGML(char* src, char* srcCoord, char* destCoord, unsigned int &isize, int gmlType); //gmlType : 0: cityGML 2.0
char * GDALCheckTransparencyColor(char *src, char r, char g, char b, char r2, char g2, char b2);

GDALDatasetH *GDALGetTransFormHandleDEM(char* src);
GDALWarpAppOptions *GDALSetTransFormOptionDEM(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue);
float * GDALTransFormImageByHandleDEM(GDALDatasetH *pahSrcDS, int nSrcCount, GDALWarpAppOptions *psOptions, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue);
void GDALFreeHandleDEM(GDALDatasetH *pahSrcDS, int  nSrcCount);

GDALDatasetH *GDALGetTransFormHandleIMG(char* src);
GDALWarpAppOptions *GDALSetTransFormOptionIMG(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize, int &imageWidth, int &imageHeight, int imgType, char r = 0, char g = 0, char b = 0, char a = 0, char *method = 0,char *fname=0);
char * GDALTransFormImageByHandleIMG(GDALDatasetH *pahSrcDS, int nSrcCount, GDALWarpAppOptions *psOptions, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize, int &imageWidth, int &imageHeight, int imgType, char r = 0, char g = 0, char b = 0, char a = 0);
void GDALFreeHandleIMG(GDALDatasetH *pahSrcDS, int  nSrcCount);

#endif