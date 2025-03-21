#ifndef _XDPROJAPI_H_
#define _XDPROJAPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <dlfcn.h>

//외부 변수 선언

extern int (*pXDProjTransform)(char *,char*,double,double,double*,double*);
extern int (*pXDCoordinateTransForm)(char *, char *, double, double, double *, double *);

void XDLoadXDProj();
void XDUnloadXDProj();

#endif