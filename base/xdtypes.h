#ifndef __XD_TYPE_H_INCLUDED__
#define __XD_TYPE_H_INCLUDED__

typedef unsigned char		u8;		// 8 bit unsigned variable.
typedef signed char			s8;		// 8 bit signed variable.
typedef char				c8;		// 8 bit character variable.
typedef unsigned short		u16;	// 16 bit unsigned variable.
typedef signed short		s16;	// 16 bit signed variable.
typedef unsigned int		u32;	// 32 bit unsigned variable.
typedef signed int			s32;	// 32 bit signed variable.
typedef float				f32;	// 32 bit floating point variable.
typedef double				f64;	// 64 bit floating point variable.
// This is a typedef for __int64, it ensures portability of the engine. 
// This type is currently not used by the engine and not supported by compilers
// other than Microsoft Compilers, so it is outcommented.
//typedef __int64				s64; // 64 bit signed variable.

//swapbyte=============================================================================
#define bswap_16(X) ((((X)&0xFF) << 8) | (((X)&=0xFF00) >> 8))
#define bswap_32(X) ( (((X)&0x000000FF)<<24) | (((X)&0xFF000000) >> 24) | (((X)&0x0000FF00) << 8) | (((X) &0x00FF0000) >> 8))

static u16 byteswap(u16 num) {return bswap_16(num);}
static s16 byteswap(s16 num) {return bswap_16(num);}
static u32 byteswap(u32 num) {return bswap_32(num);}
static s32 byteswap(s32 num) {return bswap_32(num);}
static f32 byteswap(f32 num) {u32 tmp=bswap_32(*((u32*)&num)); return *((f32*)&tmp);}


#include <wchar.h>
#ifdef __WINDOWS_API_
//! Defines for s{w,n}printf because these methods do not match the ISO C
//! standard on Windows platforms, but it does on all others.
#define swprintf _snwprintf
#define snprintf _snprintf

// define the wchar_t type if not already built in.
#ifdef _MSC_VER 
#ifndef _WCHAR_T_DEFINED
//! A 16 bit wide character type.
/**
	Defines the wchar_t-type.
	In VS6, its not possible to tell
	the standard compiler to treat wchar_t as a built-in type, and 
	sometimes we just don't want to include the huge stdlib.h or wchar.h,
	so we'll use this.
*/
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif // wchar is not defined
#endif // microsoft compiler
#endif // __WINDOWS_API_

//! define a break macro for debugging.
#if defined(_DEBUG)
#if defined(__WINDOWS_API_) && defined(_MSC_VER)
#define __DEBUG_BREAK_IF( _CONDITION_ ) if (_CONDITION_) {_asm int 3}
#else 
#include "assert.h"
#define __DEBUG_BREAK_IF( _CONDITION_ ) assert( !(_CONDITION_) );
#endif
#else 
#define __DEBUG_BREAK_IF( _CONDITION_ )
#endif

// memory debugging
#if defined(_DEBUG) && defined(_MSC_VER) && (_MSC_VER > 1299) 
	
	#define CRTDBG_MAP_ALLOC
	#define _CRTDBG_MAP_ALLOC
	#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
	#include <stdlib.h>
	#include <crtdbg.h>
//	#define new DEBUG_CLIENTBLOCK
	#define new DEBUG_NEW
#endif 

// disable truncated debug information warning in visual studio 6 by default
#if defined(_MSC_VER) && (_MSC_VER < 1300 )
#pragma warning( disable: 4786)
#endif // _MSC


//! ignore VC8 warning deprecated
/** The microsoft compiler */
#if defined(__WINDOWS_API_) && defined(_MSC_VER) && (_MSC_VER >= 1400)
	//#pragma warning( disable: 4996)
	//#define _CRT_SECURE_NO_DEPRECATE 1
	//#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif


// creates four CC codes used  for simple ids
// some compilers can create those by directly writing the code like 'code', but some generate warnings so we use this macro here 
#define MAKE_XD_ID(c0, c1, c2, c3) ((u32)(u8)(c0) | ((u32)(u8)(c1) << 8) | ((u32)(u8)(c2) << 16) | ((u32)(u8)(c3) << 24 ))
#define MAKE_COLOR(r, g, b, a) ((u32)(u8)(r) | ((u32)(u8)(g) << 8) | ((u32)(u8)(b) << 16) | ((u32)(u8)(a) << 24 ))

#endif