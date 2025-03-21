#ifndef __SPLINE_H_INCLUDED__
#define __SPLINE_H_INCLUDED__

//#include "xdmath.h"
#include "vector3d.h"

double SplineBlend(int k,int t,int *u,double v);

inline double SplineBlend(int k,int t,int *u,double v)
{
   double value;

   if (t == 1) {
      if ((u[k] <= v) && (v < u[k+1]))
         value = 1;
      else
         value = 0;
   } else {
      if ((u[k+t-1] == u[k]) && (u[k+t] == u[k+1]))
         value = 0;
      else if (u[k+t-1] == u[k]) 
         value = (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
      else if (u[k+t] == u[k+1])
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v);
     else
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v) + 
                 (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
   }
   return(value);
}

inline void SplinePoint(int *u,int n,int t,double v,vector3dd *control,vector3dd *output)
{
   int k;
   double b;

   output->x = 0;
   output->y = 0;
   output->z = 0;

   for (k=0;k<=n;k++) {
      b = SplineBlend(k,t,u,v);
      output->x += control[k].x * b;
      output->y += control[k].y * b;
      output->z += control[k].z * b;
   }
}

inline void SplineKnots(int *u,int n,int t)
{
   int j;

   for (j=0;j<=n+t;j++) {
      if (j < t)
         u[j] = 0;
      else if (j <= n)
         u[j] = j - t + 1;
      else if (j > n)
         u[j] = n - t + 2;	
   }
}

inline void SplineCurve(vector3dd *inp,int n,int *knots,int t,vector3dd *outp,int res)
{
   int i;
   double interval,increment;

   interval = 0;
   increment = (n - t + 2) / (double)(res - 1);
   for (i=0;i<res-1;i++) {
      SplinePoint(knots,n,t,interval,inp,&(outp[i]));
      interval += increment;
   }
   outp[res-1] = inp[n];
}


#endif

