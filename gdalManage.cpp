// 0. precompiled headers
// 1. related header
#include "gdalManage.h"
#include "dimension2d.h"
#include "base/rect2d.h"
#include "gdal/include/ogr_spatialref.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
*/
// 2. c System library
// 3. c++ System library
// 4. external library header
// 5. project header

/*
기능 : WKT 파일(.prj) 또는 WKT 코드를 Gdal을 통해 PROJ4로 변환.
Parameter
	- _str_wkt : .prj 파일 주소 또는 WKT 코드 데이터
return
	- PROJ4 코드 : 변환 성공
	- "" : 변환 실패
*/
//GDAL 사용함수 선언 및 전역변수
//static double          dfMinX = 0.0, dfMinY = 0.0, dfMaxX = 0.0, dfMaxY = 0.0;
//static double          dfXRes = 0.0, dfYRes = 0.0;
//static int             nForcePixels = 0, nForceLines = 0;

// 주석 임시
static GDALDatasetH
GDALWarpCreateOutput(GDALDatasetH hSrcDS, const char *pszFilename,
	const char *pszFormat, const char *pszSourceSRS,
	const char *pszTargetSRS, int nOrder,
	char **papszCreateOptions);

void VSIFree1(void * pData)

{
	if (pData != NULL)
		free(pData);
}


void GDALWarpAppOptionsForBinaryFree(GDALWarpAppOptionsForBinary* psOptionsForBinary)
{
	if (psOptionsForBinary)
	{
		CSLDestroy(psOptionsForBinary->papszSrcFiles);
		free(psOptionsForBinary->pszDstFilename);
		if(psOptionsForBinary->papszOpenOptions) CSLDestroy(psOptionsForBinary->papszOpenOptions);
		if(psOptionsForBinary->papszDestOpenOptions) CSLDestroy(psOptionsForBinary->papszDestOpenOptions);
		free(psOptionsForBinary);
	}
}

//GDAL 사용함수 정의
GDALWarpAppOptionsForBinary *GDALWarpAppOptionsForBinaryNew(void)
{
	return (GDALWarpAppOptionsForBinary*)CPLCalloc(1, sizeof(GDALWarpAppOptionsForBinary));
}




//call API 정의
std::string GDALTransFormWKTToProj(std::string _str_wkt) {
	int nEntries = 0;
	std::string str_result;
	OGRSpatialReference  oSRS;

	str_result.clear();

	if (GDALFindSRS(_str_wkt.c_str(), oSRS) != true) {
		return str_result;
	}

	for (int i = 0; i < (nEntries ? nEntries : 1); i++) {
		char *pszOutput = nullptr;
		oSRS.exportToProj4(&pszOutput);
		str_result = pszOutput;
	}

	GDALDestroyDriverManager();
	OGRCleanupAll();
	return str_result;
}
/*
기능 : Gdal 라이브러리에서 WKT 파일 또는 코드를 통해 해당 PROJ4 존재유무 확인.
Parameter
	- pszInput : .prj 파일 주소 또는 WKT 코드 데이터
	- oSRS : 해당 좌표정보를 반환.
return
	- true : WKT에 해당되는 PROJ4 코드 존재
	- false : WKT에 해당되는 PROJ4 코드 미존재
*/
bool GDALFindSRS(const char *pszInput, OGRSpatialReference &oSRS) {
	bool bGotSRS = false;
	GDALDataset *poGDALDS = nullptr;
	OGRLayer *poLayer = nullptr;
	const char *pszProjection = nullptr;
	CPLErrorHandler oErrorHandler = nullptr;
	bool bIsFile = false;
	OGRErr eErr = OGRERR_NONE;

	bool bDebug = CPLTestBool(CPLGetConfigOption("CPL_DEBUG", "OFF"));
	if (!bDebug)	oErrorHandler = CPLSetErrorHandler(CPLQuietErrorHandler);

	VSILFILE *fp = VSIFOpenL(pszInput, "r");
	if (fp) {
		bIsFile = true;
		VSIFCloseL(fp);
	}

	if (!STARTS_WITH(pszInput, "http://spatialreference.org/")) {
		//_tprintf(_T("trying to open with GDAL : %s\n"), pszInput);
		poGDALDS = static_cast<GDALDataset *>(GDALOpenEx(pszInput, 0, nullptr, nullptr, nullptr));
	}

	if (poGDALDS != nullptr) {
		//pszProjection = poGDALDS->GetProjectionRef();
		pszProjection = GDALGetProjectionRef((GDALDatasetH)poGDALDS);
		if (pszProjection != nullptr && pszProjection[0] != '\0')
		{
			//if (oSRS.importFromWkt(pszProjection) == OGRERR_NONE) {
				//_tprintf(_T("got SRS from GDAL\n"));
				//bGotSRS = true;
			//}
		}
		else if (poGDALDS->GetLayerCount() > 0)
		{
			poLayer = poGDALDS->GetLayer(0);
			if (poLayer != nullptr) {
				OGRSpatialReference *poSRS = poLayer->GetSpatialRef();
				if (poSRS != nullptr) {
					//_tprintf(_T("got SRS from OGR\n"));
					bGotSRS = true;
					OGRSpatialReference* poSRSClone = poSRS->Clone();
					oSRS = *poSRSClone;
					OGRSpatialReference::DestroySpatialReference(poSRSClone);
				}
			}
		}
		GDALClose(poGDALDS);
		if (!bGotSRS) {
			//_tprintf(_T("did not open with GDAL\n"));
		}
	}

	if (!bGotSRS && bIsFile && (strstr(pszInput, ".prj") != nullptr)) {
		char **pszTemp;
		if (strstr(pszInput, "ESRI::") != nullptr)
			pszTemp = CSLLoad(pszInput + 6);
		else
			pszTemp = CSLLoad(pszInput);

		if (pszTemp) {
			eErr = oSRS.importFromESRI(pszTemp);
			CSLDestroy(pszTemp);
		}
		else
			eErr = OGRERR_UNSUPPORTED_SRS;

		if (eErr == OGRERR_NONE)	bGotSRS = true;
	}

	if (!bGotSRS) {
		if (CPLGetConfigOption("CPL_ALLOW_VSISTDIN", nullptr) == nullptr)
			CPLSetConfigOption("CPL_ALLOW_VSISTDIN", "YES");

		eErr = oSRS.SetFromUserInput(pszInput);
		if (eErr == OGRERR_NONE) {
			bGotSRS = true;
		}
	}

	if (!bDebug) {
		CPLSetErrorHandler(oErrorHandler);
	}
	return bGotSRS;
}
void GDALStart()
{
	//테스트 할 때
	//CPLSetConfigOption("GDAL_DATA","/home/hadoop/project/jsyun/kafka/XDConvertImage/data/");
	//Docker 배포시
	CPLSetConfigOption("GDAL_DATA","/usr/local/bin");
	//"/home/hadoop/hadoop/lib/native/"
	/* -------------------------------------------------------------------- */
	/*      Register standard GDAL drivers, and process generic GDAL        */
	/*      command options.                                                */
	/* -------------------------------------------------------------------- */
	GDALAllRegister();
printf("GDALStart - GDALAllRegister\n");

	if (CPLGetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", NULL) == NULL)
	{
#if defined(__MACH__) && defined(__APPLE__)
		// On Mach, the default limit is 256 files per process
		// We should eventually dynamically query the limit
		CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "100");
#else
		CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "450");
#endif
	}

}
void GDALEnd()
{
	GDALDestroyDriverManager();

	OGRCleanupAll();

}
struct GDALWarpAppOptions
{
	/*! set georeferenced extents of output file to be created (in target SRS by default,
		or in the SRS specified with pszTE_SRS) */
	double dfMinX;
	double dfMinY;
	double dfMaxX;
	double dfMaxY;

	/*! the SRS in which to interpret the coordinates given in GDALWarpAppOptions::dfMinX,
		GDALWarpAppOptions::dfMinY, GDALWarpAppOptions::dfMaxX and GDALWarpAppOptions::dfMaxY.
		The SRS may be any of the usual GDAL/OGR forms,
		complete WKT, PROJ.4, EPSG:n or a file containing the WKT. It is a
		convenience e.g. when knowing the output coordinates in a
		geodetic long/lat SRS, but still wanting a result in a projected
		coordinate system. */
	char *pszTE_SRS;

	/*! set output file resolution (in target georeferenced units) */
	double dfXRes;
	double dfYRes;

	/*! align the coordinates of the extent of the output file to the values of the
		GDALWarpAppOptions::dfXRes and GDALWarpAppOptions::dfYRes, such that the
		aligned extent includes the minimum extent. */
	bool bTargetAlignedPixels;

	/*! set output file size in pixels and lines. If GDALWarpAppOptions::nForcePixels
		or GDALWarpAppOptions::nForceLines is set to 0, the other dimension will be
		guessed from the computed resolution. Note that GDALWarpAppOptions::nForcePixels and
		GDALWarpAppOptions::nForceLines cannot be used with GDALWarpAppOptions::dfXRes and
		GDALWarpAppOptions::dfYRes. */
	int nForcePixels;
	int nForceLines;

	/*! allow or suppress progress monitor and other non-error output */
	bool bQuiet;

	/*! the progress function to use */
	GDALProgressFunc pfnProgress;

	/*! pointer to the progress data variable */
	void *pProgressData;

	/*! creates an output alpha band to identify nodata (unset/transparent) pixels
		when set to true */
	bool bEnableDstAlpha;

	/*! forces the last band of an input file to be considered as alpha band. */
	bool bEnableSrcAlpha;

	/*! Prevent a source alpha band from being considered as such */
	bool bDisableSrcAlpha;

	/*! output format. Use the short format name. */
	char *pszFormat;

	bool bCreateOutput;

	/*! list of warp options. ("NAME1=VALUE1","NAME2=VALUE2",...). The
		GDALWarpOptions::papszWarpOptions docs show all options. */
	char **papszWarpOptions;

	double dfErrorThreshold;

	/*! the amount of memory (in megabytes) that the warp API is allowed
		to use for caching. */
	double dfWarpMemoryLimit;

	/*! list of create options for the output format driver. See format
		specific documentation for legal creation options for each format. */
	char **papszCreateOptions;

	/*! the data type of the output bands */
	GDALDataType eOutputType;

	/*! working pixel data type. The data type of pixels in the source
		image and destination image buffers. */
	GDALDataType eWorkingType;

	/*! the resampling method. Available methods are: near, bilinear,
		cubic, cubicspline, lanczos, average, mode, max, min, med,
		q1, q3 */
	GDALResampleAlg eResampleAlg;

	/*! nodata masking values for input bands (different values can be supplied
		for each band). ("value1 value2 ..."). Masked values will not be used
		in interpolation. Use a value of "None" to ignore intrinsic nodata
		settings on the source dataset. */
	char *pszSrcNodata;

	/*! nodata values for output bands (different values can be supplied for
		each band). ("value1 value2 ..."). New files will be initialized to
		this value and if possible the nodata value will be recorded in the
		output file. Use a value of "None" to ensure that nodata is not defined.
		If this argument is not used then nodata values will be copied from
		the source dataset. */
	char *pszDstNodata;

	/*! use multithreaded warping implementation. Multiple threads will be used
		to process chunks of image and perform input/output operation simultaneously. */
	bool bMulti;

	/*! list of transformer options suitable to pass to GDALCreateGenImgProjTransformer2().
		("NAME1=VALUE1","NAME2=VALUE2",...) */
	char **papszTO;

	/*! enable use of a blend cutline from the name OGR support pszCutlineDSName */
	char *pszCutlineDSName;

	/*! the named layer to be selected from the cutline datasource */
	char *pszCLayer;

	/*! restrict desired cutline features based on attribute query */
	char *pszCWHERE;

	/*! SQL query to select the cutline features instead of from a layer
		with pszCLayer */
	char *pszCSQL;

	/*! crop the extent of the target dataset to the extent of the cutline */
	bool bCropToCutline;

	/*! copy dataset and band metadata will be copied from the first source dataset. Items that differ between
		source datasets will be set "*" (see GDALWarpAppOptions::pszMDConflictValue) */
	bool bCopyMetadata;

	/*! copy band information from the first source dataset */
	bool bCopyBandInfo;

	/*! value to set metadata items that conflict between source datasets (default is "*").
		Use "" to remove conflicting items. */
	char *pszMDConflictValue;

	/*! set the color interpretation of the bands of the target dataset from the source dataset */
	bool bSetColorInterpretation;

	/*! overview level of source files to be used */
	int nOvLevel;

	/*! Whether to disable vertical grid shift adjustment */
	bool bNoVShiftGrid;
};
float* GDALTransFormDEM(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue)
{
	GDALDatasetH *pahSrcDS = NULL;
	int nSrcCount = 0;
	//const char         *pszFormat = "BMP";//BMP,GTiff
	const char         *pszFormat = "MEM";
	char               *pszTargetSRS = NULL;
	char               *pszSourceSRS = NULL;
	const char         *pszSrcFilename = NULL, *pszDstFilename = NULL;
	int                 bCreateOutput = FALSE, nOrder = 0;
	void               *hTransformArg, *hGenImgProjArg = NULL, *hApproxArg = NULL;
	char               **papszWarpOptions = NULL;
	double             dfErrorThreshold = 0.125;
	GDALTransformerFunc pfnTransformer = NULL;
	char                **papszCreateOptions = NULL;
	int i, j, k;
	// 출력을 지원하는 영상 포맷 //
	/*
	Output driver `PNG' not recognised or does not support
		direct output file creation.The following format drivers are configured
		and support direct output :
	VRT: Virtual Raster
	GTiff : GeoTIFF
	NITF : National Imagery Transmission Format
	HFA : Erdas Imagine Images(.img)
	ELAS : ELAS
	MEM : In Memory Raster
	BMP : MS Windows Device Independent Bitmap
	PCIDSK : PCIDSK Database File
	PCRaster : PCRaster Raster File
	ILWIS : ILWIS Raster Map
	SGI : SGI Image File Format 1.0
	Leveller : Leveller heightfield
	Terragen : Terragen heightfield
	ISIS3 : USGS Astrogeology ISIS cube(Version 3)
	ISIS2 : USGS Astrogeology ISIS cube(Version 2)
	PDS4 : NASA Planetary Data System 4
	ERS : ERMapper.ers Labelled
	RMF : Raster Matrix Format
	RST : Idrisi Raster A.1
	INGR : Intergraph Raster
	GSBG : Golden Software Binary Grid(.grd)
	GS7BG : Golden Software 7 Binary Grid(.grd)
	PDF : Geospatial PDF
	MRF : Meta Raster Format
	PNM : Portable Pixmap Format(netpbm)
	PAux : PCI.aux Labelled
	MFF : Vexcel MFF Raster
	MFF2 : Vexcel MFF2(HKV) Raster
	BT : VTP.bt(Binary Terrain) 1.3 Format
	LAN : Erdas.LAN / .GIS
	IDA : Image Data and Analysis
	GTX : NOAA Vertical Datum.GTX
	NTv2 : NTv2 Datum Grid Shift
	CTable2 : CTable2 Datum Grid Shift
	KRO : KOLOR Raw
	ROI_PAC : ROI_PAC raster
	RRASTER : R Raster
	NWT_GRD : Northwood Numeric Grid Format.grd / .tab
	ADRG : ARC Digitized Raster Graphics
	SAGA : SAGA GIS Binary Grid(.sdat, .sg - grd - z)
	DB2ODBC : IBM DB2 Spatial Database
	ENVI : ENVI.hdr Labelled
	EHdr : ESRI.hdr Labelled
	ISCE : ISCE raster
	*/

	// 아래의 인자 갯수가 변경되면 apps 폴더 아래의 gdal_warp_lib.cpp 안의 GDALWarpAppOptionsNew 에서 인자 갯수 변경해야함.
	// 마지막 인자 인덱스 보나 1 크게 입력
	const int argc = 20;// 18;// 20;//14;//9
	imageWidth = 0;
	char** argv = 0;
	argv = new char*[argc];
	for (i = 0; i < argc; i++) {
		argv[i] = new char[256];
		memset(argv[i], 0, 256);
	}
	/*for (i = 0; i < argc; i++) {
		argv[i] = (char*)malloc(256);
		memset(argv[i], 0, 256);
	}*/
	/*
	//argv = (char**)malloc(sizeof(char)*argc);
	argv = (char**)CPLMalloc(17);

	for (i = 0; i < argc; i++) {
		argv[i] = (char*)CPLMalloc(256);
		memset(argv[i], 0, 256);
	}
	*/
	//sprintf(argv[0], "gdalwarpsimple");
	sprintf(argv[0], "-s_srs");
	sprintf(argv[1], "%s", srcCoord);
	//sprintf(argv[2],"'+proj=tmerc +lat_0=38 +lon_0=127 +k=1.000000 +x_0=200000 +y_0=600000 +ellps=GRS80 +datum=WGS84 +units=m +no_defs'");
	//sprintf(argv[2],"EPSG:4326");
	sprintf(argv[2], "-r");
	sprintf(argv[3], "bilinear");//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3
	sprintf(argv[4], "-t_srs");
	sprintf(argv[5], "%s", destCoord);//"EPSG:4326");
	sprintf(argv[6], "-te");
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);
	sprintf(argv[11], "-of");
	//sprintf(argv[13], "BMP");//GTiff,BMP
	sprintf(argv[12], "%s", "MEM");//GTiff,BMP

	sprintf(argv[13], "-srcnodata");
	//sprintf(argv[15], "0 0 0 0"); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[15],"255 255 255 255"); //rgb 일때, 흰색이 투명색
	sprintf(argv[14], "%f", noValue);   //지형 높이값일 때

/*	sprintf(argv[15], "-ts");
	sprintf(argv[16], "65"); //dest width pixel
	sprintf(argv[17], "65"); //dest height

	sprintf(argv[18], src);
	sprintf(argv[19], "");
*/
	sprintf(argv[15], "-ts");
	sprintf(argv[16], "65"); //dest width pixel
	sprintf(argv[17], "65"); //dest height 

	sprintf(argv[18], src);
	sprintf(argv[19], "");
	/*
	int len = strlen("-s_srs") + 1;
	argv[0]=new char[len];
	memset(argv[0], 0, len);
	memcpy(argv[0], "-s_srs", len-1);
	len = strlen(srcCoord) + 1;
	argv[1] = new char[len];
	memset(argv[1], 0, len);
	memcpy(argv[1], srcCoord, len-1);


	len = strlen("-r") + 1;
	argv[2] = new char[len];
	memset(argv[2], 0, len);
	memcpy(argv[2], "-r", len-1);
	len = strlen("bilinear") + 1;
	argv[3] = new char[len];
	memset(argv[3], 0, len);
	memcpy(argv[3], "bilinear", len-1);//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3


	len = strlen("-t_srs") + 1;
	argv[4] = new char[len];
	memset(argv[4], 0, len);
	memcpy(argv[4], "-t_srs", len - 1);
	len = strlen(destCoord) + 1;
	argv[5] = new char[len];
	memset(argv[5], 0, len);
	memcpy(argv[5], destCoord, len - 1); //"EPSG:4326");


	len = strlen("-te") + 1;
	argv[6] = new char[len];
	memset(argv[6], 0, len);
	memcpy(argv[6], "-te", len - 1);

	argv[7] = new char[32];
	argv[8] = new char[32];
	argv[9] = new char[32];
	argv[10] = new char[32];
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);

	len = strlen("-of") + 1;
	argv[11] = new char[len];
	memset(argv[11], 0, len);
	memcpy(argv[11], "-of", len - 1);
	len = strlen(fmt) + 1;
	argv[12] = new char[len];
	memset(argv[12], 0, len);
	memcpy(argv[12], fmt, len - 1); //GTiff,BMP


	len = strlen("-srcnodata") + 1;
	argv[13] = new char[len];
	memset(argv[13], 0, len);
	memcpy(argv[13], "-srcnodata", len - 1);
	//sprintf(argv[15], "0 0 0 0"); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[15],"255 255 255 255"); //rgb 일때, 흰색이 투명색
	len = strlen("-999") + 1;
	argv[14] = new char[len];
	memset(argv[14], 0, len);
	memcpy(argv[14], "-999", len - 1);

	len = strlen(src) + 1;
	argv[15] = new char[len];
	memset(argv[15], 0, len);
	memcpy(argv[15], src, len - 1);

	len = strlen(dest) + 1;
	argv[16] = new char[len];
	memset(argv[16], 0, len);
	memcpy(argv[16], dest, len - 1);
	*/
	//EarlySetConfigOptions(argc, argv);
	//int argcc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	//int argcd = CSLCount(argv);
	//EarlySetConfigOptions(argc, argv);
	//jsyun 20170721 It's path to all data , csv,....
	//CPLSetConfigOption("GDAL_DATA", "/home/hadoop/hadoop/lib/native/");




	GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	// 명령어 다음 부터 인자만 넘겨줌 그래서 + 1, 인자 갯수도 정확히 넘겨줌
	GDALWarpAppOptions *psOptions = NULL;//GDALWarpAppOptionsNew(argv, psOptionsForBinary, argc);



	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */

	for (int i = 0; psOptionsForBinary->papszSrcFiles[i] != NULL; i++)
	{
		nSrcCount++;
		pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH) * nSrcCount);
		pahSrcDS[nSrcCount - 1] = GDALOpenEx(psOptionsForBinary->papszSrcFiles[i], GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL,
			(const char* const*)psOptionsForBinary->papszOpenOptions, NULL);

		if (pahSrcDS[nSrcCount - 1] == NULL)
			return NULL;
	}
	GDALDatasetH hDstDS = NULL;
	/* -------------------------------------------------------------------- */
	/*      Does the output dataset already exist?                          */
	/* -------------------------------------------------------------------- */

		/* FIXME ? source filename=target filename and -overwrite is definitely */
		/* an error. But I can't imagine of a valid case (without -overwrite), */
		/* where it would make sense. In doubt, let's keep that dubious possibility... */


	if (!(psOptionsForBinary->bQuiet))
	{
		//0..10..이거 보이게 설정
		//GDALWarpAppOptionsSetProgress(psOptions, GDALTermProgress, NULL);
	}

	//	if (hDstDS == NULL && !psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet)
	//		CheckExtensionConsistency(psOptionsForBinary->pszDstFilename, psOptionsForBinary->pszFormat);
	int bUsageError = FALSE;
	GDALDatasetH hOutDS = GDALWarp("", hDstDS, //psOptionsForBinary->pszDstFilename
		nSrcCount, pahSrcDS, psOptions, &bUsageError);
	int nRetCode = (hOutDS) ? 0 : 1;
	float *data = NULL;
	GDALDataset *pDS = NULL;
	if (hOutDS) {
		pDS = (GDALDataset *)hOutDS;
		GDALRasterBand  *poBand = NULL;
		dimension2di imagesize;
		rect2dd	rect;
		double	resolution;
		imagesize.Width = pDS->GetRasterXSize();
		imagesize.Height = pDS->GetRasterYSize();
		imageWidth = imagesize.Width;
		//resolution = adfGeoTransform[1]; //x 해상도, adfGeoTransform[5];y 해상도
		int band = pDS->GetRasterCount();
		double nodata;
		int dataBits = 0;
		for (i = 0; i < band; i++) {
			poBand = pDS->GetRasterBand(i + 1);
			dataBits += GDALGetDataTypeSize(poBand->GetRasterDataType());;
			nodata = GDALGetRasterNoDataValue(poBand, NULL);
			noValue = (float)nodata;
		}
		if (band != 1 || (dataBits != 32 && dataBits != 16)) return NULL; //float 지형만 처리한다.
		//if (band != 1 || dataBits != 32) return false; //float 지형만 처리한다.
		void** pData = new void*[band];
		//for (i = 0; i < band; i++) pData[i] = VSIMalloc(imagesize.Width);//밴드별 메모리 할당
		for (i = 0; i < band; i++) {
			if (dataBits == 32) {
				if (poBand->GetRasterDataType() == GDT_UInt32)
					pData[i] = new unsigned int[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
				else if (poBand->GetRasterDataType() == GDT_Int32)
					pData[i] = new int[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
				else if (poBand->GetRasterDataType() == GDT_Float32)
					pData[i] = new float[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
			}
			else {
				if (poBand->GetRasterDataType() == GDT_UInt16)
					pData[i] = new unsigned short[imagesize.Width];//밴드별 메모리 할당 , GDT_UInt16
				else
					pData[i] = new short[imagesize.Width];//밴드별 메모리 할당 , GDT_Int16
			}

		}
		int curidx = 0;
		//i,j 는 left-top 픽셀 위치이고 그 위치에 해당되는 값은 실제 세계 좌하단
		double xx, yy;
		GDALRasterIOExtraArg extraArg;
		extraArg.eResampleAlg = GRIORA_Cubic;
		data = new float[imagesize.Width*imagesize.Height];
		for (j = 0; j < imagesize.Height; j++) {
			for (k = 0; k < band; k++) {//데이터 로딩...
				poBand = pDS->GetRasterBand(k + 1);
				int aa = poBand->GetRasterDataType();
				////if (dataBits == 32)
				//poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, poBand->GetRasterDataType(), 0, 0);// , &extraArg);
			GDALRasterIO((GDALRasterBandH)poBand,GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, poBand->GetRasterDataType(), 0, 0);// , &extraArg);
			////else 
				////poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, GDT_UInt16, 0, 0);// , &extraArg);
			}

			for (i = 0; i < imagesize.Width; i++) {

				//curidx = (imagesize.Height - j - 1)*imagesize.Width + i; // 거꾸로 저장됨
				curidx = (j)*imagesize.Width + i; // DEM은 이미지와 반대 방향이네? 여의도만 그런가?
				if (dataBits == 32) {
					if (poBand->GetRasterDataType() == GDT_UInt32) { //GDT_UInt32 = 4
						unsigned int *block4 = (unsigned int*)pData[0];
						data[curidx] = (float)(block4[i]);
					}
					else if (poBand->GetRasterDataType() == GDT_Int32) { // GDT_Int32 = 5
						int *block4 = (int*)pData[0];
						data[curidx] = float(block4[i]);
					}
					else if (poBand->GetRasterDataType() == GDT_Float32) { //GDT_Float32 = 6
						float *block4 = (float*)pData[0];
						data[curidx] = float(block4[i]);
					}
				}
				else {
					if (poBand->GetRasterDataType() == GDT_UInt16) {
						unsigned short *block2 = (unsigned short*)pData[0]; // GDT_UInt16
						data[curidx] = float(block2[i]);//데이터에 칼라값 저장
					}
					else {
						short *block2 = (short*)pData[0]; // GDT_Int16
						data[curidx] = float(block2[i]);//데이터에 칼라값 저장
					}
				}
				//if (data[curidx] == -32767.0 || data[curidx] == -32768.0 || data[curidx] == -999.0 || data[curidx] == nodata) data[curidx] = 0.0;

			}
		}
		for (i = 0; i < band; i++) delete pData[i];//메모리 해제..

		delete[] pData;
	}
	GDALWarpAppOptionsFree(psOptions);
	//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	GDALClose(hOutDS ? hOutDS : hDstDS);
	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);

	//GDALDumpOpenDatasets(stderr);
	for (i = 0; i < argc; i++) {
		if (argv[i]) delete[] argv[i];
	}
	delete argv;
	return data;
}
GDALDatasetH *GDALGetTransFormHandleDEM(char* src)
{
	GDALDatasetH *pahSrcDS = NULL;

	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */
	pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH));
	*pahSrcDS = GDALOpenEx(src, GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL, NULL, NULL);
	return pahSrcDS;
}
rect2dd GDALGetImageRect(char *src)
{
	rect2dd	rect = rect2dd(-1,-1,-1,-1);
	GDALDataset  *poDataset = NULL;
	int i, j, k;
	poDataset = (GDALDataset *)GDALOpen((char*)src, GA_ReadOnly);
	double        adfGeoTransform[6];
	if (poDataset == NULL)
	{
		GDALClose((GDALDatasetH)poDataset);
		return rect;
	}
	poDataset->GetGeoTransform(adfGeoTransform);
	dimension2di imagesize;
	double	resolution;
	imagesize.Width = poDataset->GetRasterXSize();
	imagesize.Height = poDataset->GetRasterYSize();
	//printf("w %d,h %d\n",imagesize.Width,imagesize.Height);
	rect.ix = adfGeoTransform[0];
	rect.iy = adfGeoTransform[3] + poDataset->GetRasterYSize() * adfGeoTransform[5];
	rect.ax = adfGeoTransform[0] + poDataset->GetRasterXSize() * adfGeoTransform[1];
	rect.ay = adfGeoTransform[3];
	//printf("%f,%f ~ %f,%f",rect.ix,rect.iy,rect.ax,rect.ay);
	GDALClose((GDALDatasetH)poDataset);
	return rect;
}
GDALWarpAppOptions *GDALSetTransFormOptionDEM(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue)
{
	GDALDatasetH *pahSrcDS = NULL;
	int nSrcCount = 0;
	//const char         *pszFormat = "BMP";//BMP,GTiff
	const char         *pszFormat = "MEM";
	char               *pszTargetSRS = NULL;
	char               *pszSourceSRS = NULL;
	const char         *pszSrcFilename = NULL, *pszDstFilename = NULL;
	int                 bCreateOutput = FALSE, nOrder = 0;
	void               *hTransformArg, *hGenImgProjArg = NULL, *hApproxArg = NULL;
	char               **papszWarpOptions = NULL;
	double             dfErrorThreshold = 0.125;
	GDALTransformerFunc pfnTransformer = NULL;
	char                **papszCreateOptions = NULL;
	int i, j, k;
	// 출력을 지원하는 영상 포맷 //
	// 아래의 인자 갯수가 변경되면 apps 폴더 아래의 gdal_warp_lib.cpp 안의 GDALWarpAppOptionsNew 에서 인자 갯수 변경해야함.
	// 마지막 인자 인덱스 보나 1 크게 입력
	const int argc = 19;// 18;// 20;//14;//9
	char** argv = 0;
	argv = new char*[argc];
	for (i = 0; i < argc; i++) {
		argv[i] = new char[256];
		memset(argv[i], 0, 256);
	}
	
	/*for (i = 0; i < argc; i++) {
		argv[i] = (char*)malloc(256);
		memset(argv[i], 0, 256);
	}*/
	/*
	//argv = (char**)malloc(sizeof(char)*argc);
	argv = (char**)CPLMalloc(17);

	for (i = 0; i < argc; i++) {
		argv[i] = (char*)CPLMalloc(256);
		memset(argv[i], 0, 256);
	}
	*/

	//sprintf(argv[0], "gdalwarpsimple");
	sprintf(argv[0], "%s", "-s_srs");
	sprintf(argv[1], "%s", srcCoord);
	//sprintf(argv[2],"'+proj=tmerc +lat_0=38 +lon_0=127 +k=1.000000 +x_0=200000 +y_0=600000 +ellps=GRS80 +datum=WGS84 +units=m +no_defs'");
	//sprintf(argv[2],"EPSG:4326");
	sprintf(argv[2], "%s","-r");
	sprintf(argv[3], "%s", "bilinear");//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3
	sprintf(argv[4], "%s", "-t_srs");
	sprintf(argv[5], "%s", destCoord);//"EPSG:4326");
	sprintf(argv[6], "%s", "-te");
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);
	sprintf(argv[11], "%s", "-of");
	//sprintf(argv[13], "BMP");//GTiff,BMP
	sprintf(argv[12], "%s", "MEM");//GTiff,BMP

	sprintf(argv[13], "%s", "-srcnodata");
	//sprintf(argv[15], "0 0 0 0"); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[15],"255 255 255 255"); //rgb 일때, 흰색이 투명색
	sprintf(argv[14], "%f", noValue);   //지형 높이값일 때

	sprintf(argv[15], "%s", "-ts");
	sprintf(argv[16], "%s", "65"); //dest width pixel
	sprintf(argv[17], "%s", "65"); //dest height 

	sprintf(argv[18], "%s", src); 
	//sprintf(argv[19], "%s", "");

	//for (i = 0; i < argc; i++) {
	//	printf("[%d/%d] = %s\n",i,argc,argv[i]);
	//}

	/*
	int len = strlen("-s_srs") + 1;
	argv[0]=new char[len];
	memset(argv[0], 0, len);
	memcpy(argv[0], "-s_srs", len-1);
	len = strlen(srcCoord) + 1;
	argv[1] = new char[len];
	memset(argv[1], 0, len);
	memcpy(argv[1], srcCoord, len-1);


	len = strlen("-r") + 1;
	argv[2] = new char[len];
	memset(argv[2], 0, len);
	memcpy(argv[2], "-r", len-1);
	len = strlen("bilinear") + 1;
	argv[3] = new char[len];
	memset(argv[3], 0, len);
	memcpy(argv[3], "bilinear", len-1);//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3


	len = strlen("-t_srs") + 1;
	argv[4] = new char[len];
	memset(argv[4], 0, len);
	memcpy(argv[4], "-t_srs", len - 1);
	len = strlen(destCoord) + 1;
	argv[5] = new char[len];
	memset(argv[5], 0, len);
	memcpy(argv[5], destCoord, len - 1); //"EPSG:4326");


	len = strlen("-te") + 1;
	argv[6] = new char[len];
	memset(argv[6], 0, len);
	memcpy(argv[6], "-te", len - 1);

	argv[7] = new char[32];
	argv[8] = new char[32];
	argv[9] = new char[32];
	argv[10] = new char[32];
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);

	len = strlen("-of") + 1;
	argv[11] = new char[len];
	memset(argv[11], 0, len);
	memcpy(argv[11], "-of", len - 1);
	len = strlen(fmt) + 1;
	argv[12] = new char[len];
	memset(argv[12], 0, len);
	memcpy(argv[12], fmt, len - 1); //GTiff,BMP


	len = strlen("-srcnodata") + 1;
	argv[13] = new char[len];
	memset(argv[13], 0, len);
	memcpy(argv[13], "-srcnodata", len - 1);
	//sprintf(argv[15], "0 0 0 0"); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[15],"255 255 255 255"); //rgb 일때, 흰색이 투명색
	len = strlen("-999") + 1;
	argv[14] = new char[len];
	memset(argv[14], 0, len);
	memcpy(argv[14], "-999", len - 1);

	len = strlen(src) + 1;
	argv[15] = new char[len];
	memset(argv[15], 0, len);
	memcpy(argv[15], src, len - 1);

	len = strlen(dest) + 1;
	argv[16] = new char[len];
	memset(argv[16], 0, len);
	memcpy(argv[16], dest, len - 1);
	*/




	//EarlySetConfigOptions(argc, argv);
	//int argcc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	//int argcd = CSLCount(argv);
	//EarlySetConfigOptions(argc, argv);
	//jsyun 20170721 It's path to all data , csv,....
	//CPLSetConfigOption("GDAL_DATA", "/home/hadoop/hadoop/lib/native/");

	//int argc2 = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
	//printf("argc2 %d\n",argc2);
	CPLSetConfigOption("GDAL_MAX_DATASET_POOL_SIZE", "450");
	//GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	// 명령어 다음 부터 인자만 넘겨줌 그래서 + 1, 인자 갯수도 정확히 넘겨줌
	//GDALWarpAppOptions *psOptions = GDALWarpAppOptionsNew(argv, psOptionsForBinary, argc);
	GDALWarpAppOptions *psOptions = GDALWarpAppOptionsNew2(argv);
	//CSLDestroy( argv );
	
	for (i = 0; i < argc; i++) {
		//printf("%d/%d - before delete\n",i,argc);
		if (argv[i]) {
			//printf("%d/%d - %s before 2 delete\n",i,argc,argv[i]);
			
			delete argv[i];
			//printf("%d/%d - after 2 delete\n",i,argc);
		}
		//printf("%d/%d - after delete\n",i,argc);
	}
	if(argv) {
		delete [] argv;
		argv = NULL; 
	}
	
	return psOptions;
}
float * GDALTransFormImageByHandleDEM(GDALDatasetH *pahSrcDS, int nSrcCount, GDALWarpAppOptions *psOptions, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, const char* fmt, int &imageWidth, float noValue)
{

	int i, j, k;
	GDALDatasetH hDstDS = NULL;


	//	if (hDstDS == NULL && !psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet)
	//		CheckExtensionConsistency(psOptionsForBinary->pszDstFilename, psOptionsForBinary->pszFormat);
	int bUsageError = FALSE;
	GDALDatasetH hOutDS = GDALWarp("", hDstDS, 
		nSrcCount, pahSrcDS, psOptions, &bUsageError);
	
	int nRetCode = (hOutDS) ? 0 : 1;
	float *data = NULL;
	//data = new float[65*65];
	//imageWidth = 65;
	//return data;
	GDALDataset *pDS = NULL;
	if (hOutDS) {
		pDS = (GDALDataset *)hOutDS;
		GDALRasterBand  *poBand = NULL;
		dimension2di imagesize;
		rect2dd	rect;
		double	resolution;

		imagesize.Width = pDS->GetRasterXSize();
		imagesize.Height = pDS->GetRasterYSize();
		//printf("2 : %d, %d\n",imagesize.Width,imagesize.Height);
		imageWidth = imagesize.Width;
		//resolution = adfGeoTransform[1]; //x 해상도, adfGeoTransform[5];y 해상도
		int band = pDS->GetRasterCount();
		//printf("3 band : %d\n",band);
		double nodata;
		int dataBits = 0;
		for (i = 0; i < band; i++) {
			poBand = pDS->GetRasterBand(i + 1);
			dataBits += GDALGetDataTypeSize(poBand->GetRasterDataType());;
			nodata = GDALGetRasterNoDataValue(poBand, NULL);
			noValue = (float)nodata;
		}
		if (band != 1 || (dataBits != 32 && dataBits != 16)) return NULL; //float shrot 지형만 처리한다.
		void** pData = new void*[band];
		//for (i = 0; i < band; i++) pData[i] = VSIMalloc(imagesize.Width);//밴드별 메모리 할당
		for (i = 0; i < band; i++) {
			if (dataBits == 32) {
				if (poBand->GetRasterDataType() == GDT_UInt32)
					pData[i] = new unsigned int[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
				else if (poBand->GetRasterDataType() == GDT_Int32)
					pData[i] = new int[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
				else if (poBand->GetRasterDataType() == GDT_Float32)
					pData[i] = new float[imagesize.Width];//밴드별 메모리 할당 , GDT_Float32 , GDT_UInt32,GDT_Int32 는 고려함. jsyun
			}
			else {
				if (poBand->GetRasterDataType() == GDT_UInt16)
					pData[i] = new unsigned short[imagesize.Width];//밴드별 메모리 할당 , GDT_UInt16
				else
					pData[i] = new short[imagesize.Width];//밴드별 메모리 할당 , GDT_Int16
			}

		} 
		int curidx = 0;
		//i,j 는 left-top 픽셀 위치이고 그 위치에 해당되는 값은 실제 세계 좌하단
		double xx, yy;
		GDALRasterIOExtraArg extraArg;
		extraArg.eResampleAlg = GRIORA_Cubic;
		data = new float[imagesize.Width*imagesize.Height];
		/*
		GDALWarpAppOptionsFree(psOptions);
		//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

		//GDALClose(hOutDS ? hOutDS : hDstDS);
		//GDALClose(hOutDS);
		CPLFree(pDS);
		CPLFree(hOutDS);
		//GDALDumpOpenDatasets(stderr);
		return data;*/
		for (j = 0; j < imagesize.Height; j++) {
			//void** pData = new void*[band];
			for (i = 0; i < band; i++) {//데이터 로딩...
				poBand = pDS->GetRasterBand(i + 1);
				int aa = poBand->GetRasterDataType();
				
				////if (dataBits == 32)
				//poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, poBand->GetRasterDataType(), 0, 0);// , &extraArg);
				GDALRasterIO((GDALRasterBandH)poBand,GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, poBand->GetRasterDataType(), 0, 0);// , &extraArg);
				
			////else 
				////poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, GDT_UInt16, 0, 0);// , &extraArg);
			}

			for (i = 0; i < imagesize.Width; i++) {

				//curidx = (imagesize.Height - j - 1)*imagesize.Width + i; // 거꾸로 저장됨
				curidx = (j)*imagesize.Width + i; // DEM은 이미지와 반대 방향이네? 여의도만 그런가?
				if (dataBits == 32) {
					if (poBand->GetRasterDataType() == GDT_UInt32) { //GDT_UInt32 = 4
						unsigned int *block4 = (unsigned int*)pData[0];
						data[curidx] = (float)(block4[i]);
					}
					else if (poBand->GetRasterDataType() == GDT_Int32) { // GDT_Int32 = 5
						int *block4 = (int*)pData[0];
						data[curidx] = float(block4[i]);
					}
					else if (poBand->GetRasterDataType() == GDT_Float32) { //GDT_Float32 = 6
						float *block4 = (float*)pData[0];
						data[curidx] = float(block4[i]);
					}
				}
				else {
					if (poBand->GetRasterDataType() == GDT_UInt16) {
						unsigned short *block2 = (unsigned short*)pData[0]; // GDT_UInt16
						data[curidx] = float(block2[i]);//데이터에 칼라값 저장
					}
					else {
						short *block2 = (short*)pData[0]; // GDT_Int16
						data[curidx] = float(block2[i]);//데이터에 칼라값 저장
					}
				}
				//if (data[curidx] == -32767.0 || data[curidx] == -32768.0 || data[curidx] == -999.0 || data[curidx] == nodata) data[curidx] = 0.0;

			}
		}
		for (i = 0; i < band; i++) delete pData[i];//메모리 해제..
		delete[] pData;
		//delete pDS;// = (GDALDataset *)hOutDS;
		//delete poBand ;
	}
	GDALClose(hOutDS ? hOutDS : hDstDS);
	return data;
}
void GDALFreeHandleDEM(GDALDatasetH *pahSrcDS, int  nSrcCount)
{
	//GDALWarpAppOptionsFree(psOptions);
//주석 해제하면 오류 뜬다. 메모리 해제 문제 있음 new --> delete , malloc -> free
//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);

	//GDALDumpOpenDatasets(stderr);
}
char *SanitizeSRS(const char *pszUserInput)

{
	OGRSpatialReferenceH hSRS;
	char *pszResult = nullptr;

	CPLErrorReset();
	hSRS = OSRNewSpatialReference(nullptr);
	if (OSRSetFromUserInput(hSRS, pszUserInput) == OGRERR_NONE)
		OSRExportToWkt(hSRS, &pszResult);
	else
	{
		CPLError(CE_Failure, CPLE_AppDefined,
			"Translating source or target SRS failed:\n%s",
			pszUserInput);
	}
	OSRDestroySpatialReference(hSRS);
	return pszResult;
}

char * GDALCheckTransparencyColor(char *src, char r, char g, char b, char r2, char g2, char b2)
{
	char * retNoData = new char[3];
	GDALDatasetH raster = NULL;
	raster = GDALOpen(src, GA_ReadOnly);
	GDALRasterBandH raster_bandR, raster_bandG, raster_bandB ;
	int XSize, YSize,i;
	int band = GDALGetRasterCount(raster);
	bool bNoData = true;
	char R, G, B;
	/*
	// 밴드마다 NoData 설정이 있으면 간단히
	for (i = 0; i < band; i++) {
		raster_bandR = GDALGetRasterBand(raster, i+1);
		double ret = GDALGetRasterNoDataValue(raster_bandR, NULL);
		if (ret < 0.0 && ret > 255.0) {
			bNoData = false;
			break;
		}
		if(i==0) R = (char)ret;
		else if(i==1) G = (char)ret;
		else B = (char)ret;
	}
	if (bNoData == true) {
		if (R == r && G == g && B == b) {
			retNoData[0] = r; retNoData[1] = g; retNoData[2] = b;
		}
		else if (R == r2 && G == g2 && B == b) {
			retNoData[0] = r2; retNoData[1] = g2; retNoData[2] = b2;
		}
		return retNoData;
	}*/
	//밴드마다 NoData 설정이 없으면 외각 픽셀 조사, 시간 걸림

	raster_bandR = GDALGetRasterBand(raster, 1);
	GDALGetBlockSize(raster_bandR, &XSize, &YSize);
	int nXSize = GDALGetRasterBandXSize(raster_bandR);
	int nYSize = GDALGetRasterBandYSize(raster_bandR);
	char *lineR = (char *)CPLMalloc(sizeof(char)*nXSize);
	char *lineG = (char *)CPLMalloc(sizeof(char)*nXSize);
	char *lineB = (char *)CPLMalloc(sizeof(char)*nXSize);
	int count1, count2; //count1 은 rgb 색상 조사, count2는 r2g2b2 색상 조사
	count1 = count2 = 0;
	raster_bandR = GDALGetRasterBand(raster, 1);
	raster_bandG = GDALGetRasterBand(raster, 2);
	raster_bandB = GDALGetRasterBand(raster, 3);
	int row, col;
	char value;
	for (row = 0; row < nYSize; row++) {
		GDALRasterIO(raster_bandR, GF_Read, 0, row, nXSize, 1, lineR, nXSize, 1, GDT_Byte, 0, 0);
		GDALRasterIO(raster_bandG, GF_Read, 0, row, nXSize, 1, lineG, nXSize, 1, GDT_Byte, 0, 0);
		GDALRasterIO(raster_bandB, GF_Read, 0, row, nXSize, 1, lineB, nXSize, 1, GDT_Byte, 0, 0);
		if (row == 0 || row == nYSize - 1) {//상단,하단 끝이면
			for (col = 0; col < nXSize; col++) {
				if (lineR[col] == r && lineG[col] == g && lineB[col] == b) count1++;
				else if (lineR[col] == r2 && lineG[col] == g2 && lineB[col] == b2) count2++;
				//printf("%d\n", value);
			}
		}
		else { 
			if (lineR[0] == r && lineG[0] == g && lineB[0] == b) count1++;
			else if (lineR[0] == r2 && lineG[0] == g2 && lineB[0] == b2) count2++;

			if (lineR[nXSize-1] == r && lineG[nXSize - 1] == g && lineB[nXSize - 1] == b) count1++;
			else if (lineR[nXSize - 1] == r2 && lineG[nXSize - 1] == g2 && lineB[nXSize - 1] == b2) count2++;
			
		}
	}
	CPLFree(lineR);
	CPLFree(lineG);
	CPLFree(lineB);
	GDALClose(raster);

	if (count1 > count2) {
		retNoData[0] = r; retNoData[1] = g; retNoData[2] = b;
	}
	else if (count1 < count2) {
		retNoData[0] = r2; retNoData[1] = g2; retNoData[2] = b2;
	}
	else {
		retNoData[0] = r; retNoData[1] = g; retNoData[2] = b; // 입력된 투명컬러값이 없는 경우 기본 입력된 컬러값 사용함.
	}
	return retNoData;

}
char * GDALTransFormImage(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize, int &imageWidth, int &imageHeight, int imgType, char r, char g, char b, char a,char *method)
{
	GDALDatasetH *pahSrcDS = NULL;
	char *data = NULL;
	int nSrcCount = 0;
	//const char         *pszFormat = "BMP";//BMP,GTiff
	const char         *pszFormat = "MEM";
	char               *pszTargetSRS = NULL;
	char               *pszSourceSRS = NULL;
	const char         *pszSrcFilename = NULL, *pszDstFilename = NULL;
	int                 bCreateOutput = FALSE, nOrder = 0;
	void               *hTransformArg, *hGenImgProjArg = NULL, *hApproxArg = NULL;
	char               **papszWarpOptions = NULL;
	double             dfErrorThreshold = 0.125;
	GDALTransformerFunc pfnTransformer = NULL;
	char                **papszCreateOptions = NULL;
	// 출력을 지원하는 영상 포맷 //
	/*
	Output driver `PNG' not recognised or does not support
		direct output file creation.The following format drivers are configured
		and support direct output :
	VRT: Virtual Raster
	GTiff : GeoTIFF
	NITF : National Imagery Transmission Format
	HFA : Erdas Imagine Images(.img)
	ELAS : ELAS
	MEM : In Memory Raster
	BMP : MS Windows Device Independent Bitmap
	PCIDSK : PCIDSK Database File
	PCRaster : PCRaster Raster File
	ILWIS : ILWIS Raster Map
	SGI : SGI Image File Format 1.0
	Leveller : Leveller heightfield
	Terragen : Terragen heightfield
	ISIS3 : USGS Astrogeology ISIS cube(Version 3)
	ISIS2 : USGS Astrogeology ISIS cube(Version 2)
	PDS4 : NASA Planetary Data System 4
	ERS : ERMapper.ers Labelled
	RMF : Raster Matrix Format
	RST : Idrisi Raster A.1
	INGR : Intergraph Raster
	GSBG : Golden Software Binary Grid(.grd)
	GS7BG : Golden Software 7 Binary Grid(.grd)
	PDF : Geospatial PDF
	MRF : Meta Raster Format
	PNM : Portable Pixmap Format(netpbm)
	PAux : PCI.aux Labelled
	MFF : Vexcel MFF Raster
	MFF2 : Vexcel MFF2(HKV) Raster
	BT : VTP.bt(Binary Terrain) 1.3 Format
	LAN : Erdas.LAN / .GIS
	IDA : Image Data and Analysis
	GTX : NOAA Vertical Datum.GTX
	NTv2 : NTv2 Datum Grid Shift
	CTable2 : CTable2 Datum Grid Shift
	KRO : KOLOR Raw
	ROI_PAC : ROI_PAC raster
	RRASTER : R Raster
	NWT_GRD : Northwood Numeric Grid Format.grd / .tab
	ADRG : ARC Digitized Raster Graphics
	SAGA : SAGA GIS Binary Grid(.sdat, .sg - grd - z)
	DB2ODBC : IBM DB2 Spatial Database
	ENVI : ENVI.hdr Labelled
	EHdr : ESRI.hdr Labelled
	ISCE : ISCE raster
	*/

	// 아래의 인자 갯수가 변경되면 apps 폴더 아래의 gdal_warp_lib.cpp 안의 GDALWarpAppOptionsNew 에서 인자 갯수 변경해야함.
	// 마지막 인자 인덱스 보나 1 크게 입력
	const int argc = 19;//14;//9
	int i, j, k;
	char** argv = 0;
	argv = new char*[argc];
	for (i = 0; i < argc; i++) {
		argv[i] = new char[256];
		memset(argv[i], 0, 256);
	}
	/*for (i = 0; i < argc; i++) {
		argv[i] = (char*)malloc(256);
		memset(argv[i], 0, 256);
	}*/
	/*
	//argv = (char**)malloc(sizeof(char)*argc);
	argv = (char**)CPLMalloc(17);

	for (i = 0; i < argc; i++) {
		argv[i] = (char*)CPLMalloc(256);
		memset(argv[i], 0, 256);
	}
	*/
	//sprintf(argv[0], "gdalwarpsimple");
	sprintf(argv[0], "-s_srs");
	sprintf(argv[1], "%s", "EPSG:4326");//srcCoord);
	//sprintf(argv[2],"'+proj=tmerc +lat_0=38 +lon_0=127 +k=1.000000 +x_0=200000 +y_0=600000 +ellps=GRS80 +datum=WGS84 +units=m +no_defs'");
	//sprintf(argv[2],"EPSG:4326");
	sprintf(argv[2], "-r");
	if(method==NULL)
		sprintf(argv[3], "bilinear");//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3
	else
		sprintf(argv[3], method);//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3
	sprintf(argv[4], "-t_srs");
	sprintf(argv[5], "%s", "EPSG:4326");//destCoord);//"EPSG:4326");
	sprintf(argv[6], "-te");
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);
	sprintf(argv[11], "-of");
	//sprintf(argv[13], "BMP");//GTiff,BMP
	sprintf(argv[12], "%s", "GTiff");//GTiff,BMP

	sprintf(argv[13], "-srcnodata");
	//sprintf(argv[14], "0 0 0 0"); //rgb 일때 , 검은색이 투명색 ==> 초기에 수동 설정
	//sprintf(argv[15],"255 255 255 0"); //rgb 일때, 흰색이 투명색 ==> 초기에 수동 설정
	if (imgType == 1)
		sprintf(argv[14], "\'%d %d %d %d\'", b, g, r, a); //jpg 일 경우 반전으로 거꾸로 입력해사 원하는 색상 적용됨 2023.7.26
	else
		sprintf(argv[14], "\'%d %d %d %d\'", r, g, b, a); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[14],"-999");   //지형 높이값일 때
	sprintf(argv[15], "-wm");
	sprintf(argv[16], "50"); //작업 메모리 크기
	//sprintf(argv[17], "256"); //dest height 

	sprintf(argv[17], src);
	sprintf(argv[18], "/home/hadoop/project/jsyun/kafka/XDConvertImage/lla.tif");

	/*
	int len = strlen("-s_srs") + 1;
	argv[0]=new char[len];
	memset(argv[0], 0, len);
	memcpy(argv[0], "-s_srs", len-1);
	len = strlen(srcCoord) + 1;
	argv[1] = new char[len];
	memset(argv[1], 0, len);
	memcpy(argv[1], srcCoord, len-1);


	len = strlen("-r") + 1;
	argv[2] = new char[len];
	memset(argv[2], 0, len);
	memcpy(argv[2], "-r", len-1);
	len = strlen("bilinear") + 1;
	argv[3] = new char[len];
	memset(argv[3], 0, len);
	memcpy(argv[3], "bilinear", len-1);//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3


	len = strlen("-t_srs") + 1;
	argv[4] = new char[len];
	memset(argv[4], 0, len);
	memcpy(argv[4], "-t_srs", len - 1);
	len = strlen(destCoord) + 1;
	argv[5] = new char[len];
	memset(argv[5], 0, len);
	memcpy(argv[5], destCoord, len - 1); //"EPSG:4326");


	len = strlen("-te") + 1;
	argv[6] = new char[len];
	memset(argv[6], 0, len);
	memcpy(argv[6], "-te", len - 1);

	argv[7] = new char[32];
	argv[8] = new char[32];
	argv[9] = new char[32];
	argv[10] = new char[32];
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);

	len = strlen("-of") + 1;
	argv[11] = new char[len];
	memset(argv[11], 0, len);
	memcpy(argv[11], "-of", len - 1);
	len = strlen(fmt) + 1;
	argv[12] = new char[len];
	memset(argv[12], 0, len);
	memcpy(argv[12], fmt, len - 1); //GTiff,BMP


	len = strlen("-srcnodata") + 1;
	argv[13] = new char[len];
	memset(argv[13], 0, len);
	memcpy(argv[13], "-srcnodata", len - 1);
	//sprintf(argv[15], "0 0 0 0"); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[15],"255 255 255 255"); //rgb 일때, 흰색이 투명색
	len = strlen("-999") + 1;
	argv[14] = new char[len];
	memset(argv[14], 0, len);
	memcpy(argv[14], "-999", len - 1);

	len = strlen(src) + 1;
	argv[15] = new char[len];
	memset(argv[15], 0, len);
	memcpy(argv[15], src, len - 1);

	len = strlen(dest) + 1;
	argv[16] = new char[len];
	memset(argv[16], 0, len);
	memcpy(argv[16], dest, len - 1);
	*/
	//EarlySetConfigOptions(argc, argv);
	//int argcc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
	//int argcd = CSLCount(argv);
	//EarlySetConfigOptions(argc, argv);
	//jsyun 20170721 It's path to all data , csv,....
	//CPLSetConfigOption("GDAL_DATA", "/home/hadoop/hadoop/lib/native/");

	char* block;
	int curidx = 0;
	isize = 0;
	GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	// 명령어 다음 부터 인자만 넘겨줌 그래서 + 1, 인자 갯수도 정확히 넘겨줌
	GDALWarpAppOptions *psOptions = NULL;//GDALWarpAppOptionsNew(argv, psOptionsForBinary, argc);

printf("%s\n",psOptionsForBinary->papszSrcFiles[0]);

	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */
	for (i = 0; psOptionsForBinary->papszSrcFiles[i] != NULL; i++)
	{
		nSrcCount++;
		pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH) * nSrcCount);
		pahSrcDS[nSrcCount - 1] = GDALOpenEx(psOptionsForBinary->papszSrcFiles[i], GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL,
			(const char* const*)psOptionsForBinary->papszOpenOptions, NULL);
		printf("%s\n",psOptionsForBinary->papszSrcFiles[i]);

		if (pahSrcDS[nSrcCount - 1] == NULL)
			return NULL;
	}
	GDALDatasetH hDstDS = NULL;
	/* -------------------------------------------------------------------- */
	/*      Does the output dataset already exist?                          */
	/* -------------------------------------------------------------------- */

		/* FIXME ? source filename=target filename and -overwrite is definitely */
		/* an error. But I can't imagine of a valid case (without -overwrite), */
		/* where it would make sense. In doubt, let's keep that dubious possibility... */



	if (!(psOptionsForBinary->bQuiet))
	{
		//0..10..이거 보이게 설정
		//GDALWarpAppOptionsSetProgress(psOptions, GDALTermProgress, NULL);
	}

	//	if (hDstDS == NULL && !psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet)
	//		CheckExtensionConsistency(psOptionsForBinary->pszDstFilename, psOptionsForBinary->pszFormat);
	int bUsageError = FALSE;
	GDALDatasetH hOutDS = GDALWarp("", hDstDS, //psOptionsForBinary->pszDstFilename
		nSrcCount, pahSrcDS, psOptions, &bUsageError);
	imageWidth = 0;
	int nRetCode = (hOutDS) ? 0 : 1;
	GDALDataset *pDS = NULL;
	if (hOutDS) {
		pDS = (GDALDataset *)hOutDS;
		GDALRasterBand  *poBand;
		dimension2di imagesize;
		rect2dd	rect;

		imagesize.Width = pDS->GetRasterXSize();
		imagesize.Height = pDS->GetRasterYSize();
		imageWidth = imagesize.Width;
		imageHeight = imagesize.Height;
		int band = pDS->GetRasterCount();

		if (!(band == 1 || band == 3 || band == 4)) return NULL; //3,4밴드, 24,32비트 만 변환함.
		int Bits = band * 8;
		if (band == 3 || band == 4) Bits = 24; //==> 3band만 출력함.
		//band = 3;//rgb만 받아옴
		void** pData = new void*[band];
		for (i = 0; i < band; i++) pData[i] = new char[imagesize.Width];//밴드별 메모리 할당
		int nRowBytes = ((((imagesize.Width * Bits) + 31) & ~31) / 8);//한줄의 byte사이즈, bmp 처리 때문에 한줄은 4로 나누어져야함. 
		isize = nRowBytes * imagesize.Height;
		data = new char[isize];
		memset(data, 0, isize);
		for (j = 0; j < imagesize.Height; j++) {
			for (i = 0; i < band; i++) {//데이터 로딩...
				poBand = pDS->GetRasterBand(i + 1); //1 부터 시작함, ㅋ
				//poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, GDT_Byte, 0, 0);// , &extraArg);
				GDALRasterIO((GDALRasterBandH)poBand,GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, GDT_Byte, 0, 0);// , &extraArg);
			}
			for (i = 0; i < imagesize.Width; i++) {
				//curidx = (imagesize.Height - j - 1)*nRowBytes  + i*band; // 거꾸로 저장됨
				if (band == 1) {
					curidx = (imagesize.Height - j - 1)*nRowBytes + i * band; // 거꾸로 저장됨 ==>bmp 가로 길이 4배수 아닌 경우 밀림
					block = (char*)pData[0];
					data[curidx + 2] = (block[i] >> 5) * 255 / 7;

					data[curidx + 1] = ((block[i] >> 2) & 0x07) * 255 / 7;

					data[curidx + 0] = (block[i] & 0x03) * 255 / 3;
					//data[curidx * 3 + 2] = block[i];

					//data[curidx * 3 + 1] = block[i];

					//data[curidx * 3 + 0] = block[i];
				}
				else {
					curidx = (imagesize.Height - j - 1)*nRowBytes + i * 3; // 거꾸로 저장됨 ==>bmp 가로 길이 4배수 아닌 경우 밀림
					if (imgType == 1) { //jpg 일 경우,,왜 jpg만 그런거지?
						block = (char*)pData[0];
						data[curidx + 0] = block[i];//데이터에 칼라값 저장

						block = (char*)pData[1];
						data[curidx + 1] = block[i];

						block = (char*)pData[2];
						data[curidx + 2] = block[i];
					}
					else { //png,dds,...

						block = (char*)pData[0];
						data[curidx + 2] = block[i];//데이터에 칼라값 저장

						block = (char*)pData[1];
						data[curidx + 1] = block[i];

						block = (char*)pData[2];
						data[curidx + 0] = block[i];
					}
				}

			}
		}
		for (i = 0; i < band; i++) delete pData[i];//메모리 해제..

		delete[] pData;
	}
	GDALWarpAppOptionsFree(psOptions);
	//주석 해제하면 오류 뜬다. 메모리 해제 문제 있음 new --> delete , malloc -> free
	//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	GDALClose(hOutDS ? hOutDS : hDstDS);
	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);

	//GDALDumpOpenDatasets(stderr);
	for (i = 0; i < argc; i++) {
		if (argv[i]) delete[] argv[i];
	}
	delete argv; 
	return data;
}
bool GDALTransFormImageTest(char* src, char* dest, char* srcCoord, char* destCoord,double xmin,double ymin,double xmax,double ymax,const char* fmt)
{
	GDALDatasetH *pahSrcDS = NULL;
	int nSrcCount = 1;
	//const char         *pszFormat = "BMP";//BMP,GTiff
	const char         *pszFormat = fmt;
	char               *pszTargetSRS = NULL;
	char               *pszSourceSRS = NULL;
	const char         *pszSrcFilename = NULL, *pszDstFilename = NULL;
	int                 bCreateOutput = FALSE, i, nOrder = 0;
	void               *hTransformArg, *hGenImgProjArg = NULL, *hApproxArg = NULL;
	char               **papszWarpOptions = NULL;
	double             dfErrorThreshold = 0.125;
	GDALTransformerFunc pfnTransformer = NULL;
	char                **papszCreateOptions = NULL;
	// 출력을 지원하는 영상 포맷 //
	/*
	Output driver `PNG' not recognised or does not support
		direct output file creation.The following format drivers are configured
		and support direct output :
	VRT: Virtual Raster
	GTiff : GeoTIFF
	NITF : National Imagery Transmission Format
	HFA : Erdas Imagine Images(.img)
	ELAS : ELAS
	MEM : In Memory Raster
	BMP : MS Windows Device Independent Bitmap
	PCIDSK : PCIDSK Database File
	PCRaster : PCRaster Raster File
	ILWIS : ILWIS Raster Map
	SGI : SGI Image File Format 1.0
	Leveller : Leveller heightfield
	Terragen : Terragen heightfield
	ISIS3 : USGS Astrogeology ISIS cube(Version 3)
	ISIS2 : USGS Astrogeology ISIS cube(Version 2)
	PDS4 : NASA Planetary Data System 4
	ERS : ERMapper.ers Labelled
	RMF : Raster Matrix Format
	RST : Idrisi Raster A.1
	INGR : Intergraph Raster
	GSBG : Golden Software Binary Grid(.grd)
	GS7BG : Golden Software 7 Binary Grid(.grd)
	PDF : Geospatial PDF
	MRF : Meta Raster Format
	PNM : Portable Pixmap Format(netpbm)
	PAux : PCI.aux Labelled
	MFF : Vexcel MFF Raster
	MFF2 : Vexcel MFF2(HKV) Raster
	BT : VTP.bt(Binary Terrain) 1.3 Format
	LAN : Erdas.LAN / .GIS
	IDA : Image Data and Analysis
	GTX : NOAA Vertical Datum.GTX
	NTv2 : NTv2 Datum Grid Shift
	CTable2 : CTable2 Datum Grid Shift
	KRO : KOLOR Raw
	ROI_PAC : ROI_PAC raster
	RRASTER : R Raster
	NWT_GRD : Northwood Numeric Grid Format.grd / .tab
	ADRG : ARC Digitized Raster Graphics
	SAGA : SAGA GIS Binary Grid(.sdat, .sg - grd - z)
	DB2ODBC : IBM DB2 Spatial Database
	ENVI : ENVI.hdr Labelled
	EHdr : ESRI.hdr Labelled
	ISCE : ISCE raster
	*/

	

	GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	// 명령어 다음 부터 인자만 넘겨줌 그래서 + 1, 인자 갯수도 정확히 넘겨줌
	//GDALWarpAppOptions *psOptions = GDALWarpAppOptionsNew(argv , psOptionsForBinary, argc);
	GDALWarpAppOptions *psOptions = static_cast<GDALWarpAppOptions *>(CPLCalloc(1, sizeof(GDALWarpAppOptions)));

	psOptions->dfMinX = 0.0;
	psOptions->dfMinY = 0.0;
	psOptions->dfMaxX = 0.0;
	psOptions->dfMaxY = 0.0;
	psOptions->dfXRes = 0.0;
	psOptions->dfYRes = 0.0;
	psOptions->bTargetAlignedPixels = false;
	psOptions->nForcePixels = 0;
	psOptions->nForceLines = 0;
	psOptions->bQuiet = true;
	psOptions->pfnProgress = GDALDummyProgress;
	psOptions->pProgressData = nullptr;
	psOptions->bEnableDstAlpha = false;
	psOptions->bEnableSrcAlpha = false;
	psOptions->bDisableSrcAlpha = false;
	psOptions->pszFormat = nullptr;
	psOptions->bCreateOutput = false;
	psOptions->papszWarpOptions = nullptr;
	psOptions->dfErrorThreshold = -1;
	psOptions->dfWarpMemoryLimit = 0.0;
	psOptions->papszCreateOptions = nullptr;
	psOptions->eOutputType = GDT_Unknown;
	psOptions->eWorkingType = GDT_Unknown;
	psOptions->eResampleAlg = GRA_NearestNeighbour;
	psOptions->pszSrcNodata = nullptr;
	psOptions->pszDstNodata = nullptr;
	psOptions->bMulti = false;
	psOptions->papszTO = nullptr;
	psOptions->pszCutlineDSName = nullptr;
	psOptions->pszCLayer = nullptr;
	psOptions->pszCWHERE = nullptr;
	psOptions->pszCSQL = nullptr;
	psOptions->bCropToCutline = false;
	psOptions->bCopyMetadata = true;
	psOptions->bCopyBandInfo = true;
	psOptions->pszMDConflictValue = CPLStrdup("*");
	psOptions->bSetColorInterpretation = false;
	psOptions->nOvLevel = -2;
	psOptions->bNoVShiftGrid = false;

	//원본 좌표계 설정
	char *pszSRS = SanitizeSRS(srcCoord);
	psOptions->papszTO = CSLSetNameValue(psOptions->papszTO, "SRC_SRS", pszSRS);
	//LFree(pszSRS);

	//대상 좌표계 설정
	char *pszSRS2 = SanitizeSRS(destCoord);
	//CPLFree(psOptions->pszTE_SRS);
	psOptions->pszTE_SRS = CPLStrdup(pszSRS2);
	//CPLFree(pszSRS2);
	psOptions->bCreateOutput = true;

	//이미지 추출 방식 설정
	psOptions->eResampleAlg = GRA_Bilinear;

	//이미지 추출 영역 설정
	psOptions->dfMinX = xmin;
	psOptions->dfMinY = ymin;
	psOptions->dfMaxX = xmax;
	psOptions->dfMaxY = ymax;
	psOptions->bCreateOutput = true;

	//이미지 추출 포맷 설정
	//CPLFree(psOptions->pszFormat);
	psOptions->pszFormat = CPLStrdup(fmt);
	psOptions->bCreateOutput = true;

	//nosrcdata
	//CPLFree(psOptions->pszSrcNodata);
	psOptions->pszSrcNodata = CPLStrdup("-999");

	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */
	pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH) * nSrcCount);
	pahSrcDS = (GDALDatasetH *)GDALOpenEx(src, GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL,
			(const char* const*)psOptionsForBinary->papszOpenOptions, NULL);

	GDALDatasetH hDstDS = NULL;
	
	//printf("cc - %s\n", psOptionsForBinary->pszDstFilename);
	bool bQuiet = false;
	if (!bQuiet)
	{
		GDALWarpAppOptionsSetProgress(psOptions, GDALTermProgress, NULL);
	}

//	if (hDstDS == NULL && !psOptionsForBinary->bQuiet && !psOptionsForBinary->bFormatExplicitlySet)
//		CheckExtensionConsistency(psOptionsForBinary->pszDstFilename, psOptionsForBinary->pszFormat);
	int bUsageError = FALSE;
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
	GDALDatasetH hOutDS = GDALWarp(dest, hDstDS,
		1, (GDALDatasetH *)pahSrcDS, psOptions, &bUsageError);
	printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");

	int nRetCode = (hOutDS) ? 0 : 1;
	//GDALWarpAppOptionsFree(psOptions);
	//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	GDALClose(hOutDS ? hOutDS : hDstDS);
	GDALClose(pahSrcDS);
	//CPLFree(pahSrcDS);

	//GDALDumpOpenDatasets(stderr);

	return true;
}
float * GDALGetDEMRect(char *src,int *witdh)
{
	GDALDataset  *poDataset = NULL;

	int i, j, k;
	poDataset = (GDALDataset *)GDALOpen((char*)src, GA_ReadOnly);
	double        adfGeoTransform[6];
	if (poDataset == NULL)
	{
		GDALClose((GDALDatasetH)poDataset);
		return NULL;
	}
	GDALRasterBand  *poBand;
	dimension2di imagesize;
	rect2dd	rect;
	double	resolution;
	imagesize.Width = poDataset->GetRasterXSize();
	imagesize.Height = poDataset->GetRasterYSize();
	*witdh = imagesize.Width;
	rect.ix = adfGeoTransform[0];
	rect.iy = adfGeoTransform[3] + poDataset->GetRasterYSize() * adfGeoTransform[5];
	rect.ax = adfGeoTransform[0] + poDataset->GetRasterXSize() * adfGeoTransform[1];
	rect.ay = adfGeoTransform[3];
	resolution = adfGeoTransform[1]; //x 해상도, adfGeoTransform[5];y 해상도
	int band = poDataset->GetRasterCount();
	int dataBits = 0;
	for (i = 0; i < band; i++) {
		poBand = poDataset->GetRasterBand(i + 1);
		dataBits += GDALGetDataTypeSize(poBand->GetRasterDataType());;
	}
	if (band != 1 || dataBits != 32) return NULL; //float 지형만 처리한다.
	void** pData = new void*[band];
	//for (i = 0; i < band; i++) pData[i] = VSIMalloc(imagesize.Width);//밴드별 메모리 할당
	for (i = 0; i < band; i++) pData[i] = new char[imagesize.Width*dataBits/8];//밴드별 메모리 할당
	float* block;
	int curidx = 0;
	//i,j 는 left-top 픽셀 위치이고 그 위치에 해당되는 값은 실제 세계 좌하단
	double xx, yy;
	GDALRasterIOExtraArg extraArg;
	extraArg.eResampleAlg = GRIORA_Cubic;
	int iy, ay;
	iy = 0;
	ay = imagesize.Height;
	float *data = new float[imagesize.Width*imagesize.Height];
	for (j = iy; j < ay; j++) {
		for (k = 0; k < band; k++) {//데이터 로딩...
			poBand = poDataset->GetRasterBand(k + 1);
			//poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, GDT_Float32, 0, 0);// , &extraArg);
			GDALRasterIO((GDALRasterBandH)poBand,GF_Read, 0, j, imagesize.Width, 1, pData[k], imagesize.Width, 1, GDT_Float32, 0, 0);// , &extraArg);
		}

		for (i = 0; i < imagesize.Width; i++) {

			curidx = (ay - j - 1)*imagesize.Width + i; // 거꾸로 저장됨

			block = (float*)pData[0];
			data[curidx] = block[i];//데이터에 칼라값 저장
		}
	}
	//if (band == 1) {
	//	SAFE_DELETE(pData);
	//}
	//else {
		for (i = 0; i < band; i++) delete pData[i];//메모리 해제..

		delete[] pData;
	//}
	GDALClose((GDALDatasetH)poDataset);
	return data;
}

short GDALGetBandCount(char *src) 
{
	short ret = -1;
	GDALDataset  *poDataset = NULL;

	int i, j, k;
	poDataset = (GDALDataset *)GDALOpen((char*)src, GA_ReadOnly);
	if (poDataset == NULL)
	{
		GDALClose((GDALDatasetH)poDataset);
		return -1;
	}

	int band = poDataset->GetRasterCount();
	GDALClose((GDALDatasetH)poDataset);
	return (short)band;
}
bool GDALIsDEM(char *src)
{
	bool ret = false;
	GDALDataset  *poDataset = NULL;

	int i, j, k;
	poDataset = (GDALDataset *)GDALOpen((char*)src, GA_ReadOnly);
	if (poDataset == NULL)
	{
		GDALClose((GDALDatasetH)poDataset);
		return false;
	}

	int band = poDataset->GetRasterCount();
	GDALClose((GDALDatasetH)poDataset);
	if (band == 1) return true;
	else return false;
}
char *GDALGetCityGML(char* src, char* srcCoord, char* destCoord, unsigned int &isize, int gmlType)
{
	return 0;
}
GDALDatasetH *GDALGetTransFormHandleIMG(char* src)
{
	GDALDatasetH *pahSrcDS = NULL;

	/* -------------------------------------------------------------------- */
	/*      Open Source files.                                              */
	/* -------------------------------------------------------------------- */
	pahSrcDS = (GDALDatasetH *)CPLRealloc(pahSrcDS, sizeof(GDALDatasetH));
	*pahSrcDS = GDALOpenEx(src, GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL,NULL, NULL);
	return pahSrcDS;
}

GDALWarpAppOptions *GDALWarpAppOptionsNew2(char** papszArgv);
 GDALWarpAppOptions *GDALSetTransFormOptionIMG(char* src, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize, int &imageWidth, int &imageHeight, int imgType, char r, char g, char b, char a, char *method,char *fname)
{
	// 출력을 지원하는 영상 포맷 //
	/*
	Output driver `PNG' not recognised or does not support
		direct output file creation.The following format drivers are configured
		and support direct output :
	VRT: Virtual Raster
	GTiff : GeoTIFF
	NITF : National Imagery Transmission Format
	HFA : Erdas Imagine Images(.img)
	ELAS : ELAS
	MEM : In Memory Raster
	BMP : MS Windows Device Independent Bitmap
	PCIDSK : PCIDSK Database File
	PCRaster : PCRaster Raster File
	ILWIS : ILWIS Raster Map
	SGI : SGI Image File Format 1.0
	Leveller : Leveller heightfield
	Terragen : Terragen heightfield
	ISIS3 : USGS Astrogeology ISIS cube(Version 3)
	ISIS2 : USGS Astrogeology ISIS cube(Version 2)
	PDS4 : NASA Planetary Data System 4
	ERS : ERMapper.ers Labelled
	RMF : Raster Matrix Format
	RST : Idrisi Raster A.1
	INGR : Intergraph Raster
	GSBG : Golden Software Binary Grid(.grd)
	GS7BG : Golden Software 7 Binary Grid(.grd)
	PDF : Geospatial PDF
	MRF : Meta Raster Format
	PNM : Portable Pixmap Format(netpbm)
	PAux : PCI.aux Labelled
	MFF : Vexcel MFF Raster
	MFF2 : Vexcel MFF2(HKV) Raster
	BT : VTP.bt(Binary Terrain) 1.3 Format
	LAN : Erdas.LAN / .GIS
	IDA : Image Data and Analysis
	GTX : NOAA Vertical Datum.GTX
	NTv2 : NTv2 Datum Grid Shift
	CTable2 : CTable2 Datum Grid Shift
	KRO : KOLOR Raw
	ROI_PAC : ROI_PAC raster
	RRASTER : R Raster
	NWT_GRD : Northwood Numeric Grid Format.grd / .tab
	ADRG : ARC Digitized Raster Graphics
	SAGA : SAGA GIS Binary Grid(.sdat, .sg - grd - z)
	DB2ODBC : IBM DB2 Spatial Database
	ENVI : ENVI.hdr Labelled
	EHdr : ESRI.hdr Labelled
	ISCE : ISCE raster
	*/

	// 아래의 인자 갯수가 변경되면 apps 폴더 아래의 gdal_warp_lib.cpp 안의 GDALWarpAppOptionsNew 에서 인자 갯수 변경해야함.
	// 마지막 인자 인덱스 보나 1 크게 입력
	const int argc = 19;//14;//9
	int i;
	/*char argv[19][256];
	for (i = 0; i < argc; i++) {
		memset(argv[i], 0, 256);
	}*/
	
	char** argv = 0;
	argv = new char*[argc];
	for (i = 0; i < argc; i++) {
		argv[i] = new char[256];
		memset(argv[i], 0, 256);
	}
	
	/*for (i = 0; i < argc; i++) {
		argv[i] = (char*)malloc(256);
		memset(argv[i], 0, 256);
	}*/
	/*
	//argv = (char**)malloc(sizeof(char)*argc);
	argv = (char**)CPLMalloc(17);

	for (i = 0; i < argc; i++) {
		argv[i] = (char*)CPLMalloc(256);
		memset(argv[i], 0, 256);
	}
	*/
	//sprintf(argv[0], "gdalwarpsimple");
	sprintf(argv[0], "-s_srs");
	sprintf(argv[1], "%s", srcCoord);
	//sprintf(argv[2],"'+proj=tmerc +lat_0=38 +lon_0=127 +k=1.000000 +x_0=200000 +y_0=600000 +ellps=GRS80 +datum=WGS84 +units=m +no_defs'");
	//sprintf(argv[2],"EPSG:4326");
	sprintf(argv[2], "-r");
	if (method == NULL)
		sprintf(argv[3], "bilinear");//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3
	else
		sprintf(argv[3], method);//near,bilinear,cubic,cubicspline,lanczos,average,mode,max,min,med,q1,q3	sprintf(argv[4], "-t_srs");
	sprintf(argv[4], "-t_srs");
	sprintf(argv[5], "%s", destCoord);//"EPSG:4326");
	sprintf(argv[6], "-te");
	sprintf(argv[7], "%f", xmin); //dest coordinate
	sprintf(argv[8], "%f", ymin);
	sprintf(argv[9], "%f", xmax);
	sprintf(argv[10], "%f", ymax);
	sprintf(argv[11], "-of");
	//sprintf(argv[13], "BMP");//GTiff,BMP
	sprintf(argv[12], "%s", "MEM");//GTiff,BMP,MEM,HFA

	sprintf(argv[13], "-srcnodata");
	//sprintf(argv[14], "0 0 0 0"); //rgb 일때 , 검은색이 투명색 ==> 초기에 수동 설정
	//sprintf(argv[15],"255 255 255 0"); //rgb 일때, 흰색이 투명색 ==> 초기에 수동 설정
	if (imgType == 1)
		sprintf(argv[14], "%d %d %d %d", b, g, r, a); //jpg 일 경우 반전으로 거꾸로 입력해사 원하는 색상 적용됨 2023.7.26
	else
		sprintf(argv[14], "%d %d %d %d", r, g, b, a); //rgb 일때 , 검은색이 투명색
	//sprintf(argv[14],"-999");   //지형 높이값일 때
	sprintf(argv[15], "-wm");
	sprintf(argv[16], "50"); //작업 메모리 크기
	//sprintf(argv[17], "256"); //dest height 

	sprintf(argv[17], src);
	//sprintf(argv[18], "/home/hadoop/project/jsyun/kafka/XDConvertImage/aa0a.tif");
	sprintf(argv[18], ""); //bmp
	//sprintf(argv[18], fname); //bmp
	//printf("fname : %s\n",fname);
	//GDALWarpAppOptionsForBinary* psOptionsForBinary = GDALWarpAppOptionsForBinaryNew();
	//for (i = 0; i < argc; i++) printf("%d/%d - %s\n",i,argc,argv[i]);
	
	
	// 명령어 다음 부터 인자만 넘겨줌 그래서 + 1, 인자 갯수도 정확히 넘겨줌
	GDALWarpAppOptions *psOptions = GDALWarpAppOptionsNew2(argv);
	for (i = 0; i < argc; i++) {
		delete [] argv[i];
	}
	delete argv;
	argv = NULL; 
	return psOptions;
}
char * GDALTransFormImageByHandleIMG(GDALDatasetH *pahSrcDS, int nSrcCount, GDALWarpAppOptions *psOptions, char* srcCoord, char* destCoord, double xmin, double ymin, double xmax, double ymax, unsigned int &isize, int &imageWidth, int &imageHeight, int imgType, char r, char g, char b, char a)
{
	char *data = NULL;
	GDALDatasetH hDstDS = NULL;
	int i, j;
	char* block;
	int curidx = 0;
	

	//if (!(psOptionsForBinary->bQuiet))
	{
		//0..10..이거 보이게 설정
		//GDALWarpAppOptionsSetProgress(psOptions, GDALTermProgress, NULL);
	}

	int bUsageError = FALSE;
	//pszDstFilename
	//psOptionsForBinary->pszDstFilename
	//(const char*)fname
	GDALDatasetH hOutDS = GDALWarp("", hDstDS, //psOptionsForBinary->pszDstFilename
		nSrcCount, pahSrcDS, psOptions, &bUsageError);
	imageWidth = 0;
	int nRetCode = (hOutDS) ? 0 : 1;
	GDALDataset *pDS = NULL;
	if (hOutDS) {
		pDS = (GDALDataset *)hOutDS;
		GDALRasterBand  *poBand;
		dimension2di imagesize;
		rect2dd	rect;
		imagesize.Width = pDS->GetRasterXSize();
		imagesize.Height = pDS->GetRasterYSize();
		imageWidth = imagesize.Width;
		imageHeight = imagesize.Height;
		int band = pDS->GetRasterCount();

		if (!(band == 1 || band == 3 || band == 4)) return NULL; //3,4밴드, 24,32비트 만 변환함.
		int Bits = band * 8;
		if (band == 3 || band == 4) Bits = 24; //==> 3band만 출력함.
		//band = 3;//rgb만 받아옴
		void** pData = new void*[band];
		for (i = 0; i < band; i++) pData[i] = new char[imagesize.Width];//밴드별 메모리 할당
		int nRowBytes = ((((imagesize.Width * Bits) + 31) & ~31) / 8);//한줄의 byte사이즈, bmp 처리 때문에 한줄은 4로 나누어져야함. 
		isize = nRowBytes * imagesize.Height;
		data = new char[isize];
		memset(data, 0, isize);
		//printf("2-1 - imagesize height : %d\n",imagesize.Height);
		
		for (j = 0; j < imagesize.Height; j++) {
			for (i = 0; i < band; i++) {//데이터 로딩...
				poBand = pDS->GetRasterBand(i + 1); //1 부터 시작함, ㅋ
				//poBand->RasterIO(GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, GDT_Byte, 0, 0);// , &extraArg);
				GDALRasterIO((GDALRasterBandH)poBand,GF_Read, 0, j, imagesize.Width, 1, pData[i], imagesize.Width, 1, GDT_Byte, 0, 0);// , &extraArg);
			}

			for (i = 0; i < imagesize.Width; i++) {
				//curidx = (imagesize.Height - j - 1)*nRowBytes  + i*band; // 거꾸로 저장됨
				if (band == 1) {
					curidx = (imagesize.Height - j - 1)*nRowBytes + i * band; // 거꾸로 저장됨 ==>bmp 가로 길이 4배수 아닌 경우 밀림
					block = (char*)pData[0];
					data[curidx + 2] = (block[i] >> 5) * 255 / 7;

					data[curidx + 1] = ((block[i] >> 2) & 0x07) * 255 / 7;

					data[curidx + 0] = (block[i] & 0x03) * 255 / 3;
					//data[curidx * 3 + 2] = block[i];

					//data[curidx * 3 + 1] = block[i];

					//data[curidx * 3 + 0] = block[i];
				}
				else {
					curidx = (imagesize.Height - j - 1)*nRowBytes + i * 3; // 거꾸로 저장됨 ==>bmp 가로 길이 4배수 아닌 경우 밀림
					if (imgType == 1) { //jpg 일 경우,,왜 jpg만 그런거지?
						block = (char*)pData[0];
						data[curidx + 0] = block[i];//데이터에 칼라값 저장

						block = (char*)pData[1];
						data[curidx + 1] = block[i];

						block = (char*)pData[2];
						data[curidx + 2] = block[i];
					}
					else { //png,dds,...

						block = (char*)pData[0];
						data[curidx + 2] = block[i];//데이터에 칼라값 저장

						block = (char*)pData[1];
						data[curidx + 1] = block[i];

						block = (char*)pData[2];
						data[curidx + 0] = block[i];
					}
				}

			}
		}
		for (i = 0; i < band; i++) delete pData[i];//메모리 해제..

		delete[] pData;
		
	}
	
	GDALClose(hOutDS);
	//GDALDumpOpenDatasets(stderr);
	/*
	GDALWarpAppOptionsFree(psOptions);
	//주석 해제하면 오류 뜬다. 메모리 해제 문제 있음 new --> delete , malloc -> free
	//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	GDALClose(hOutDS ? hOutDS : hDstDS);
	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);

	GDALDumpOpenDatasets(stderr);
	//GDALEnd()로 대체
	*/
	return data;
}
void GDALFreeHandleIMG(GDALDatasetH *pahSrcDS, int  nSrcCount) {
	//GDALWarpAppOptionsFree(psOptions);
	//주석 해제하면 오류 뜬다. 메모리 해제 문제 있음 new --> delete , malloc -> free
	//GDALWarpAppOptionsForBinaryFree(psOptionsForBinary);

	for (int i = 0; i < nSrcCount; i++)
	{
		GDALClose(pahSrcDS[i]);
	}
	CPLFree(pahSrcDS);
	//해제가 안된 핸들 목록이 나온다. 아예 안 나오는 것이 정상이다. 목록이 나오면 어디선가 핸들를 해제 안했다는 거임.
	//GDALDumpOpenDatasets(stderr);
}