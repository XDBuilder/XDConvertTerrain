
#include <setjmp.h>
#include "CImageJPG.h"
#include "CImageBMP.h"


    // struct for handling jpeg errors
    struct xd_jpeg_error_mgr {
        // public jpeg error fields
        struct jpeg_error_mgr pub;
        // for longjmp, to return to caller on a fatal error
        jmp_buf setjmp_buffer;
    };

void CImageJPG::init_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}



boolean CImageJPG::fill_input_buffer (j_decompress_ptr cinfo)
{
	// DO NOTHING
	return 1;
}



void CImageJPG::skip_input_data (j_decompress_ptr cinfo, long count)
{
	jpeg_source_mgr * src = cinfo->src;
	if(count > 0)
	{
		src->bytes_in_buffer -= count;
		src->next_input_byte += count;
	}
}



void CImageJPG::term_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}


void CImageJPG::error_exit (j_common_ptr cinfo)
{
	// unfortunately we need to use a goto rather than throwing an exception
	// as gcc crashes under linux crashes when using throw from within
	// extern c code

	// Always display the message
	(*cinfo->err->output_message) (cinfo);

	// cinfo->err really points to a xd_error_mgr struct
	xd_jpeg_error_mgr *myerr = (xd_jpeg_error_mgr*) cinfo->err;

	longjmp(myerr->setjmp_buffer, 1);
}


void CImageJPG::output_message(j_common_ptr cinfo)
{
	// display the error message.
	c8 temp1[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, temp1);
//	os::Printer::log("JPEG FATAL ERROR",temp1, ELL_ERROR);
}

// returns true if the file maybe is able to be loaded by this class
bool CImageJPG::isALoadableFileExtension(const c8* fileName)
{
	return strstr(fileName, ".jpg") != 0;
}

//! returns true if the file maybe is able to be loaded by this class
bool CImageJPG::isALoadableFileFormat(FILE* file)
{
	if (!file)
		return false;

	s32 jfif = 0;
	fseek(file, 6, SEEK_SET);
	fread(&jfif, sizeof(s32), 1, file);
	return (jfif == 0x4a464946 || jfif == 0x4649464a);
}

//! creates a surface from the file
CImage* CImageJPG::loadImage( FILE* file )
{
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	u8 **rowPtr=0;
	u8* input = new u8[fileSize];
	fread( input, fileSize, 1, file);

	// allocate and initialize JPEG decompression object
	struct jpeg_decompress_struct cinfo;
	struct xd_jpeg_error_mgr jerr;

	//We have to set up the error handler first, in case the initialization
	//step fails.  (Unlikely, but it could happen if you are out of memory.)
	//This routine fills in the contents of struct jerr, and returns jerr's
	//address which we place into the link field in cinfo.

	cinfo.err = jpeg_std_error(&jerr.pub);
	cinfo.err->error_exit = error_exit;
	cinfo.err->output_message = output_message;

	// compatibility fudge:
	// we need to use setjmp/longjmp for error handling as gcc-linux
	// crashes when throwing within external c code
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object and return.

		jpeg_destroy_decompress(&cinfo);

		delete [] input;
		// if the row pointer was created, we delete it.
		if (rowPtr)
			delete [] rowPtr;

		// return null pointer
		return 0;
	}

	// Now we can initialize the JPEG decompression object.
	jpeg_create_decompress(&cinfo);

	// specify data source
	jpeg_source_mgr jsrc;

	// Set up data pointer
	jsrc.bytes_in_buffer = fileSize;
	jsrc.next_input_byte = (JOCTET*)input;
	cinfo.src = &jsrc;

	jsrc.init_source = init_source;
	jsrc.fill_input_buffer = fill_input_buffer;
	jsrc.skip_input_data = skip_input_data;
	jsrc.resync_to_restart = jpeg_resync_to_restart;
	jsrc.term_source = term_source;

	// Decodes JPG input from whatever source
	// Does everything AFTER jpeg_create_decompress
	// and BEFORE jpeg_destroy_decompress
	// Caller is responsible for arranging these + setting up cinfo

	// read file parameters with jpeg_read_header()
	jpeg_read_header(&cinfo, TRUE);

	cinfo.out_color_space=JCS_RGB;
	cinfo.out_color_components=3;
	cinfo.do_fancy_upsampling=FALSE;

	// Start decompressor
	jpeg_start_decompress(&cinfo);

	// Get image data
	u16 rowspan = cinfo.image_width * cinfo.out_color_components;
	u32 width = cinfo.image_width;
	u32 height = cinfo.image_height;

	// Allocate memory for buffer
	u8* output = new u8[rowspan * height];

	// Here we use the library's state variable cinfo.output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
	// Create array of row pointers for lib
	rowPtr = new u8* [height];

	for( u32 i = 0; i < height; i++ )
		rowPtr[i] = &output[ i * rowspan ];

	u32 rowsRead = 0;

	while( cinfo.output_scanline < cinfo.output_height )
		rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );

	delete [] rowPtr;
	// Finish decompression

	jpeg_finish_decompress(&cinfo);

	// Release JPEG decompression object
	// This is an important step since it will release a good deal of memory.
	jpeg_destroy_decompress(&cinfo);

	// convert image
	CImage* image = new CImage(ECF_R8G8B8, dimension2di(width, height), output);

	delete [] input;

	return image;
}


/////////////////////////////////////////////////////////////////////////
//write jpeg
/////////////////////////////////////////////////////////////////////////

	typedef struct {
		struct jpeg_destination_mgr pub;	/* public fields */
		JOCTET * buffer;					/* image buffer */
		u32 buffer_size;					/* image buffer size */
	} mem_destination_mgr;


typedef mem_destination_mgr * mem_dest_ptr;

void init_destination (j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;

	/* image buffer must be allocated before mem_dest routines are called.  */
	if(dest->buffer == NULL) {
		//fprintf(stderr, "jmem_dest: init_destination: buffer not allocated\n");
	}

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->buffer_size;
}


boolean empty_output_buffer (j_compress_ptr cinfo)
{
	//mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
	// empty_output_buffer: buffer should not ever be full\n");
	return FALSE;
}


void term_destination (j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
	size_t datacount = dest->buffer_size - dest->pub.free_in_buffer;
}

void jpeg_memory_dest (j_compress_ptr cinfo, u8 *jfif_buffer,
                  s32 buf_size)
{
  mem_dest_ptr dest;

  if(jfif_buffer == NULL) {
    //fprintf(stderr, "jpeg_memory_dest: memory buffer needs to be allocated\n");
    //ERREXIT(cinfo, JERR_BUFFER_SIZE);
	  return;
  }

  if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  (size_t) sizeof(mem_destination_mgr));
  }

  dest = (mem_dest_ptr) cinfo->dest;  /* for casting */

  /* Initialize method pointers */
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;

  /* Initialize private member */
  dest->buffer = (JOCTET*)jfif_buffer;
  dest->buffer_size = buf_size;
}

/* write_JPEG_memory: store JPEG compressed image into memory.
*/
void write_JPEG_memory (void *img_buf, s32 width, s32 height, u32 bpp, u32 pitch,
					u8 *jpeg_buffer, u32 jpeg_buffer_size,
					s32 quality, u32 *jpeg_comp_size)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	/* More stuff */
	JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_memory_dest(&cinfo, jpeg_buffer, jpeg_buffer_size);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = bpp;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = (u8*) img_buf + (cinfo.next_scanline * pitch );
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);

	{
		mem_dest_ptr dest = (mem_dest_ptr) cinfo.dest;
		*jpeg_comp_size = dest->buffer_size - dest->pub.free_in_buffer;
	}

	jpeg_destroy_compress(&cinfo);
}

bool CImageJPG::writeImage( FILE* file, CImage* input, u32 quality )
{
	dimension2di dim = input->getDimension();
	CImage * image = new CImage(ECF_R8G8B8, dim );

	void (*format)(const void*, s32, void*) = 0;
	switch( input->getColorFormat () )
	{
		case ECF_R8G8B8:	format = CColorConverter::convert_R8G8B8toR8G8B8; break;
		case ECF_A8R8G8B8:	format = CColorConverter::convert_A8R8G8B8toR8G8B8; break;
		case ECF_A1R5G5B5:	format = CColorConverter::convert_A1R5G5B5toB8G8R8; break;
		case ECF_R5G6B5:	format = CColorConverter::convert_R5G6B5toR8G8B8; break;
	}

	// couldn't find a color converter
	if ( 0 == format ) return false;

	s32 y;
	void *src = input->lock();
	void *dst = image->lock();
	for ( y = 0; y!= dim.Height; ++y )
	{
		format( src, dim.Width, dst );
		src = (void*) ( (u8*) src + input->getPitch () );
		dst = (void*) ( (u8*) dst + image->getPitch () );
	}
	input->unlock ();
	image->unlock ();

	// temp buffer
	u32 destSize = image->getImageDataSizeInBytes ();
	u8 * dest = new u8 [ destSize ];
	
	if ( 0 == quality ) quality = 75;

	write_JPEG_memory ( image->lock(), dim.Width, dim.Height,
				image->getBytesPerPixel(), image->getPitch(),
				dest, destSize,
				quality,
				&destSize);

	fwrite ( dest, destSize,1,file );

	delete image;
	delete [] dest;

	return true;
}


///////////////////////////////////////////////////////////////////////////
////Jpg2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool CImageJPG::jpg2bmp(char* jpgname, char* bmpname)
{
	FILE* file=fopen(jpgname,"rb");
	CImage* image=loadImage(file);
	fclose(file);

	if(image==0) return false;

	CImageBMP ibmp;
	file=fopen(bmpname,"wb");
	ibmp.writeImage(file,image,0);
	fclose(file);

	return true;
}
