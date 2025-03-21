#include "XDRasterAPI.h"
//#define LIB_NAME_XDRASTER "/hadoop/lib/native/libXDRaster.so"
#define LIB_NAME_XDRASTER "./libXDRaster.so"
// image common
void* plibXDRaster=NULL;
double (*pXDGetNoDataValue)(char*)=NULL;
void (*pXDUpdateDSM)(char*,char*)=NULL;

//GEOTIFF
void (*pXDProjection)(char *,char*,char*, char*,double,double,double,double)=NULL;
char* (*pXDGetGeoImageInfo)(char *)=NULL;
void* (*pXDXTIFFOpen)(char *)=NULL;
void (*pXDTIFFGetField)(void *,int,int*)=NULL;
void* (*pXDGTIFNew)(void *)=NULL;
int (*pXDGTIFImageToPCS)(void *,double*,double*)=NULL;
void (*pXDXTIFFClose)(void *)=NULL;
bool (*pXDTIFFIsTiled)(void *)=NULL;
int (*pXDTIFFTileSize)(void *)=NULL;
void* (*_pXDTIFFmalloc)(long)=NULL;
long (*pXDTIFFReadTile)(void*,void*,int x,int y,int z,int s )=NULL;
void (*_pXDTIFFfree)(void *)=NULL;
unsigned int (*pXDTIFFNumberOfStrips)(void *)=NULL;
long (*pXDTIFFStripSize)(void*)=NULL;
long (*pXDTIFFReadEncodedStrip)(void* , unsigned int , void* , long )=NULL;
long (*pXDTIFFScanlineSize)(void*)=NULL;
//erdas img
void* (*pXDHFAOpen)(const char * , const char *  )=NULL;
void* (*pXDHFAGetDatum)(void* )=NULL;
void* (*pXDHFAGetMapInfo)(void* )=NULL;
void* (*pXDHFAGetProParameters)(void* )=NULL;
int   (*pXDHFAGetDataTypeBits)(int type)=NULL;
void* (*pXDHFAClose)(void*)=NULL;
void* (*pXDHFAGetRasterInfo)(void* ,int *, int *, int *)=NULL;
void XDLoadXDRaster() 
{
	plibXDRaster = dlopen( LIB_NAME_XDRASTER, RTLD_LAZY );
	if( plibXDRaster == NULL ) { fprintf(stderr,"error : GDAL loading failed \n");		return;	}		

	if(pXDGetNoDataValue==NULL) {
		pXDGetNoDataValue = (double(*)(char*))dlsym( plibXDRaster, "XDGetNoDataValue" );
		if(pXDGetNoDataValue == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	
	if(pXDUpdateDSM==NULL) {
		pXDUpdateDSM = (void(*)(char*,char*))dlsym( plibXDRaster, "XDUpdateDSM" );
		if(pXDUpdateDSM == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}

	if(pXDProjection==NULL) {
		pXDProjection = (void(*)(char*,char*,char*,char*,double,double,double,double))dlsym( plibXDRaster, "XDProjection" );
		if(pXDProjection == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	
	if(pXDXTIFFOpen==NULL) {
		pXDXTIFFOpen = (void*(*)(char*))dlsym( plibXDRaster, "XDXTIFFOpen" );
		if(pXDXTIFFOpen == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	if(pXDTIFFGetField==NULL) {
		pXDTIFFGetField = (void(*)(void*,int,int*))dlsym( plibXDRaster, "XDTIFFGetField" );
		if(pXDTIFFGetField == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	
	if(pXDGTIFNew==NULL) {
		pXDGTIFNew = (void*(*)(void*))dlsym( plibXDRaster, "XDGTIFNew" );
		if(pXDGTIFNew == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	if(pXDGTIFImageToPCS==NULL) {
		pXDGTIFImageToPCS = (int(*)(void *,double*,double*))dlsym( plibXDRaster, "XDGTIFImageToPCS" );
		if(pXDGTIFImageToPCS == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	if(pXDXTIFFClose==NULL) {
		pXDXTIFFClose = (void(*)(void *))dlsym( plibXDRaster, "XDXTIFFClose" );
		if(pXDXTIFFClose == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	if(pXDTIFFIsTiled==NULL) {
		pXDTIFFIsTiled = (bool(*)(void*))dlsym( plibXDRaster, "XDTIFFIsTiled" );
		if(pXDTIFFIsTiled == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(pXDTIFFTileSize==NULL) {
		pXDTIFFTileSize = (int(*)(void*))dlsym( plibXDRaster, "XDTIFFTileSize" );
		if(pXDTIFFTileSize == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(_pXDTIFFmalloc==NULL) {
		_pXDTIFFmalloc = (void*(*)(long))dlsym( plibXDRaster, "_XDTIFFmalloc" );
		if(_pXDTIFFmalloc == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	
	if(pXDTIFFReadTile==NULL) {
		pXDTIFFReadTile = (long(*)(void*, void*,int,int,int,int))dlsym( plibXDRaster, "XDTIFFReadTile" );
		if(pXDTIFFReadTile == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(_pXDTIFFfree==NULL) {
		_pXDTIFFfree = (void(*)(void*))dlsym( plibXDRaster, "_XDTIFFfree" );
		if(_pXDTIFFfree == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(pXDTIFFNumberOfStrips==NULL) {
		pXDTIFFNumberOfStrips = (unsigned int(*)(void*))dlsym( plibXDRaster, "XDTIFFNumberOfStrips" );
		if(pXDTIFFNumberOfStrips == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(pXDTIFFStripSize==NULL) {
		pXDTIFFStripSize = (long(*)(void*))dlsym( plibXDRaster, "XDTIFFStripSize" );
		if(pXDTIFFStripSize == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(pXDTIFFReadEncodedStrip==NULL) {
		pXDTIFFReadEncodedStrip = (long(*)(void*,unsigned int,void*,long))dlsym( plibXDRaster, "XDTIFFReadEncodedStrip" );
		if(pXDTIFFReadEncodedStrip == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}		
	if(pXDTIFFScanlineSize==NULL) {
		pXDTIFFScanlineSize = (long(*)(void*))dlsym( plibXDRaster, "XDTIFFScanlineSize" );
		if(pXDTIFFScanlineSize == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	
	if(pXDHFAOpen==NULL) {
		pXDHFAOpen = (void*(*)(const char * , const char * ))dlsym( plibXDRaster, "XDHFAOpen" );
		if(pXDHFAOpen == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	
	if(pXDHFAGetDatum==NULL) {
		pXDHFAGetDatum = (void*(*)(void*))dlsym( plibXDRaster, "XDHFAGetDatum" );
		if(pXDHFAGetDatum == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	

	if(pXDHFAGetMapInfo==NULL) {
		pXDHFAGetMapInfo = (void*(*)(void*))dlsym( plibXDRaster, "XDHFAGetMapInfo" );
		if(pXDHFAGetMapInfo == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	

	if(pXDHFAGetProParameters==NULL) {
		pXDHFAGetProParameters = (void*(*)(void*))dlsym( plibXDRaster, "XDHFAGetProParameters" );
		if(pXDHFAGetProParameters == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	

	if(pXDHFAGetDataTypeBits==NULL) {
		pXDHFAGetDataTypeBits = (int(*)(int))dlsym( plibXDRaster, "XDHFAGetDataTypeBits" );
		if(pXDHFAGetDataTypeBits == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	

	if(pXDHFAClose==NULL) {
		pXDHFAClose = (void*(*)(void*))dlsym( plibXDRaster, "XDHFAClose" );
		if(pXDHFAClose == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
	if(pXDHFAGetRasterInfo==NULL) {
		pXDHFAGetRasterInfo = (void*(*)(void* ,int * , int * , int *))dlsym( plibXDRaster, "XDHFAGetRasterInfo" );
		if(pXDHFAGetRasterInfo == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}	
}
void XDUnloadXDRaster()
{
	if(plibXDRaster) dlclose(plibXDRaster);	
}

