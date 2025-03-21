/*
	History:
	- changed behaviour for log2 textures ( replaced multiplies by shift )
*/

#ifndef __S_VIDEO_2_SOFTWARE_HELPER_H_INCLUDED__
#define __S_VIDEO_2_SOFTWARE_HELPER_H_INCLUDED__


#include "../base/xdmath.h"

// ----------------------- Generic ----------------------------------
//! a more useful memset for pixel
inline void memset32 ( void * dest, const u32 value, u32 bytesize )
{
	u32 * d = (u32*) dest;

	u32 i;

	i = bytesize >> ( 2 + 3 );
	while( i )
	{
		d[0] = value;
		d[1] = value;
		d[2] = value;
		d[3] = value;

		d[4] = value;
		d[5] = value;
		d[6] = value;
		d[7] = value;

		d += 8;
		i -= 1;
	}

	i = (bytesize >> 2 ) & 7;
	while( i )
	{
		d[0] = value;
		d += 1;
		i -= 1;
	}

}

/*
	use biased loop counter
	--> 0 byte copy is forbidden
*/
inline void memcpy32_small ( void * dest, const void *source, u32 bytesize )
{
	u32 c = bytesize >> 2;

	do
	{
		((u32*) dest ) [ c + -1 ] = ((u32*) source) [ c + -1 ];
	} while ( --c );

}



// integer log2 of a float ieee 754. TODO: non ieee floating point
static inline s32 s32_log2_f32( f32 f)
{
	u32 x = IR ( f );

	return ((x & 0x7F800000) >> 23) - 127;
}

static inline s32 s32_log2_s32(u32 x)
{
	return s32_log2_f32( (f32) x);
}

static inline s32 s32_abs(s32 x)
{
	s32 b = x >> 31;
	return (x ^ b ) - b;
}


// TODO: don't stick on 32 Bit Pointer
#define PointerAsValue(x) ( (u32) (u32*) (x) ) 


//! conditional set based on mask and arithmetic shift
inline u32 if_mask_a_else_b ( const u32 mask, const u32 a, const u32 b )
{
	return ( mask & ( a ^ b ) ) ^ b;
}

inline void setbits ( u32 &state, s32 condition, u32 mask )
{
	state ^= ( ( -condition >> 31 ) ^ state ) & mask;
}


// ------------------ Video---------------------------------------
/*!
	Pixel = dest * ( 1 - alpha ) + source * alpha
	alpha [0;256]
*/
inline u32 PixelBlend32 ( const u32 c2, const u32 c1, u32 alpha )
{
	u32 srcRB = c1 & 0x00FF00FF;
	u32 srcXG = c1 & 0x0000FF00;

	u32 dstRB = c2 & 0x00FF00FF;
	u32 dstXG = c2 & 0x0000FF00;


	u32 rb = srcRB - dstRB;
	u32 xg = srcXG - dstXG;

	rb *= alpha;
	xg *= alpha;
	rb >>= 8;
	xg >>= 8;

	rb += dstRB;
	xg += dstXG;

	rb &= 0x00FF00FF;
	xg &= 0x0000FF00;

	return rb | xg; 
}

/*!
	Pixel = dest * ( 1 - alpha ) + source * alpha
	alpha [0;32]
*/
inline u16 PixelBlend16 ( const u16 c2, const u32 c1, const u32 alpha )
{
	u32 srcRB = c1 & 0x7C1F;
	u32 srcXG = c1 & 0x03E0;

	u32 dstRB = c2 & 0x7C1F;
	u32 dstXG = c2 & 0x03E0;


	u32 rb = srcRB - dstRB;
	u32 xg = srcXG - dstXG;

	rb *= alpha;
	xg *= alpha;
	rb >>= 5;
	xg >>= 5;

	rb += dstRB;
	xg += dstXG;

	rb &= 0x7C1F;
	xg &= 0x03E0;

	return rb | xg; 
}

/*!
	Scale Color by (1/value)
	value 0 - 256 ( alpha )
*/
inline u32 PixelLerp32 ( const u32 source, const u32 value )
{
	u32 srcRB = source & 0x00FF00FF;
	u32 srcXG = (source & 0xFF00FF00) >> 8;

	srcRB *= value;
	srcXG *= value;

	srcRB >>= 8;
	//srcXG >>= 8;

	srcXG &= 0xFF00FF00;
	srcRB &= 0x00FF00FF;

	return srcRB | srcXG; 
}

/*
	return alpha in [0;256] Granularity
	 add highbit alpha ( alpha > 127 ? + 1 )
*/
inline u32 extractAlpha ( const u32 c )
{
	return ( c >> 24 ) + ( c >> 31 );
}

/*
	Pixel = c0 * (c1/31). c0 Alpha retain
*/
inline u16 PixelMul16 ( const u16 c0, const u16 c1)
{
	return ((( ( (c0 & 0x7C00) * (c1 & 0x7C00) ) & 0x3E000000 ) >> 15 ) |
			(( ( (c0 & 0x03E0) * (c1 & 0x03E0) ) & 0x000F8000 ) >> 10 ) |
			(( ( (c0 & 0x001F) * (c1 & 0x001F) ) & 0x000003E0 ) >> 5 ) |
			(c0 & 0x8000));
}

/*
	Pixel = c0 * (c1/31). 
*/
inline u16 PixelMul16_2 ( u16 c0, u16 c1)
{
	return	( ( (c0 & 0x7C00) * (c1 & 0x7C00) ) & 0x3E000000 ) >> 15 |
			( ( (c0 & 0x03E0) * (c1 & 0x03E0) ) & 0x000F8000 ) >> 10 |
			( ( (c0 & 0x001F) * (c1 & 0x001F) ) & 0x000003E0 ) >> 5  |
			( c0 & c1 & 0x8000);
}

/*
	Pixel = c0 * (c1/255). c0 Alpha Retain
*/
inline u32 PixelMul32 ( const u32 c0, const u32 c1)
{
	return	(c0 & 0xFF000000) |
			(( ( (c0 & 0x00FF0000) >> 12 ) * ( (c1 & 0x00FF0000) >> 12 ) ) & 0x00FF0000 ) |
			(( ( (c0 & 0x0000FF00) * (c1 & 0x0000FF00) ) >> 16 ) & 0x0000FF00 ) |
			(( ( (c0 & 0x000000FF) * (c1 & 0x000000FF) ) >> 8  ) & 0x000000FF);
}

/*
	Pixel = c0 * (c1/255). 
*/
inline u32 PixelMul32_2 ( const u32 c0, const u32 c1)
{
	return	(( ( (c0 & 0xFF000000) >> 16 ) * ( (c1 & 0xFF000000) >> 16 ) ) & 0xFF000000 ) |
			(( ( (c0 & 0x00FF0000) >> 12 ) * ( (c1 & 0x00FF0000) >> 12 ) ) & 0x00FF0000 ) |
			(( ( (c0 & 0x0000FF00) * (c1 & 0x0000FF00) ) >> 16 ) & 0x0000FF00 ) |
			(( ( (c0 & 0x000000FF) * (c1 & 0x000000FF) ) >> 8  ) & 0x000000FF);
}

/*
	Pixel = clamp ( c0 + c1, 0, 255 )
*/
inline u32 PixelAdd32 ( const u32 c2, const u32 c1)
{
	u32 sum = ( c2 & 0x00FFFFFF )  + ( c1 & 0x00FFFFFF );
	u32 low_bits = ( c2 ^ c1 ) & 0x00010101;
	s32 carries  = ( sum - low_bits ) & 0x01010100;
	u32 modulo = sum - carries;
	u32 clamp = carries - ( carries >> 8 );
	return modulo | clamp;
}



// 1 - Bit Alpha Blending
inline u16 PixelBlend16 ( const u16 c2, const u16 c1 )
{
	u16 c = c1 & 0x8000;
	
	c >>= 15;
	c += 0x7fff;
	
	c &= c2;
	c |= c1;
	
	return c;
}

// 1 - Bit Alpha Blending 16Bit SIMD
inline u32 PixelBlend16_simd ( const u32 c2, const u32 c1 )
{
	u32 c = c1 & 0x80008000;
	
	c >>= 15;
	c += 0x7fff7fff;
	
	c &= c2;
	c |= c1;
	
	return c;
}


/*!
	Pixel = dest * ( 1 - SourceAlpha ) + source * SourceAlpha
*/
inline u32 PixelBlend32 ( const u32 c2, const u32 c1 )
{
	// alpha test
	u32 alpha = c1 & 0xFF000000;

	if ( 0 == alpha )
		return c2;

	if ( 0xFF000000 == alpha )
	{
		return c1;
	}

	alpha >>= 24;

	// add highbit alpha, if ( alpha > 127 ) alpha += 1;
	alpha += ( alpha >> 7);

	u32 srcRB = c1 & 0x00FF00FF;
	u32 srcXG = c1 & 0x0000FF00;

	u32 dstRB = c2 & 0x00FF00FF;
	u32 dstXG = c2 & 0x0000FF00;


	u32 rb = srcRB - dstRB;
	u32 xg = srcXG - dstXG;

	rb *= alpha;
	xg *= alpha;
	rb >>= 8;
	xg >>= 8;

	rb += dstRB;
	xg += dstXG;

	rb &= 0x00FF00FF;
	xg &= 0x0000FF00;

	return rb | xg; 
}

// some 2D Defines
struct AbsRectangle
{
	s32 x0;
	s32 y0;
	s32 x1;
	s32 y1;
};

inline void intersect ( AbsRectangle &dest, const AbsRectangle& a, const AbsRectangle& b)
{
	dest.x0 = s32_max( a.x0, b.x0 );
	dest.y0 = s32_max( a.y0, b.y0 );
	dest.x1 = s32_min( a.x1, b.x1 );
	dest.y1 = s32_min( a.y1, b.y1 );
}

inline bool isValid (const AbsRectangle& a)
{
	return a.x0 < a.x1 && a.y0 < a.y1;
}

// some 1D defines
struct sIntervall
{
	s32 start;
	s32 end;
};

// returning intersection width
inline s32 intervall_intersect_test( const sIntervall& a, const sIntervall& b)
{
	return s32_min( a.end, b.end ) - s32_max( a.start, b.start );
}


#endif 

