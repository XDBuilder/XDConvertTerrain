# 요구 CMake 최소 버전
CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
 
# 프로젝트 이름 및 버전
PROJECT ( XDConvertImage )

# 메세지 출력
MESSAGE ( ${CMAKE_PROJECT_NAME} )

SET ( PROJECT_VERSION_MAJOR 0 )
SET ( PROJECT_VERSION_MINOR 1 )

# 빌드 형상(Configuration) 및 주절주절 Makefile 생성 여부
SET ( CMAKE_BUILD_TYPE Debug )
#SET ( CMAKE_VERBOSE_MAKEFILE true )


# 빌드 대상 바이너리 파일명 및 소스 파일 목록
SET ( OUTPUT_ELF
        "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
		
SET ( SRC_FILES 
	common.cpp 
	XDConvertImage.cpp 
	XDProjAPI.cpp 
	XDRasterAPI.cpp
	gdalManage.cpp
	images/Baseimage.cpp  images/CImage.cpp  images/CImagePNG.cpp images/CColorConverter.cpp
	images/libpng/png.c  images/libpng/pngerror.c	images/libpng/pngget.c  images/libpng/pngmem.c  images/libpng/pngpread.c  images/libpng/pngread.c  images/libpng/pngrio.c  
	images/libpng/pngrtran.c  images/libpng/pngrutil.c  images/libpng/pngset.c  images/libpng/pngtrans.c  images/libpng/pngvcrd.c  images/libpng/pngwio.c  images/libpng/pngwrite.c  
	images/libpng/pngwtran.c  images/libpng/pngwutil.c
	zip/zlib/adler32.c  zip/zlib/compressz.c  zip/zlib/crc32z.c  zip/zlib/deflate.c  zip/zlib/gzio.c  zip/zlib/infback.c  zip/zlib/inffast.c  zip/zlib/inflate.c  zip/zlib/inftrees.c  zip/zlib/trees.c  zip/zlib/uncompr.c  zip/zlib/zutil.c
	)

# 출력 디렉토리	
#SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY bin )
SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY lib )
SET ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib )


SET ( CMAKE_PREFIX_PATH /home/hadoop/project/jsyun/gdal/GDAL/build )
SET ( GDAL_DIR /home/hadoop/project/jsyun/gdal/GDAL/build )
SET ( CMAKE_MODULE_PATH /home/hadoop/project/jsyun/gdal/GDAL/build )
SET ( CMAKE_CURRENT_LIST_DIR /home/hadoop/project/jsyun/gdal/GDAL/build/CMakeFiles/Export/lib/cmake/gdal )

#add_subdirectory(GDAL)
# 공통 컴파일러
SET ( CMAKE_C_COMPILER "g++" )

# 공통 헤더 파일 Include 디렉토리 (-I)
INCLUDE_DIRECTORIES ( 
	/usr/include/glib-2.0
	/usr/lib/x86_64-linux-gnu/glib-2.0/include
	/usr/include
	/usr/include/gdal ) 

# 공통 컴파일 옵션, 링크 옵션
ADD_COMPILE_OPTIONS (-Wall -g -lstdc++fs -std=c++17 -fpermissive )
#SET ( CMAKE_EXE_LINKER_FLAGS "-static -Wl,--gc-sections" )

# 공통 링크 라이브러리 디렉토리 (-L)
LINK_DIRECTORIES ( /home/hadoop/project/jsyun/kafka/XDConvertImage )
	
# 공통 링크 라이브러리 (-l)
LINK_LIBRARIES( glib-2.0 rdkafka pthread dl XDProj XDRaster gdal )

# 빌드 대상 바이너리 추가	
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME} ${SRC_FILES} )
	
#find_package(GDAL CONFIG REQUIRED)
#SET ( CMAKE_GDAL_LIB_PATH /home/hadoop/project/jsyun/kafka/XDConvertImage )
#target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE gdal.so.36.3.11)
#target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_GDAL_LIB_PATH}/libs")
#target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_GDAL_LIB_PATH}/libs")
	