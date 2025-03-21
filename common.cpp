#include "common.h"
#include <math.h>
#include <fstream>
#include <filesystem>
string stdformat(const char* fmt, ...) {
	int size = 2048;
	char* buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt, vl);
	if (size <= nsize) { //fail delete buffer and try again
		delete[] buffer;
		buffer = 0;
		buffer = new char[nsize + 1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt, vl);
	}
	string ret(buffer);
	va_end(vl);
	delete[] buffer;
	return ret;
}
void getMeshWithImages( int level, rect2dd _imageRect, std::vector<TMIdentify>& meshlist )
{
	int i,j;
	double meshsize = double(36)/pow(2,level);
	//파라미터로 넘어온 이미지 경계의 인덱스 구함
	int ix = (_imageRect.ix + double(180))/meshsize;
	int iy = (_imageRect.iy + double( 90))/meshsize;
	int ax = (_imageRect.ax + double(180))/meshsize+0.5;//jsyun 기존 1임. 
	int ay = (_imageRect.ay + double( 90))/meshsize+0.5;//jsyun 기존 1임. y 방향으로 폴더가 하나 더 생겨서 0.5 처리
	for( i=iy;i<=ay;i++ )
	{
		for( j=ix;j<=ax;j++ )
		{
			TMIdentify ztmesh(level,j,i);
			meshlist.push_back(ztmesh);
		}//for( j:
	}//for( i
}
unsigned long getMeshWithImagesCount( int level, rect2dd _imageRect )
{
	unsigned long ret=0;
	int i,j;
	double meshsize = double(36)/pow(2,level);
	//파라미터로 넘어온 이미지 경계의 인덱스 구함
	int ix = (_imageRect.ix + double(180))/meshsize;
	int iy = (_imageRect.iy + double( 90))/meshsize;
	int ax = (_imageRect.ax + double(180))/meshsize+0.5;//jsyun 기존 1임. 
	int ay = (_imageRect.ay + double( 90))/meshsize+0.5;//jsyun 기존 1임. y 방향으로 폴더가 하나 더 생겨서 0.5 처리
	for( i=iy;i<=ay;i++ )
	{
		for( j=ix;j<=ax;j++ )
		{
			ret++;;
		}//for( j:
	}//for( i
	return ret;
}
void getStringInteger(int value, char* temp) {
	int i = ilog10(value) + 1; 

	int blen = 4;
	if (i > 4) {
		blen = 8 - i;
		snprintf(temp, 9, "%8d%c", value, '\0');
	} else {
		blen = 4 - i;
		snprintf(temp, 5, "%4d%c", value, '\0');
	}

	for (i = 0; i < blen; ++i)
		temp[i] = '0';

}
//void mkdirs(string path) {
//	fs::path dir(path);
//	if(fs::exists(dir)==false) fs::create_directory(dir);
//}
vector<string> split(string input, char delimiter) {
	vector<string> answer;
	istringstream  ss(input);
	string temp;

	while (getline(ss, temp, delimiter)) {
		answer.push_back(temp);
	}

	return answer;
}
void ImageResample(u8* sDIB,int SW,int SH,u8* dDIB,int DW,int DH,int Bits,bool bForce) 
{
	
	int BytesPerPixel = Bits/8;
	int nResultRowBytes = ((((SW * Bits) + 31) & ~31) / 8);//?쟾泥대컯?뒪?쓽 ?븳以꾩쓽 byte?궗?씠利?
	int nResultRowBytes_DW = ((((DW * Bits) + 31) & ~31) / 8);//?쟾泥대컯?뒪?쓽 ?븳以꾩쓽 byte?궗?씠利?

	float xScale = (float)SW / (float)DW;
	float yScale = (float)SH / (float)DH;
//	str.Format("xScale %f,yScale %f",xScale,yScale);
//	AfxMessageBox(str);


// jsyun 20170803 - ?븘?옒?쓽 諛⑹떇?쑝濡? 由ъ궗?씠吏? ?븳?떎. ?썝蹂멸낵 ???寃잛쓽 ?겕湲곗?? 臾닿???븯寃? ?옒?맖. ??꾨━?떚媛? 醫뗭쓬.
	// bilinear interpolation
	if ( !( SW>DW && SH>DH ) || bForce==true)  // ?썝蹂몃낫?떎 ?겢?븣 ?궗?슜?븳?떎.
	{	//1999 Steve McMahon (steve@dogma.demon.co.uk)
//_debug("bilinear interpolation");
	
		float fX,fY;
		long ifX, ifY, ifX1, ifY1, xmax, ymax;
		float ir1, ir2, ig1, ig2, ib1, ib2,ia1,ia2, dx, dy;
		u8 r,g,b,a;
		RGBQUAD rgb1, rgb2, rgb3, rgb4;
		xmax = SW-1;
		ymax = SH-1;
		for(long y=0; y<DH; y++){
//			info.nProgress = (long)(100*y/newy);
//			if (info.nEscape) break;
			fY = y * yScale;
			ifY = (int)fY;
			ifY1 = min(ymax, ifY+1);
			dy = fY - ifY;
			for(long x=0; x<DW; x++){
				fX = x * xScale;
				ifX = (int)fX;
				ifX1 = min(xmax, ifX+1);
				dx = fX - ifX;
				// Interpolate using the four nearest pixels in the source

			//pSource = info.pImage + yy * info.dwEffWidth;
			//pSource = (sDIB+yy*nResultRowBytes);
					u8* iDst;
					iDst = sDIB + ifY*nResultRowBytes + ifX*BytesPerPixel; //(x, y) = (x, y) + (0, 0)
					rgb1.rgbBlue = *iDst++;	rgb1.rgbGreen= *iDst++;	rgb1.rgbRed =*iDst++;
					if(BytesPerPixel==4) rgb1.rgbReserved = *iDst;
					
					iDst = sDIB + ifY*nResultRowBytes + ifX1*BytesPerPixel;//(x + 1, y) = (x, y) + (1, 0)
					rgb2.rgbBlue = *iDst++;	rgb2.rgbGreen= *iDst++;	rgb2.rgbRed =*iDst++;
					if(BytesPerPixel==4) rgb2.rgbReserved = *iDst;

					iDst = sDIB + ifY1*nResultRowBytes + ifX*BytesPerPixel;//(x , y + 1) = (x, y) + (0, 1)
					rgb3.rgbBlue = *iDst++;	rgb3.rgbGreen= *iDst++;	rgb3.rgbRed =*iDst++;
					if(BytesPerPixel==4) rgb3.rgbReserved = *iDst;

					iDst = sDIB + ifY1*nResultRowBytes + ifX1*BytesPerPixel;//(x + 1, y + 1) = (x, y) + (1, 1)
					rgb4.rgbBlue = *iDst++;	rgb4.rgbGreen= *iDst++;	rgb4.rgbRed =*iDst++;
					if(BytesPerPixel==4) rgb4.rgbReserved = *iDst;

				// Interplate in x direction:
				bool bALL1=false;
				bool bALL2=false;
				bool bi1=false;
				bool bi2=false;
				ir1=ir2=ig1=ig2=ib1=ib2=ia1=ia2=0;
				if(rgb1.rgbReserved==0 && rgb3.rgbReserved !=0) {
				//if((rgb1.rgbRed==0 && rgb1.rgbGreen==0 && rgb1.rgbBlue==0) && (rgb3.rgbRed!=0 || rgb3.rgbGreen!=0 || rgb3.rgbBlue!=0) ) {
					ir1 = rgb3.rgbRed   * dy;
					ig1 = rgb3.rgbGreen * dy;
					ib1 = rgb3.rgbBlue  * dy;
					if(BytesPerPixel==4) ia1 = rgb3.rgbReserved  * dy;
					bi1=true;
				}
				else if(rgb1.rgbReserved!=0 && rgb3.rgbReserved ==0) {
				//else if((rgb1.rgbRed!=0 || rgb1.rgbGreen!=0 || rgb1.rgbBlue!=0) && (rgb3.rgbRed==0 && rgb3.rgbGreen==0 && rgb3.rgbBlue==0) ) {
					ir1 = rgb1.rgbRed   * (1 - dy) ;
					ig1 = rgb1.rgbGreen * (1 - dy) ;
					ib1 = rgb1.rgbBlue  * (1 - dy) ;
					if(BytesPerPixel==4) ia1 = rgb1.rgbReserved  * (1 - dy) ;
					bi1=true;
				}
				else if(rgb1.rgbReserved!=0 && rgb3.rgbReserved !=0) {
				//else if((rgb1.rgbRed!=0 || rgb1.rgbGreen!=0 || rgb1.rgbBlue!=0) && (rgb3.rgbRed!=0 || rgb3.rgbGreen!=0 || rgb3.rgbBlue!=0) ) {
					ir1 = rgb1.rgbRed   * (1 - dy) + rgb3.rgbRed   * dy;
					ig1 = rgb1.rgbGreen * (1 - dy) + rgb3.rgbGreen * dy;
					ib1 = rgb1.rgbBlue  * (1 - dy) + rgb3.rgbBlue  * dy;
					if(BytesPerPixel==4) ia1 = rgb1.rgbReserved  * (1 - dy) + rgb3.rgbReserved  * dy;
				}
				else { // ?몮?떎 ?븣?뙆媛? 0 
					bALL1=true;
					bi1=true;
				}

				if(rgb2.rgbReserved==0 && rgb4.rgbReserved !=0) {
				//if((rgb2.rgbRed==0 && rgb2.rgbGreen==0 && rgb2.rgbBlue==0) && (rgb4.rgbRed!=0 || rgb4.rgbGreen!=0 || rgb4.rgbBlue!=0) ) {
					ir2 = rgb4.rgbRed   * dy;
					ig2 = rgb4.rgbGreen * dy;
					ib2 = rgb4.rgbBlue  * dy;
					if(BytesPerPixel==4) ia2 = rgb4.rgbReserved  * dy;
					bi2=true;
				}
				else if(rgb2.rgbReserved!=0 && rgb4.rgbReserved ==0) {
				//else if((rgb2.rgbRed!=0 || rgb2.rgbGreen!=0 || rgb2.rgbBlue!=0) && (rgb4.rgbRed==0 && rgb4.rgbGreen==0 && rgb4.rgbBlue==0) ) {
					ir2 = rgb2.rgbRed   * (1 - dy);
					ig2 = rgb2.rgbGreen * (1 - dy);
					ib2 = rgb2.rgbBlue  * (1 - dy);
					if(BytesPerPixel==4) ia2 = rgb2.rgbReserved  * (1 - dy);
					bi2=true;
				}
				else if(rgb2.rgbReserved!=0 && rgb4.rgbReserved !=0) {
				//else if((rgb2.rgbRed!=0 || rgb2.rgbGreen!=0 || rgb2.rgbBlue!=0) && (rgb4.rgbRed!=0 || rgb4.rgbGreen!=0 || rgb4.rgbBlue!=0) ) {
					ir2 = rgb2.rgbRed   * (1 - dy) + rgb4.rgbRed   * dy;
					ig2 = rgb2.rgbGreen * (1 - dy) + rgb4.rgbGreen * dy;
					ib2 = rgb2.rgbBlue  * (1 - dy) + rgb4.rgbBlue  * dy;
					if(BytesPerPixel==4) ia2 = rgb2.rgbReserved  * (1 - dy) + rgb4.rgbReserved  * dy;
				}
				else {
					bALL2=true;
					bi2=true;
				}
				// Interpolate in y:
				if(bi1==false && bi2==false) {
					r = (u8)(ir1 * (1 - dx) + ir2 * dx);
					g = (u8)(ig1 * (1 - dx) + ig2 * dx);
					b = (u8)(ib1 * (1 - dx) + ib2 * dx);
					if(BytesPerPixel==4) a = (u8)(ia1 * (1 - dx) + ia2 * dx);
				}
				else if(bi1==true && bi2==false) {
					r = (u8)(ir2);
					g = (u8)(ig2);
					b = (u8)(ib2);
					if(BytesPerPixel==4) a = (u8)(ia2);
				}
				else if(bi1==false && bi2==true) {
					r = (u8)(ir1);
					g = (u8)(ig1);
					b = (u8)(ib1);
					if(BytesPerPixel==4) a = (u8)(ia1);
				}
				else {
					r=g=b=a=0;
				}
				
				// Set output
				dDIB[y*nResultRowBytes_DW+x*BytesPerPixel] = b;
				dDIB[y*nResultRowBytes_DW+x*BytesPerPixel+1] = g;
				dDIB[y*nResultRowBytes_DW+x*BytesPerPixel+2] = r;
				if(BytesPerPixel==4) dDIB[y*nResultRowBytes_DW+x*BytesPerPixel+3] = a;
			}
		} 				
	} 
	
	else { // ?썝蹂몃낫?떎 ?옉?쓣 ?븣 ?궗?슜?븳?떎.
	
//_debug("high resolution shrink");
		/////////////////////////////////////////////
		//high resolution shrink, thanks to Henrik Stellmann <henrik.stellmann@volleynet.de>
		/////////////////////////////////////////////
		const long ACCURACY = 1000;
		long i_x,j_y; // index for faValue
		long xx,yy; // coordinates in  source image
		unsigned char* pSource;
		unsigned char* pDest = dDIB;
		long* naAccu  = new long[BytesPerPixel * DW + BytesPerPixel];
		long* naCarry = new long[BytesPerPixel * DW + BytesPerPixel];
		long* naTemp;
		long  nWeightX,nWeightY;
		float fEndX;
		float xScale, yScale;
		xScale = (float)(SW) / (float)DW;
		yScale = (float)(SH) / (float)DH;
		long nScale = (long)(ACCURACY * xScale * yScale);

		memset(naAccu,  0, sizeof(long) * (BytesPerPixel * DW + BytesPerPixel));
		memset(naCarry, 0, sizeof(long) * (BytesPerPixel * DW + BytesPerPixel));

		int u, v = 0; // coordinates in dest image

		float fEndY = yScale - 1.0f;
		unsigned char colorRGB;
		for (yy = 0; yy < SH; yy++){
			//pSource = info.pImage + yy * info.dwEffWidth;
			pSource = (sDIB+yy*nResultRowBytes);
			u = i_x = 0;
			fEndX = xScale - 1.0f;
			//s.Format("fEndX=%f,xScale=%f,yy=%d",fEndX,xScale,yy);
			//AfxMessageBox(s);
			if ((float)yy < fEndY) { // complete source row goes into dest row
				for (xx = 0; xx < SW; xx++){
					if ((float)xx < fEndX){ // complete source pixel goes into dest pixel
						for (j_y = 0; j_y < BytesPerPixel; j_y++)	naAccu[i_x + j_y] += (*pSource++) * ACCURACY;
					} else {       // source pixel is splitted for 2 dest pixels
						nWeightX = (long)(((float)xx - fEndX) * ACCURACY);
						for (j_y = 0; j_y < BytesPerPixel; j_y++){
							naAccu[i_x] += (ACCURACY - nWeightX) * (*pSource);
							naAccu[BytesPerPixel + i_x++] += nWeightX * (*pSource++);
						}
						fEndX += xScale;
						u++;
					}
				}
			} else {       // source row is splitted for 2 dest rows       
				nWeightY = (long)(((float)yy - fEndY) * ACCURACY);
				for (xx = 0; xx < SW; xx++){
					if ((float)xx < fEndX){       // complete source pixel goes into 2 pixel
						for (j_y = 0; j_y < BytesPerPixel; j_y++){
							naAccu[i_x + j_y] += ((ACCURACY - nWeightY) * (*pSource));
							naCarry[i_x + j_y] += nWeightY * (*pSource++);
						}
					} else {       // source pixel is splitted for 4 dest pixels
						nWeightX = (int)(((float)xx - fEndX) * ACCURACY);
						for (j_y = 0; j_y < BytesPerPixel; j_y++) {
							naAccu[i_x] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
							colorRGB = (unsigned char)(naAccu[i_x] / nScale);
							//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
							*pDest++ = colorRGB;
							naCarry[i_x] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
							naAccu[i_x + BytesPerPixel] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
							naCarry[i_x + BytesPerPixel] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
							i_x++;
							pSource++;
						}
						fEndX += xScale;
						u++;
					}
				}
				if (u < DW){ // possibly not completed due to rounding errors
					for (j_y = 0; j_y < BytesPerPixel; j_y++) {
						colorRGB = (unsigned char)(naAccu[i_x++] / nScale);
						//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
						*pDest++ = colorRGB;
					}
				}
				naTemp = naCarry;
				naCarry = naAccu;
				naAccu = naTemp;
				// ?븘?옒?뒗 以묒슂?븿. ?쐞履쎌쓽 memset 怨? 媛숈?? ?겕湲곕줈 ?빐 以섏빞 ?븿.
				memset(naCarry, 0, sizeof(long) * (BytesPerPixel * DW + BytesPerPixel));				// need only to set first pixel zero
				pDest = dDIB + (++v * nResultRowBytes_DW);
				fEndY += yScale;
			}
		}
		if (v < DH){	// possibly not completed due to rounding errors
			for (i_x = 0; i_x < BytesPerPixel * DW; i_x++) {
				colorRGB = (unsigned char)(naAccu[i_x] / nScale);
				//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
				*pDest++ = colorRGB;
			}
		}
		delete [] naAccu;
		delete [] naCarry;
		
		
	}
	
	//Resize( sDIB, SW, SH, dDIB, DW, DH, Bits);
}
void enhance_edges(u8* the_image, u8* out_image, int rows, int cols, int band, int high, int rowBytes)
{
	short enhance_mask[3][3] = {
		  {-1, 0, -1},
		  { 0, 5,  0},
		  {-1, 0, -1} };
	short enhance_mask1[3][3] = {
		  {-1,-1,-1},
		  {-1, 9,-1},
		  {-1,-1,-1} };
	short enhance_mask2[3][3] = {
		  { 0,-1, 0},
		  {-1, 5,-1},
		  { 0,-1, 0} };
	short enhance_mask3[3][3] = {
		  { 1,-2, 1},
		  {-2, 5,-2},
		  { 1,-2, 1} };
	short enhance_mask4[3][3] = { //==> [0][0] ==> -1 , lefttop element
		  {-1, 2,-1},
		  { 2,-4, 2},
		  {-1, 2,-1} };
	int    a, b, i, j, k,
		length, max, new_hi,
		new_lo, width;
	int sum[4];
	max = 255;
	//   if(bits_per_pixel == 4)  max = 16;


			 /* Do convolution over image array */
	for (i = 1; i < rows - 1; i++) {
		// if( (i%10) == 0) printf("%d ", i);
		for (j = 1; j < cols - 1; j++) {
			sum[0] = sum[1] = sum[2] = sum[3] = 0;
			for (k = 0; k < band; k++) { 
				if (k == 3) { //alpha value pass
					out_image[i*rowBytes + j * band + k] = the_image[i*rowBytes + j * band + k];
					continue;
				}
				for (a = -1; a < 2; a++) {
					for (b = -1; b < 2; b++) {
						/*if(i==0 && j == 0) { //left top
							if(a==-1 || b==-1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else if(i==rows-1 && j==cols-1) { //right bottom
							if(a==1 || b==1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else if(i==0) {// 1-st row case
							if(a==-1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else if(i==rows-1) {// last row case
							if(a==1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else if(j==0) {// 1-st col case
							if(b==-1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else if(j==cols-1) {//last col case
							if(b==1) continue;
							sum[k] = sum[k] + the_image[(i+a)*cols*band+(j+b)*band+k] * enhance_mask1[a+1][b+1];
						}
						else */
						sum[k] = sum[k] + the_image[(i + a)*rowBytes + (j + b)*band + k] * enhance_mask2[a + 1][b + 1];
					}
				}
				//printf("%d - %d,", k,sum[k]);
				if (sum[k] < 0)   sum[k] = 0;
				if (sum[k] > max) sum[k] = max;
				if (sum[k] > high)
					out_image[i*rowBytes + j * band + k] = the_image[i*rowBytes + j * band + k];
				else
					out_image[i*rowBytes + j * band + k] = sum[k];//the_image[i*cols*band+j*band+k];
			}//k end
		}  /* ends loop over j */
		//printf("\n");
	}  /* ends loop over i */
	//1-st, last row copy
	//1-st, last col copy
	i = 0;
	memcpy(out_image + i * rowBytes, the_image + i * rowBytes, rowBytes);
	i = rows - 1;
	memcpy(out_image + i * rowBytes, the_image + i * rowBytes, rowBytes);
	for (i = 0; i < rows; i++) {
		j = 0;
		for (k = 0; k < band; k++) out_image[i*rowBytes + j * band + k] = the_image[i*rowBytes + j * band + k];
		j = cols - 1;
		for (k = 0; k < band; k++) out_image[i*rowBytes + j * band + k] = the_image[i*rowBytes + j * band + k];
	}
}  /* ends enhance_edges */

unsigned long long generate_unique_value() {
    struct timeval tv;
    gettimeofday(&tv, NULL);  // 현재 시간을 마이크로초 단위로 가져옴
    unsigned long long unique_value = (unsigned long long)(tv.tv_sec) * 1000000 + tv.tv_usec;
    return unique_value;
}

void Resampleing(float* sDIB, int SW, int SH, float* dDIB, int DW, int DH, char flag)
{
	int nResultRowBytes = SW;
	int nResultRowBytes_DW = DW;

	float xScale = (float)SW / (float)DW;
	float yScale = (float)SH / (float)DH;
	//	str.Format("xScale %f,yScale %f",xScale,yScale);
	//	AfxMessageBox(str);
	//	unsigned char R,G,B;
	if (flag == 0)  // nearest pixel
	{
		long fX, fY;
		for (long y = 0; y < DH; y++) {
			fY = y * yScale;
			for (long x = 0; x < DW; x++) {
				fX = x * xScale;
				dDIB[y*nResultRowBytes_DW + x] = sDIB[fY*nResultRowBytes + fX];
			}
		}
	}
	else if (flag == 1)// bicubic interpolation by Blake L. Carlson <blake-carlson(at)uiowa(dot)edu
	{
		float f_x, f_y, a, b, rr, gg, bb, r1, r2, B;
		int   i_x, i_y, xx, yy;

		for (long y = 0; y < DH; y++) {
			//			info.nProgress = (long)(100*y/newy);
			//			if (info.nEscape) break;
			f_y = (float)y * yScale;
			i_y = (int)floor(f_y);
			a = f_y - (float)floor(f_y);
			for (long x = 0; x < DW; x++) {
				f_x = (float)x * xScale;
				i_x = (int)floor(f_x);
				b = f_x - (float)floor(f_x);

				rr = gg = bb = 0.0F;
				for (int m = -1; m < 3; m++) {
					r1 = b3spline2((float)m - a);
					for (int n = -1; n < 3; n++) {
						r2 = b3spline2(-1.0F*((float)n - b));
						//						xx = i_x+n+2;
						//						yy = i_y+m+2;
						xx = i_x + n;
						yy = i_y + m;
						if (xx < 0) xx = 0;
						if (yy < 0) yy = 0;
						if (xx >= SW) xx = SW - 1;
						if (yy >= SH) yy = SH - 1;

						B = sDIB[yy*nResultRowBytes + xx];
						//						rgb = GetPixelColor(xx,yy);
						bb += B * r1 * r2;
					}
				}
				dDIB[y*nResultRowBytes_DW + x] = bb;
				//				newImage.SetPixelColor(x,y,RGB(rr,gg,bb));
			}
		}
	}
	else {
		// bilinear interpolation
		if (!(SW > DW && SH > DH))
		{	//1999 Steve McMahon (steve@dogma.demon.co.uk)
	//_debug("bilinear interpolation");
			float fX, fY;
			long ifX, ifY, ifX1, ifY1, xmax, ymax;
			float ir1, ir2, dx, dy;
			float r;
			float rgb1, rgb2, rgb3, rgb4;

			xmax = SW - 1;
			ymax = SH - 1;
			for (long y = 0; y < DH; y++) {
				//			info.nProgress = (long)(100*y/newy);
				//			if (info.nEscape) break;
				fY = y * yScale;
				ifY = (int)fY;
				ifY1 = min(ymax, ifY + 1);
				dy = fY - ifY;
				for (long x = 0; x < DW; x++) {
					fX = x * xScale;
					ifX = (int)fX;
					ifX1 = min(xmax, ifX + 1);
					dx = fX - ifX;
					float* iDst;
					iDst = sDIB + ifY * nResultRowBytes + ifX; //(x, y) = (x, y) + (0, 0)
					rgb1 = *iDst++;

					iDst = sDIB + ifY * nResultRowBytes + ifX1;//(x + 1, y) = (x, y) + (1, 0)
					rgb2 = *iDst++;

					iDst = sDIB + ifY1 * nResultRowBytes + ifX;//(x , y + 1) = (x, y) + (0, 1)
					rgb3 = *iDst++;

					iDst = sDIB + ifY1 * nResultRowBytes + ifX1;//(x + 1, y + 1) = (x, y) + (1, 1)
					rgb4 = *iDst++;
					//				}
									// Interplate in x direction:
					ir1 = rgb1 * (1 - dy) + rgb3 * dy;

					ir2 = rgb2 * (1 - dy) + rgb4 * dy;
					// Interpolate in y:
					r = (unsigned char)(ir1 * (1 - dx) + ir2 * dx);

					// Set output
					dDIB[y*nResultRowBytes_DW + x] = r;
				}
			}
		}
		else {
			//_debug("high resolution shrink");
					/////////////////////////////////////////////
					//high resolution shrink, thanks to Henrik Stellmann <henrik.stellmann@volleynet.de>
					/////////////////////////////////////////////
			const long ACCURACY = 1000;
			long i_x, j_y; // index for faValue
			long xx, yy; // coordinates in  source image
			float* pSource;
			float* pDest = dDIB;
			float* naAccu = new float[DW + 1];
			float* naCarry = new float[DW + 1];
			float* naTemp;
			long  nWeightX, nWeightY;
			float fEndX;
			float xScale, yScale;
			xScale = (float)(SW) / (float)DW;
			yScale = (float)(SH) / (float)DH;
			float nScale = (float)(ACCURACY * xScale * yScale);

			memset(naAccu, 0, sizeof(float) * (DW + 1));
			memset(naCarry, 0, sizeof(float) * (DW + 1));

			int u, v = 0; // coordinates in dest image

			float fEndY = yScale - 1.0f;
			float colorRGB;
			for (yy = 0; yy < SH; yy++) {
				//pSource = info.pImage + yy * info.dwEffWidth;
				pSource = (sDIB + yy * nResultRowBytes);
				u = i_x = 0;
				fEndX = xScale - 1.0f;
				//s.Format("fEndX=%f,xScale=%f,yy=%d",fEndX,xScale,yy);
				//AfxMessageBox(s);
				if ((float)yy < fEndY) { // complete source row goes into dest row
					for (xx = 0; xx < SW; xx++) {
						if ((float)xx < fEndX) { // complete source pixel goes into dest pixel
							for (j_y = 0; j_y < 1; j_y++)	naAccu[i_x + j_y] += (*pSource++) * ACCURACY;
						}
						else {       // source pixel is splitted for 2 dest pixels
							nWeightX = (long)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 1; j_y++) {
								naAccu[i_x] += (ACCURACY - nWeightX) * (*pSource);
								naAccu[i_x++] += nWeightX * (*pSource++);
							}
							fEndX += xScale;
							u++;
						}
					}
				}
				else {       // source row is splitted for 2 dest rows       
					nWeightY = (long)(((float)yy - fEndY) * ACCURACY);
					for (xx = 0; xx < SW; xx++) {
						if ((float)xx < fEndX) {       // complete source pixel goes into 2 pixel
							for (j_y = 0; j_y < 1; j_y++) {
								naAccu[i_x + j_y] += ((ACCURACY - nWeightY) * (*pSource));
								naCarry[i_x + j_y] += nWeightY * (*pSource++);
							}
						}
						else {       // source pixel is splitted for 4 dest pixels
							nWeightX = (int)(((float)xx - fEndX) * ACCURACY);
							for (j_y = 0; j_y < 1; j_y++) {
								naAccu[i_x] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
								colorRGB = (float)(naAccu[i_x] / nScale);
								//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
								*pDest++ = colorRGB;
								naCarry[i_x] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
								naAccu[i_x] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
								naCarry[i_x] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
								i_x++;
								pSource++;
							}
							fEndX += xScale;
							u++;
						}
					}
					if (u < DW) { // possibly not completed due to rounding errors
						for (j_y = 0; j_y < 1; j_y++) {
							colorRGB = (float)(naAccu[i_x++] / nScale);
							//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
							*pDest++ = colorRGB;
						}
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					memset(naCarry, 0, sizeof(float) * 1);    // need only to set first pixel zero
					pDest = dDIB + (++v * nResultRowBytes_DW);
					fEndY += yScale;
				}
			}
			if (v < DH) {	// possibly not completed due to rounding errors
				for (i_x = 0; i_x < 1 * DW; i_x++) {
					colorRGB = (float)(naAccu[i_x] / nScale);
					//if(colorRGB-40<0) colorRGB=0; else colorRGB-=40;
					*pDest++ = colorRGB;
				}
			}
			delete[] naAccu;
			delete[] naCarry;
		}
	}

}

string GetCVTFileName(string _str_name) {
	std::string str_result;
	string::size_type n_find;

	str_result.clear();
	n_find = 0;

	str_result = _str_name;
	n_find = str_result.find_last_of("\\");
	str_result = str_result.substr(n_find + 1, str_result.length() - n_find);
	return str_result;
}

string GetCVTPath(string _str_name) {
	std::string str_result;
	string::size_type n_find;

	str_result.clear();
	n_find = 0;

	str_result = _str_name;
	n_find = str_result.find_last_of("\\");
	str_result = str_result.substr(0, n_find);
	return std::move(str_result);
}

string GetCVTFileExtension(string _str_name) {
	std::string str_result;
	string::size_type n_find;

	str_result.clear();
	n_find = 0;

	str_result = _str_name;
	n_find = str_result.find_last_of(".");
	str_result = str_result.substr(n_find + 1, str_result.length() - n_find);
	return str_result;
}
float b3spline2(float x)
{
	float a, b, c, d;

	if ((x + 2.0f) <= 0.0f) a = 0.0f; else a = (float)pow((x + 2.0f), 3.0f);
	if ((x + 1.0f) <= 0.0f) b = 0.0f; else b = (float)pow((x + 1.0f), 3.0f);
	if (x <= 0) c = 0.0f; else c = (float)pow(x, 3.0f);
	if ((x - 1.0f) <= 0.0f) d = 0.0f; else d = (float)pow((x - 1.0f), 3.0f);

	return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));
}