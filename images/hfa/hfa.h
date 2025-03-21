/******************************************************************************
 * $Id: hfa.h,v 1.19 2006/04/03 04:34:19 fwarmerdam Exp $
 *
 * Project:  Erdas Imagine (.img) Translator
 * Purpose:  Public (C callable) interface for the Erdas Imagine reading
 *           code.  This include files, and it's implementing code depends
 *           on CPL, but not GDAL. 
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Intergraph Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: hfa.h,v $
 * Revision 1.19  2006/04/03 04:34:19  fwarmerdam
 * added support for reading affine polynomial transforms as geotransform
 *
 * Revision 1.18  2006/03/29 14:24:04  fwarmerdam
 * added preliminary nodata support (readonly)
 *
 * Revision 1.17  2005/12/21 05:30:45  fwarmerdam
 * return compression type as metadata
 *
 * Revision 1.16  2005/08/19 02:14:11  fwarmerdam
 * bug 857: add ability to set layer names
 *
 * Revision 1.15  2005/05/13 04:57:28  fwarmerdam
 * fix handling of large offsets in ige file for HFACreateLayer()
 *
 * Revision 1.14  2005/05/13 02:05:54  fwarmerdam
 * adjusted CreateLayer
 *
 * Revision 1.13  2005/05/10 00:57:40  fwarmerdam
 * added HFACreateLayer, HFACreateOverview
 *
 * Revision 1.12  2005/04/04 13:14:00  fwarmerdam
 * Added HFAGetClassNames() (bug 819).
 *
 * Revision 1.11  2003/05/13 19:32:10  warmerda
 * support for reading and writing opacity provided by Diana Esch-Mosher
 *
 * Revision 1.10  2003/04/22 19:40:36  warmerda
 * fixed email address
 *
 * Revision 1.9  2003/03/18 21:06:07  dron
 * Added HFADelete() function.
 *
 * Revision 1.8  2002/05/21 15:09:12  warmerda
 * read/write support for GDAL_MetaData table now supported
 *
 * Revision 1.7  2000/10/20 04:18:15  warmerda
 * added overviews, stateplane, and u4
 *
 * Revision 1.6  2000/10/13 21:14:08  warmerda
 * added more projection codes
 *
 * Revision 1.5  2000/10/12 19:30:31  warmerda
 * substantially improved write support
 *
 * Revision 1.4  2000/09/29 21:42:38  warmerda
 * preliminary write support implemented
 *
 * Revision 1.3  1999/01/22 17:39:09  warmerda
 * Added projections structures, and various other calls
 *
 * Revision 1.2  1999/01/04 22:52:47  warmerda
 * field access working
 *
 * Revision 1.1  1999/01/04 05:28:13  warmerda
 * New
 *
 */

#ifndef _HFAOPEN_H_INCLUDED
#define _HFAOPEN_H_INCLUDED

/* -------------------------------------------------------------------- */
/*      Include standard portability stuff.                             */
/* -------------------------------------------------------------------- */
#include "../port/cpl_conv.h"
#include "../port/cpl_string.h"

#ifdef HFA_PRIVATE
typedef HFAInfo_t *HFAHandle;
#else
typedef void *HFAHandle;
#endif

/* -------------------------------------------------------------------- */
/*      Structure definitions from eprj.h, with some type               */
/*      simplifications.                                                */
/* -------------------------------------------------------------------- */
typedef struct {
	double x;			/* coordinate x-value */
	double y;			/* coordinate y-value */
} Eprj_Coordinate;


typedef struct {
	double width;		/* pixelsize width */
	double height;		/* pixelsize height */
} Eprj_Size;


typedef struct {
	char * proName;		/* projection name */
	Eprj_Coordinate upperLeftCenter;	/* map coordinates of center of
						   upper left pixel */
	Eprj_Coordinate lowerRightCenter;	/* map coordinates of center of
						   lower right pixel */
	Eprj_Size pixelSize;			/* pixel size in map units */
	char * units;			/* units of the map */
} Eprj_MapInfo;

typedef enum {
	EPRJ_INTERNAL,		/* Indicates that the projection is built into
				   the eprj package as function calls */
	EPRJ_EXTERNAL		/* Indicates that the projection is accessible
				   as an EXTERNal executable */
} Eprj_ProType;

typedef enum {
	EPRJ_NAD27=1,		/* Use the North America Datum 1927 */
	EPRJ_NAD83=2,		/* Use the North America Datum 1983 */
	EPRJ_HARN		/* Use the North America Datum High Accuracy
				   Reference Network */
} Eprj_NAD;

typedef enum {
	EPRJ_DATUM_PARAMETRIC,		/* The datum info is 7 doubles */
	EPRJ_DATUM_GRID,		/* The datum info is a name */
	EPRJ_DATUM_REGRESSION,
	EPRJ_DATUM_NONE
} Eprj_DatumType;

typedef struct {
	char * datumname;		/* name of the datum */
	Eprj_DatumType type;			/* The datum type */
	double  params[7];		/* The parameters for type
						   EPRJ_DATUM_PARAMETRIC */
	char * gridname;		/* name of the grid file */
} Eprj_Datum;

typedef struct {
	char * sphereName;	/* name of the ellipsoid */
	double a;			/* semi-major axis of ellipsoid */
	double b;			/* semi-minor axis of ellipsoid */
	double eSquared;		/* eccentricity-squared */
	double radius;			/* radius of the sphere */
} Eprj_Spheroid;

typedef struct {
	Eprj_ProType proType;		/* projection type */
	long proNumber;			/* projection number for internal 
					   projections */
	char * proExeName;	/* projection executable name for
					   EXTERNal projections */
	char * proName;	/* projection name */
	long proZone;			/* projection zone (UTM, SP only) */
	double proParams[15];	/* projection parameters array in the
					   GCTP form */
	Eprj_Spheroid proSpheroid;	/* projection spheroid */
} Eprj_ProParameters;

/* -------------------------------------------------------------------- */
/*      Prototypes                                                      */
/* -------------------------------------------------------------------- */

CPL_C_START

HFAHandle CPL_DLL HFAOpen( const char * pszFilename, const char * pszMode );
void	CPL_DLL HFAClose( HFAHandle );
CPLErr HFADelete( const char *pszFilename );

HFAHandle CPL_DLL HFACreateLL( const char *pszFilename );
HFAHandle CPL_DLL HFACreate( const char *pszFilename, int nXSize, int nYSize, 
                             int nBands, int nDataType, char ** papszOptions );
CPLErr  CPL_DLL HFAFlush( HFAHandle );
int CPL_DLL HFACreateOverview( HFAHandle hHFA, int nBand, int nOverviewLevel);

const Eprj_MapInfo CPL_DLL *HFAGetMapInfo( HFAHandle );
int CPL_DLL HFAGetGeoTransform( HFAHandle, double* );
CPLErr CPL_DLL HFASetMapInfo( HFAHandle, const Eprj_MapInfo * );
const Eprj_Datum CPL_DLL *HFAGetDatum( HFAHandle );
CPLErr CPL_DLL HFASetDatum( HFAHandle, const Eprj_Datum * );
const Eprj_ProParameters CPL_DLL *HFAGetProParameters( HFAHandle );
CPLErr CPL_DLL HFASetProParameters( HFAHandle, const Eprj_ProParameters * );

CPLErr CPL_DLL HFAGetRasterInfo( HFAHandle hHFA, int *pnXSize, int *pnYSize,
                                 int *pnBands );
CPLErr CPL_DLL HFAGetBandInfo( HFAHandle hHFA, int nBand, int * pnDataType,
                               int * pnBlockXSize, int * pnBlockYSize, 
                               int * pnOverviews, int *pnCompressionType );
int    CPL_DLL HFAGetBandNoData( HFAHandle hHFA, int nBand, double *pdfValue );
CPLErr CPL_DLL HFAGetOverviewInfo( HFAHandle hHFA, int nBand, int nOverview, 
                                   int * pnXSize, int * pnYSize,
                                   int * pnBlockXSize, int * pnBlockYSize );
CPLErr CPL_DLL HFAGetRasterBlock( HFAHandle hHFA, int nBand, int nXBlock, 
                                  int nYBlock, void * pData );
CPLErr CPL_DLL HFAGetOverviewRasterBlock( HFAHandle hHFA, int nBand, 
                                          int iOverview,
                                   int nXBlock, int nYBlock, void * pData );
CPLErr CPL_DLL HFASetRasterBlock( HFAHandle hHFA, int nBand, 
                                  int nXBlock, int nYBlock,
                                  void * pData );
CPLErr CPL_DLL HFASetOverviewRasterBlock( 
    HFAHandle hHFA, int nBand, int iOverview,int nXBlock, int nYBlock, 
    void * pData );
const char * HFAGetBandName( HFAHandle hHFA, int nBand );
void HFASetBandName( HFAHandle hHFA, int nBand, const char *pszName );
int     CPL_DLL HFAGetDataTypeBits( int );
const char CPL_DLL *HFAGetDataTypeName( int );
CPLErr	CPL_DLL HFAGetPCT( HFAHandle, int, int *, 
                           double **, double **, double ** , double **);
CPLErr  CPL_DLL HFASetPCT( HFAHandle, int, int, double *, double *, double *, double * );
void    CPL_DLL HFADumpTree( HFAHandle, FILE * );
void    CPL_DLL HFADumpDictionary( HFAHandle, FILE * );
CPLErr  CPL_DLL HFAGetDataRange( HFAHandle, int, double *, double * );
char  CPL_DLL **HFAGetMetadata( HFAHandle hHFA, int nBand );
CPLErr  CPL_DLL HFASetMetadata( HFAHandle hHFA, int nBand, char ** );
char  CPL_DLL **HFAGetClassNames( HFAHandle hHFA, int nBand );
int CPL_DLL 
HFACreateLayer( HFAHandle psInfo, HFAEntry *poParent,
                const char *pszLayerName,
                int bOverview, int nBlockSize, 
                int bCreateCompressed, int bCreateLargeRaster,
                int nXSize, int nYSize, int nDataType, 
                char **papszOptions,
                
                // these are only related to external (large) files
                GIntBig nStackValidFlagsOffset, 
                GIntBig nStackDataOffset,
                int nStackCount, int nStackIndex );

/* -------------------------------------------------------------------- */
/*      data types.                                                     */
/* -------------------------------------------------------------------- */
#define EPT_u1	0
#define EPT_u2	1
#define EPT_u4	2
#define EPT_u8	3
#define EPT_s8	4
#define EPT_u16	5
#define EPT_s16	6
#define EPT_u32	7
#define EPT_s32	8
#define EPT_f32	9
#define EPT_f64	10
#define EPT_c64	11
#define EPT_c128 12

/* -------------------------------------------------------------------- */
/*      Projection codes.                                               */
/* -------------------------------------------------------------------- */
#define EPRJ_LATLONG				0
#define EPRJ_UTM				1
#define EPRJ_STATE_PLANE 			2
#define EPRJ_ALBERS_CONIC_EQUAL_AREA		3
#define EPRJ_LAMBERT_CONFORMAL_CONIC	        4
#define EPRJ_MERCATOR                           5
#define EPRJ_POLAR_STEREOGRAPHIC                6
#define EPRJ_POLYCONIC                          7
#define EPRJ_EQUIDISTANT_CONIC                  8
#define EPRJ_TRANSVERSE_MERCATOR                9
#define EPRJ_STEREOGRAPHIC                      10
#define EPRJ_LAMBERT_AZIMUTHAL_EQUAL_AREA       11
#define EPRJ_AZIMUTHAL_EQUIDISTANT              12
#define EPRJ_GNOMONIC                           13
#define EPRJ_ORTHOGRAPHIC                       14
#define EPRJ_GENERAL_VERTICAL_NEAR_SIDE_PERSPECTIVE 15
#define EPRJ_SINUSOIDAL                         16
#define EPRJ_EQUIRECTANGULAR                    17
#define EPRJ_MILLER_CYLINDRICAL                 18
#define EPRJ_VANDERGRINTEN                      19
#define EPRJ_HOTINE_OBLIQUE_MERCATOR            20
#define EPRJ_SPACE_OBLIQUE_MERCATOR             21
#define EPRJ_MODIFIED_TRANSVERSE_MERCATOR       22
#define EPRJ_EOSAT_SOM                          23
#define EPRJ_ROBINSON                           24
#define EPRJ_SOM_A_AND_B                        25
#define EPRJ_ALASKA_CONFORMAL                   26
#define EPRJ_INTERRUPTED_GOODE_HOMOLOSINE       27
#define EPRJ_MOLLWEIDE                          28
#define EPRJ_INTERRUPTED_MOLLWEIDE              29
#define EPRJ_HAMMER                             30
#define EPRJ_WAGNER_IV                          31
#define EPRJ_WAGNER_VII                         32
#define EPRJ_OBLATED_EQUAL_AREA                 33
#define EPRJ_PLATE_CARREE                       34
#define EPRJ_EQUIDISTANT_CYLINDRICAL            35
#define EPRJ_GAUSS_KRUGER                       36
#define EPRJ_ECKERT_VI                          37
#define EPRJ_ECKERT_V                           38
#define EPRJ_ECKERT_IV                          39
#define EPRJ_ECKERT_III                         40
#define EPRJ_ECKERT_II                          41
#define EPRJ_ECKERT_I                           42
#define EPRJ_GALL_STEREOGRAPHIC                 43
#define EPRJ_BEHRMANN                           44
#define EPRJ_WINKEL_I                           45
#define EPRJ_WINKEL_II                          46
#define EPRJ_QUARTIC_AUTHALIC                   47
#define EPRJ_LOXIMUTHAL                         48
#define EPRJ_BONNE                              49
#define EPRJ_STEREOGRAPHIC_EXTENDED             50
#define EPRJ_CASSINI                            51
#define EPRJ_TWO_POINT_EQUIDISTANT              52

#define EPRJ_EXTERNAL_RSO			"eprj_rso"
#define EPRJ_EXTERNAL_NZMG                      "nzmg"

CPL_C_END

#endif /* ndef _HFAOPEN_H_INCLUDED */
