#include "XDProjAPI.h"
//#define LIB_NAME_XDPROJ "/hadoop/lib/native/libXDProj.so"
#define LIB_NAME_XDPROJ "./libXDProj.so"
void* plibXDProj=NULL;
int (*pXDProjTransform)(char *,char*,double,double,double*,double*)=NULL;
int (*pXDCoordinateTransForm)(char *, char *, double, double, double *, double *)=NULL;

void XDLoadXDProj() 
{
	plibXDProj = dlopen( LIB_NAME_XDPROJ, RTLD_LAZY );
	if( plibXDProj == NULL ) { fprintf(stderr,"error : Proj loading failed \n");		return;	}		
	
	if(pXDProjTransform==NULL) {
		pXDProjTransform = (int(*)(char*,char*,double,double,double*,double*))dlsym( plibXDProj, "XDProjTransform" );
		if(pXDProjTransform == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
	
	if(pXDCoordinateTransForm==NULL) {
		pXDCoordinateTransForm = (int(*)(char*,char*,double,double,double*,double*))dlsym( plibXDProj, "XDCoordinateTransForm" );
		if(pXDCoordinateTransForm == NULL) { fprintf(stderr,"error : %s \n",dlerror()); return ; }
	}
}
void XDUnloadXDProj()
{
	if(plibXDProj) dlclose(plibXDProj);	
}