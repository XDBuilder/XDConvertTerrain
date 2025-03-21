#include "libpng/png.h" // use included lib png
#include "libpng/pngstruct.h" // use included lib png
#include "CImagePNG.h" 
#include <string.h>
//#pragma comment(lib, "png/lib/png.lib")

// PNG function for error handling
static void png_cpexcept_error(png_structp png_ptr, png_const_charp msg)
{
//	os::Printer::log("PNG FATAL ERROR", msg, ELL_ERROR);
	longjmp(png_ptr->jmp_buf_local, 1);
}

// PNG function for file reading
void PNGAPI user_read_data_fcn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;
	// changed by zola {
	FILE* file=(FILE*)png_ptr->io_ptr;
	check=(png_size_t) fread((void*)data,length,1,file);
	// }
	if (check != 1)//length
		png_error(png_ptr, "Read Error");

}

// PNG function for file writing
void PNGAPI user_write_data_fcn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;
	//fprintf(stderr,"\n CImagePNG.cpp -- png_ptr->bHDFS==false");
	FILE* file=(FILE*)png_ptr->io_ptr;
	check=(png_size_t) fwrite((void*)data,length,1,file);
	if (check*length != length)
		png_error(png_ptr, "Write Error");
}

CImagePNG::CImagePNG()
{
	// do something?
}

CImagePNG::~CImagePNG()
{
	// do something?
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImagePNG::isALoadableFileExtension(const c8* fileName)
{
	// added fix for file extension check by jox
	const c8* ext = strrchr(fileName, '.');
	if (ext == 0)
		return false;
	return (strcmp(ext, ".PNG") == 0) || (strcmp(ext, ".png") == 0);
}


//! returns true if the file maybe is able to be loaded by this class
bool CImagePNG::isALoadableFileFormat( FILE* file )
{
	if (!file)
		return false;

	png_byte buffer[8];
	// Read the first few bytes of the PNG file
	if (fread(buffer, 8, 1, file) != 8)
		return false;

	// Check if it really is a PNG file
	return !png_sig_cmp(buffer, 0, 8);
}


// load in the image data
CImage* CImagePNG::loadImage(FILE* file)
{
	if (!file)
		return 0; 
	Image = 0;
	RowPointers = 0;

	png_byte buffer[8];
	fseek( file, 0, SEEK_SET);
	// Read the first few bytes of the PNG file
	if( fread(buffer, 8, 1, file) != 1) //8 )
	{
	//		os::Printer::log("LOAD PNG: can't read file\n", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Check if it really is a PNG file
	if( png_sig_cmp(buffer, 0, 8) )
	{
//		os::Printer::log("LOAD PNG: not really a png\n", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Allocate the png read struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, (png_error_ptr)png_cpexcept_error, NULL);
		
	//png_ptr->offset=8;//위에서 읽고 시작함.
	if (!png_ptr)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create read struct failure\n", file->getFileName(), ELL_ERROR);
		return 0;
	}
//if(png_ptr->io_ptr==NULL) {
//	fprintf(stderr,"\n CImagePNG.cpp - loadImage 3 - png_ptr->io_ptr==NULL");
//}

	// Allocate the png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create info struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 4");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n CImagePNG.cpp - loadImage 4 - png_ptr->io_ptr==NULL");


	// for proper error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		if (RowPointers)
			delete [] RowPointers;
		return 0;
	}
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 5");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n CImagePNG.cpp - loadImage 5 - png_ptr->io_ptr==NULL");


	// changed by zola so we don't need to have public FILE pointers
	png_set_read_fn(png_ptr, file, user_read_data_fcn);

//fprintf(stderr,"\n CImagePNG.cpp - loadImage 6");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n CImagePNG.cpp - loadImage 6 - png_ptr->io_ptr==NULL");

	png_set_sig_bytes(png_ptr, 8); // Tell png that we read the signature
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 7");
	png_read_info(png_ptr, info_ptr); // Read the info section of the png file
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 8");
	// Extract info
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32*)&Width, (png_uint_32*)&Height,
		&BitDepth, &ColorType, NULL, NULL, NULL);

	// Convert palette color to true color
	if (ColorType==PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 9");
	// Convert low bit colors to 8 bit colors
//	if (BitDepth < 8)
//	{
//		if (ColorType==PNG_COLOR_TYPE_GRAY || ColorType==PNG_COLOR_TYPE_GRAY_ALPHA)
//			png_set_gray_1_2_4_to_8(png_ptr);
//		else
//			png_set_packing(png_ptr);
//	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// Convert high bit colors to 8 bit colors
	if (BitDepth == 16)
		png_set_strip_16(png_ptr);

	// Convert gray color to true color
	if (ColorType==PNG_COLOR_TYPE_GRAY || ColorType==PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 10");
	// Update the changes
	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32*)&Width, (png_uint_32*)&Height, &BitDepth, &ColorType, NULL, NULL, NULL);

	// Convert RGBA to BGRA
	if (ColorType==PNG_COLOR_TYPE_RGB_ALPHA)
	{
//#ifdef __BIG_ENDIAN__
//		png_set_swap_alpha(png_ptr);
//#else
		png_set_bgr(png_ptr);
//#endif
	}
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 11");
	// Update the changes
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32*)&Width, (png_uint_32*)&Height,
		&BitDepth, &ColorType, NULL, NULL, NULL);

	// Create the image structure to be filled by png data
	if (ColorType==PNG_COLOR_TYPE_RGB_ALPHA)
		Image = new CImage(ECF_A8R8G8B8, dimension2di(Width, Height));
	else
		Image = new CImage(ECF_R8G8B8, dimension2di(Width, Height));
	if (!Image)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create image struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 12");
	// Create array of pointers to rows in image data
	RowPointers = new png_bytep[Height];
	if (!RowPointers)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create row pointers failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		delete Image;
		return 0;
	}

	// Fill array of pointers to rows in image data
	unsigned char* data = (unsigned char*)Image->lock();
	for (u32 i=0; i<Height; ++i)
	{
		RowPointers[i]=data;
		data += Image->getPitch();
	}

	// for proper error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		delete [] RowPointers;
		Image->unlock();
		delete [] Image;
		return 0;
	}
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 13");
	// Read data using the library function that handles all transformations including interlacing
	png_read_image(png_ptr, RowPointers);
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 14");
	png_read_end(png_ptr, NULL);
	delete [] RowPointers;
	Image->unlock();
	png_destroy_read_struct(&png_ptr,&info_ptr, 0); // Clean up memory
//fprintf(stderr,"\n CImagePNG.cpp - loadImage 15");
	return Image;

}



bool CImagePNG::writeImage(FILE* file, CImage* image,u32 param)
{
	
//fprintf(stderr,"\n CImagePNG.cpp - writeImage 1");
	
	if (!file) return false;
//fprintf(stderr,"\n CImagePNG.cpp - writeImage 1-1");
	if (!image) return false;
//fprintf(stderr,"\n CImagePNG.cpp - writeImage 1-2");

	// Allocate the png write struct
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, (png_error_ptr)png_cpexcept_error, NULL);
	if (!png_ptr)
	{
//		AfxMessageBox("LOAD PNG: Internal PNG create write struct failure");
//		os::Printer::log("LOAD PNG: Internal PNG create write struct failure\n", file->getFileName(), ELL_ERROR);
		return false;
	}
//fprintf(stderr,"\n CImagePNG.cpp - writeImage 2");

	// Allocate the png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
//		AfxMessageBox("LOAD PNG: Internal PNG create info struct failure");
//		os::Printer::log("LOAD PNG: Internal PNG create info struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	// for proper error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}
//fprintf(stderr,"\n CImagePNG.cpp - writeImage 3");
	png_set_write_fn(png_ptr, file, user_write_data_fcn, NULL);

	// Set info
	switch(image->getColorFormat())
	{
		case ECF_A8R8G8B8:
		case ECF_A1R5G5B5:
			png_set_IHDR(png_ptr, info_ptr,
				image->getDimension().Width, image->getDimension().Height,
				8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		break;
		default:
			png_set_IHDR(png_ptr, info_ptr,
				image->getDimension().Width, image->getDimension().Height,
				8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	}

//fprintf(stderr,"\n writeImage 4");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n writeImage 4 - png_ptr->io_ptr==NULL");
//else fprintf(stderr,"\n writeImage 4 - png_ptr->io_ptr==NOT NULL");
	s32 lineWidth=image->getDimension().Width;
	switch(image->getColorFormat())
	{
	case ECF_R8G8B8:
	case ECF_R5G6B5:
		lineWidth*=3;
		break;
	case ECF_A8R8G8B8:
	case ECF_A1R5G5B5:
		lineWidth*=4;
		break;
	}
	u8* tmpImage = new u8[image->getDimension().Height*lineWidth];
	if (!tmpImage)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create image failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}
//fprintf(stderr,"\n writeImage 5");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n writeImage 5 - png_ptr->io_ptr==NULL");
//else fprintf(stderr,"\n writeImage 5 - png_ptr->io_ptr==NOT NULL");
	u8* data = (u8*)image->lock();
	switch(image->getColorFormat())
	{
	case ECF_R8G8B8:
		CColorConverter::convert_R8G8B8toR8G8B8(data,image->getDimension().Height*image->getDimension().Width,tmpImage);
		break;
	case ECF_A8R8G8B8:
		CColorConverter::convert_A8R8G8B8toA8R8G8B8(data,image->getDimension().Height*image->getDimension().Width,tmpImage);
		break;
	case ECF_R5G6B5:
		CColorConverter::convert_R5G6B5toR8G8B8(data,image->getDimension().Height*image->getDimension().Width,tmpImage);
		break;
	case ECF_A1R5G5B5:
		CColorConverter::convert_A1R5G5B5toA8R8G8B8(data,image->getDimension().Height*image->getDimension().Width,tmpImage);
		break;
	}
	image->unlock();
	// Create array of pointers to rows in image data
	RowPointers = new png_bytep[image->getDimension().Height];
	if (!RowPointers)
	{
//		os::Printer::log("LOAD PNG: Internal PNG create row pointers failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete [] tmpImage;
		return false;
	}
//fprintf(stderr,"\n writeImage 6");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n writeImage 6 - png_ptr->io_ptr==NULL");
//else fprintf(stderr,"\n writeImage 6 - png_ptr->io_ptr==NOT NULL");
	data=tmpImage;
	// Fill array of pointers to rows in image data
	for (s32 i=0; i<image->getDimension().Height; ++i)
	{
		RowPointers[i]=data;
		data += lineWidth;
	}
	// for proper error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{

		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete [] RowPointers;
		delete [] tmpImage;
		return false;
	}
	png_init_io (png_ptr, file);
	png_set_rows(png_ptr, info_ptr, RowPointers);
//fprintf(stderr,"\n writeImage 7");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n writeImage 7 - png_ptr->io_ptr==NULL");
//else fprintf(stderr,"\n writeImage 7 - png_ptr->io_ptr==NOT NULL");
	if (image->getColorFormat()==ECF_A8R8G8B8 || image->getColorFormat()==ECF_A1R5G5B5) {
//fprintf(stderr,"\n writeImage 8");
//if(png_ptr->io_ptr==NULL) fprintf(stderr,"\n writeImage 8 - png_ptr->io_ptr==NULL");
//else fprintf(stderr,"\n writeImage 8 - png_ptr->io_ptr==NOT NULL");
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);
//fprintf(stderr,"\n writeImage 9");
		
	}
	else
	{
//fprintf(stderr,"\n writeImage 10");
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
//fprintf(stderr,"\n writeImage 11");
	}

	delete [] RowPointers;
	delete [] tmpImage;
	png_destroy_write_struct(&png_ptr, &info_ptr);
//fprintf(stderr,"\n writeImage 8");
	return true;

}
