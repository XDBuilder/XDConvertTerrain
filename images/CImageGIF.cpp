#include <stdafx.h>
#include "CImageBMP.h"
#include "CImageGIF.h"

CImageGIF::CImageGIF()
{
	fp=NULL;
	showComment=NULL;
	verbose = 0;
	ZeroDataBlock = 0;
}

CImageGIF::~CImageGIF()
{
	// do something?
}

// returns true if the file maybe is able to be loaded by this class
bool CImageGIF::isALoadableFileExtension(const c8* fileName)
{
	return strstr(fileName, ".gif") != 0;
}

//! returns true if the file maybe is able to be loaded by this class
bool CImageGIF::isALoadableFileFormat(FILE* file)
{
	if (!file)
		return false;

	unsigned char buf[16];
	if (! ReadOK(file,buf,6)) {
//		AfxMessageBox("error reading magic number");
		return false;
	}

	if (strncmp((char *)buf,"GIF",3) != 0){
//		AfxMessageBox("not a GIF file");
		return false;
	}
	return true;
}

//! creates a surface from the file
CImage* CImageGIF::loadImage( char* fileName )
{
	char bmptemp[256];
	sprintf(bmptemp,"c:\\tempbmp.bmp");
	CImage* image=NULL;
	if(gif2bmp(fileName,bmptemp)){
		FILE* fb=fopen(bmptemp,"rb");
		CImageBMP ibmp;
		image=ibmp.loadImage(fb);
		fclose(fb);
	}
	return image;
}
/*
bool CImageJPG::writeImage(char* gifname, CImage* image, u32 param)
{
	if( !image || !jpgname ) return false;

	char bmptemp[256];
	sprintf(bmptemp,"c:\\tempbmp.bmp");
	FILE* fb=fopen(bmptemp,"rb+");
	CImageBMP ibmp;
	if(ibmp.writeImage(fb,image,0)){
		fclose(fb);
		return bmp2jpg(bmptemp,jpgname);
	}
	return false;	
}
*/


///////////////////////////////////////////////////////////////////////////
////Gif2Bmp////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool CImageGIF::gif2bmp(char* gifname,char* bmpname)
{
	FILE*	in;
	int		imageNumber = 1;

	in= fopen(gifname, "rb");
	if(in==0){
		AfxMessageBox("gif file is not exist");
		return FALSE;
	}

	fp= fopen(bmpname, "wb");
	if(fp==0){
		AfxMessageBox("bmp file is not exist");
		return FALSE;
	}
	ReadGIF(in, imageNumber);

	fclose(in);
	fclose(fp);
	return 1;

}



int CImageGIF::dib_wib(int bitcount, int wi)
{
	switch (bitcount){
	case 1: wi= (wi+31) >> 3; break;
	case 4: wi= (wi+7)  >> 1; break;
	case 8: wi=  wi+3; break;
	case 16:wi= (wi*2)+3; break;
	case 24:wi= (wi*3)+3; break;
	case 32:return wi*4;
	}
	return wi & ~3;
}
/* Output .bmp header */
int CImageGIF::spew_header(int wi, int hi, int n, RGBQUAD *cmap)
{
	/* Writes to the global fp */
		 if(n>16) bi.biBitCount= 8;
	else if(n>2 ) bi.biBitCount= 4;
	else          bi.biBitCount= 1;
	n = 1 << ((int)bi.biBitCount);
	bi.biSize= sizeof(bi);
	bi.biWidth= wi;
	bi.biHeight= hi;
	bi.biPlanes= 1;
	bi.biCompression= 0;
	wib = dib_wib((int)bi.biBitCount, (int)bi.biWidth);
	bi.biSizeImage= bi.biHeight * wib;
	bi.biClrUsed= n;
	bi.biClrImportant= n;
	bi.biXPelsPerMeter= 0;
	bi.biYPelsPerMeter= 0;

	hdr.bfType= 0x4d42;	/* BM */
	hdr.bfReserved1= 0;
	hdr.bfReserved2= 0;
	hdr.bfOffBits = sizeof(hdr) + bi.biSize + n*sizeof(RGBQUAD);
	hdr.bfSize= hdr.bfOffBits + bi.biSizeImage;

	fwrite(&hdr,1,sizeof(hdr),fp);
	fwrite(&bi, 1, sizeof(bi), fp);
	if(n>0)
		fwrite(cmap, sizeof(RGBQUAD), n, fp);
	return 1;
}

void CImageGIF::ReadGIF(FILE *fd, int imageNumber)
{
	unsigned char buf[16];
	unsigned char c;
	static RGBQUAD localColorMap[MAXCOLORMAPSIZE];
	int useGlobalColormap;
	int bitPixel;
	int imageCount = 0;
	char version[4];
	CString str;

	if (! ReadOK(fd,buf,6)) {
		AfxMessageBox("error reading magic number");
		return;
	}

	if (strncmp((char *)buf,"GIF",3) != 0){
		AfxMessageBox("not a GIF file");
		return;
	}

	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';

	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)){
		AfxMessageBox("bad version number, not '87a' or '89a'");
		return;
	}

	if (! ReadOK(fd,buf,7)){
		AfxMessageBox("failed to read screen descriptor");
		return;
	}

	GifScreen.Width 	  = LM_to_uint(buf[0],buf[1]);
	GifScreen.Height	  = LM_to_uint(buf[2],buf[3]);
	GifScreen.BitPixel	  = 2<<(buf[4]&0x07);
	GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
	GifScreen.Background	 = buf[5];
	GifScreen.AspectRatio	 = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
		if (ReadColorMap(fd,GifScreen.BitPixel, GifScreen.ColorMap)){
			AfxMessageBox("error reading global colormap");
			return;
		}
	}
	/*
	if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
		float	r;
		r = ( (float) GifScreen.AspectRatio + 15.0 ) / 64.0;
		fprintf(stderr,"warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'\n",
		  r < 1.0 ? 'x' : 'y',
		  r < 1.0 ? 1.0 / r : r );
	}
	*/
	for (;;) {
		if (! ReadOK(fd,&c,1)){
			AfxMessageBox("EOF / read error on image data");
			return;
		}

		if (c == ';') {	 /* GIF terminator */
			if (imageCount < imageNumber){
				str.Format("only %d image%s found in file\n",
													imageCount, imageCount>1?"s":"" );
				AfxMessageBox(str);
				return;
			}
			return;
		}

		if (c == '!') {	 /* Extension */
			if (! ReadOK(fd,&c,1)){
				AfxMessageBox("OF / read error on extention function code");
				return;
			}
			DoExtension(fd, c);
			continue;
		}

		if (c != ',') {	 /* Not a valid start character */
			str.Format("bogus character 0x%02x, ignoring\n", (int)c);
			AfxMessageBox(str);
			continue;
		}

		++imageCount;

		if (! ReadOK(fd,buf,9)){
			AfxMessageBox("couldn't read left/top/width/height");
			return;
		}

		useGlobalColormap = ! (buf[8] & LOCALCOLORMAP);

		bitPixel = 1<<((buf[8]&0x07)+1);

		if (! useGlobalColormap) {
			if (ReadColorMap(fd, bitPixel, localColorMap)){
				AfxMessageBox("error reading local colormap");
				return;
			}
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				localColorMap, bitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		} else {
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				GifScreen.ColorMap, GifScreen.BitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		}

	}
}

int CImageGIF::ReadColorMap(FILE *fd, int number, RGBQUAD *buffer)
{
	int i;
	unsigned char rgb[3];

	for (i = 0; i < number; ++i, buffer++) {
		if (! ReadOK(fd, rgb, sizeof(rgb))){
			AfxMessageBox("bad colormap");
			return -1;
		}
		buffer->rgbRed= rgb[0];
		buffer->rgbGreen= rgb[1];
		buffer->rgbBlue= rgb[2];
		buffer->rgbReserved= 0;
	}
	return 0;
}
int CImageGIF::DoExtension(FILE *fd, int label)
{
	static char	buf[256];
	char		*str;

	switch (label) {
	case 0x01:		/* Plain Text Extension */
		str = "Plain Text Extension";
#ifdef notdef
		if (GetDataBlock(fd, (unsigned char*) buf) == 0);

		lpos	= LM_to_uint(buf[0], buf[1]);
		tpos	= LM_to_uint(buf[2], buf[3]);
		width	= LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw	= buf[8];
		cellh	= buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			PPM_ASSIGN(image[ypos][xpos],
					cmap[CM_RED][v],
					cmap[CM_GREEN][v],
					cmap[CM_BLUE][v]);
			++index;
		}

		return FALSE;
#else
		break;
#endif
	case 0xff:		/* Application Extension */
		str = "Application Extension";
		GetDataBlock(fd, (unsigned char*) buf);
		if (showComment){
			fprintf(stderr, "Application Extension: %c%c%c%c%c%c%c%c ",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
			fprintf(stderr, " Authentication Code=)%02x %02x %02x\n",
				buf[8], buf[9], buf[10]);
		}
		break;
	case 0xfe:		/* Comment Extension */
		str = "Comment Extension";
		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			if (showComment)
				fprintf(stderr,"gif comment: %s\n", buf );
		}
		return FALSE;
	case 0xf9:		/* Graphic Control Extension */
		str = "Graphic Control Extension";
		(void) GetDataBlock(fd, (unsigned char*) buf);
		Gif89.disposal    = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
		Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
		if ((buf[0] & 0x1) != 0)
			Gif89.transparent = buf[3];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0)
			;
		return FALSE;
	default:
		str = buf;
		sprintf(buf, "UNKNOWN (0x%02x)", label);
		break;
	}

	fprintf(stderr,"got a '%s' extension\n", str );

	while (GetDataBlock(fd, (unsigned char*) buf) != 0)
		;

	return FALSE;
}

int CImageGIF::GetDataBlock(FILE *fd, unsigned char  *buf)
{
	unsigned char	count;

	if (! ReadOK(fd,&count,1)) {
		fprintf(stderr,"error in getting DataBlock size\n" );
		return -1;
	}

	ZeroDataBlock = count == 0;

	if ((count != 0) && (! ReadOK(fd, buf, count))) {
		fprintf(stderr,"error in reading DataBlock\n" );
		return -1;
	}

	return count;
}
int CImageGIF::GetCode(FILE *fd, int code_size, int flag)
{
	static unsigned char	buf[280];
	static int		curbit, lastbit, done, last_byte;
	int			i, j, ret;
	unsigned char		count;

	if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( (curbit+code_size) >= lastbit) {
		if (done) {
			if (curbit >= lastbit){
				AfxMessageBox("ran off the end of my bits");
				return -1;
			}
			return -1;
		}
		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		if ((count = GetDataBlock(fd, &buf[2])) == 0)
			done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2+count)*8 ;
	}

	ret = 0;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

	curbit += code_size;

	return ret;
}
int CImageGIF::LWZReadByte(FILE *fd, int flag, int input_code_size)
{
	static int	fresh = FALSE;
	int		code, incode;
	static int	code_size, set_code_size;
	static int	max_code, max_code_size;
	static int	firstcode, oldcode;
	static int	clear_code, end_code;
	static int	table[2][(1<< MAX_LWZ_BITS)];
	static int	stack[(1<<(MAX_LWZ_BITS))*2], *sp;
	register int	i;

	if (flag) {
		set_code_size = input_code_size;
		code_size = set_code_size+1;
		clear_code = 1 << set_code_size ;
		end_code = clear_code + 1;
		max_code_size = 2*clear_code;
		max_code = clear_code+2;

		GetCode(fd, 0, TRUE);

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i) {
			table[0][i] = 0;
			table[1][i] = i;
		}
		for (; i < (1<<MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][0] = 0;

		sp = stack;

		return 0;
	} else if (fresh) {
		fresh = FALSE;
		do {
			firstcode = oldcode =
				GetCode(fd, code_size, FALSE);
		} while (firstcode == clear_code);
		return firstcode;
	}

	if (sp > stack)
		return *--sp;

	while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
			for (i = 0; i < clear_code; ++i) {
				table[0][i] = 0;
				table[1][i] = i;
			}
			for (; i < (1<<MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = 0;
			code_size = set_code_size+1;
			max_code_size = 2*clear_code;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode =
					GetCode(fd, code_size, FALSE);
			return firstcode;
		} else if (code == end_code) {
			int		count;
			unsigned char   buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = GetDataBlock(fd, buf)) > 0)
				;

			if (count != 0)
				fprintf(stderr,"missing EOD in data stream (common occurence)\n");
			return -2;
		}

		incode = code;

		if (code >= max_code) {
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code) {
			*sp++ = table[1][code];
			if (code == table[0][code]){
				AfxMessageBox("circular table entry BIG ERROR");
				return -1;
			}
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1<<MAX_LWZ_BITS))) {
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}
void CImageGIF::ReadImage(FILE *fd, int len, int height, RGBQUAD *cmap,
									 int bpp, int interlace, int ignore)
{
	unsigned char	c;
	int		v;
	int		xpos = 0, ypos = 0, pass = 0;
	unsigned char *scanline;
	CString str;

	/*
	**  Initialize the Compression routines
	*/
	if (! ReadOK(fd,&c,1)){
		AfxMessageBox("EOF / read error on image data");
		return ;
	}

	if (LWZReadByte(fd, TRUE, c) < 0){
		AfxMessageBox("error reading image");
		return ;
	}

	/*
	**  If this is an "uninteresting picture" ignore it.
	*/
	if (ignore) {
		if (verbose)
			fprintf(stderr,"skipping image...\n" );

		while (LWZReadByte(fd, FALSE, c) >= 0)
			;
		return;
	}
	if ((scanline= (unsigned char *)malloc(len)) == NULL){
		AfxMessageBox("couldn't alloc space for image");
		return ;
	}


	if (verbose){
		str.Format("reading %d by %d%s GIF image\n",
							len, height, interlace ? " interlaced" : "" );
		AfxMessageBox(str);
	}

	spew_header(len, height, bpp, cmap);

	/* Fill the whole file with junk */
	for(v= 0; v<height; v++)
		fwrite(scanline, 1, (int)wib, fp);

	while (ypos<height && (v = LWZReadByte(fd,FALSE,c)) >= 0) {
		switch(bi.biBitCount){
		case 1:
			if(v)
				scanline[xpos>>3] |= 128 >> (xpos&7);
			else
				scanline[xpos>>3] &= 0xff7f >> (xpos&7);
			break;
		case 4:
			if(xpos&1)
				scanline[xpos>>1] |= v&15;
			else
				scanline[xpos>>1] = (v&15) << 4;
			break;
		case 8:
			scanline[xpos]= v;
			break;
		}
		++xpos;
		if (xpos == len) {
			fseek(fp, -(ypos+1)*wib, SEEK_END);
			fwrite(scanline, 1, (int)wib, fp);
			xpos = 0;
			if (interlace) {
				static int dpass[]= {8,8,4,2};
				ypos += dpass[pass];
				if (ypos >= height) {
					static int restart[]= {0,4,2,1,32767};
					ypos= restart[++pass];
				}
			} else
				++ypos;
		}
	}
	if(LWZReadByte(fd, FALSE,c) >= 0)
		AfxMessageBox("too much input data, ignoring extra...\n");

}
