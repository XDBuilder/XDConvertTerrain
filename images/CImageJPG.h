#ifndef __C_IMAGE_JPG_H_INCLUDED__
#define __C_IMAGE_JPG_H_INCLUDED__

#include "CImage.h"
extern "C"
{
	#include "libjpeg/jpeglib.h" // use  jpeglib
}

//! Surface Loader for JPG images
class CImageJPG 
{
public:

	//! constructor
	CImageJPG(){}

	//! destructor
	virtual ~CImageJPG(){}

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const c8* fileName);

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(FILE* file);

	//! creates a surface from the file
	virtual CImage* loadImage( FILE* file);
  	// creates a file from the surface
	virtual bool writeImage( FILE* file, CImage* input, u32 quality);

	bool jpg2bmp(char* jpgname,char* bmpname);

private:

	/* 	Receives control for a fatal error.  Information sufficient to
	generate the error message has been stored in cinfo->err; call
	output_message to display it.  Control must NOT return to the caller;
	generally this routine will exit() or longjmp() somewhere.
	Typically you would override this routine to get rid of the exit()
	default behavior.  Note that if you continue processing, you should
	clean up the JPEG object with jpeg_abort() or jpeg_destroy().
	*/
	static void error_exit (j_common_ptr cinfo);

	/* output error messages via logger. */
	static void output_message(j_common_ptr cinfo);

	/*	Initialize source.  This is called by jpeg_read_header() before any
	data is actually read.  Unlike init_destination(), it may leave
	bytes_in_buffer set to 0 (in which case a fill_input_buffer() call
	will occur immediately). */
	static void init_source (j_decompress_ptr cinfo);

	/*	This is called whenever bytes_in_buffer has reached zero and more
	data is wanted.  In typical applications, it should read fresh data
	into the buffer (ignoring the current state of next_input_byte and
	bytes_in_buffer), reset the pointer & count to the start of the
	buffer, and return TRUE indicating that the buffer has been reloaded.
	It is not necessary to fill the buffer entirely, only to obtain at
	least one more byte.  bytes_in_buffer MUST be set to a positive value
	if TRUE is returned.  A FALSE return should only be used when I/O
	suspension is desired (this mode is discussed in the next section). */
	static boolean fill_input_buffer (j_decompress_ptr cinfo);

	/* Skip num_bytes worth of data.  The buffer pointer and count should
	be advanced over num_bytes input bytes, refilling the buffer as
	needed.  This is used to skip over a potentially large amount of
	uninteresting data (such as an APPn marker).  In some applications
	it may be possible to optimize away the reading of the skipped data,
	but it's not clear that being smart is worth much trouble; large
	skips are uncommon.  bytes_in_buffer may be zero on return.
	A zero or negative skip count should be treated as a no-op. */
	static void skip_input_data (j_decompress_ptr cinfo, long num_bytes);

	/* Terminate source --- called by jpeg_finish_decompress() after all
	data has been read.  Often a no-op. */
	static void term_source (j_decompress_ptr cinfo);

};


#endif

