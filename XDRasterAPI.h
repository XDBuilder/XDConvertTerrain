#ifndef _XDRASTERAPI_H_
#define _XDRASTERAPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <dlfcn.h>

//외부 변수 선언
//COMMON
extern double (*pXDGetNoDataValue)(char *);
extern void (*pXDUpdateDSM)(char *,char *);
//GEOTIFF
extern void (*pXDProjection)(char *,char*,char*, char*,double,double,double,double);
extern char* (*pXDGetGeoImageInfo)(char *);
extern void* (*pXDXTIFFOpen)(char *);
extern void (*pXDTIFFGetField)(void *,int,int*);
extern void* (*pXDGTIFNew)(void *);
extern int (*pXDGTIFImageToPCS)(void *,double*,double*);
extern void (*pXDXTIFFClose)(void *);
extern bool (*pXDTIFFIsTiled)(void *);
extern int (*pXDTIFFTileSize)(void *);
extern void* (*_pXDTIFFmalloc)(long );
extern long (*pXDTIFFReadTile)(void*,void*,int x,int y,int z,int s );
extern void (*_pXDTIFFfree)(void *);
extern unsigned int (*pXDTIFFNumberOfStrips)(void *);
extern long (*pXDTIFFStripSize)(void*);
extern long (*pXDTIFFReadEncodedStrip)(void* , unsigned int , void* , long );
extern long (*pXDTIFFScanlineSize)(void*);
//ERDAS IMG
extern void* (*pXDHFAOpen)(const char * , const char *  );
extern void* (*pXDHFAGetDatum)(void* );
extern void* (*pXDHFAGetMapInfo)(void* );
extern void* (*pXDHFAGetProParameters)(void* );
extern int   (*pXDHFAGetDataTypeBits)(int type);
extern void* (*pXDHFAClose)(void*);
extern void* (*pXDHFAGetRasterInfo)(void* handle,int *width, int *height, int *nBand);
void XDLoadXDRaster();
void XDUnloadXDRaster();

#endif