
#ifndef __FAST_A_TO_F_H_INCLUDED__
#define __FAST_A_TO_F_H_INCLUDED__

#include <stdlib.h>
#include "xdmath.h"


const float fast_atof_table[16] =	{  // we write [16] here instead of [] to work around a swig bug
	0.f,
	0.1f,
	0.01f,
	0.001f,
	0.0001f,
	0.00001f,
	0.000001f,
	0.0000001f,
	0.00000001f,
	0.000000001f,
	0.0000000001f,
	0.00000000001f,
	0.000000000001f,
	0.0000000000001f,
	0.00000000000001f,
	0.000000000000001f
};

// ------------------------------------------------------------------------------------
// Convert a string in decimal format to a number
// ------------------------------------------------------------------------------------
inline unsigned int strtoul10( const char* in, const char** out=0)
{
	unsigned int value = 0;

	bool running = true;
	while ( running )
	{
		if ( *in < '0' || *in > '9' )
			break;

		value = ( value * 10 ) + ( *in - '0' );
		++in;
	}
	if (out)*out = in;
	return value;
}

inline u32 strtol10(const char* in, const char** out=0)
{
	u32 value = 0;

	while ( 1 )
	{
		if ( *in < '0' || *in > '9' )
			break;

		value = ( value * 10 ) + ( *in - '0' );
		++in;
	}
	if (out)
		*out = in;
	return value;
}

// Provides a fast function for converting a string into a float, about 6 times faster than atof in win32.
inline const char* fast_atof_move( const char* c, float& out)
{
	bool inv = false;
	const char *t;
	float f;

	if (*c=='-')
	{
		++c;
		inv = true;
	}

	//f = (float)strtol(c, &t, 10);
	f = (float) strtol10 ( c, &c );

	if (*c == '.')
	{
		++c;

		//float pl = (float)strtol(c, &t, 10);
		float pl = (float) strtol10 ( c, &t );
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == 'e')
		{
			++c;
			//float exp = (float)strtol(c, &t, 10);
			bool einv = (*c=='-');
			if (einv)
				++c;

			float exp = (float)strtol10(c, &c);
			if (einv)
				exp *= -1.0f;

			f *= (float)pow(10.0f, exp);
		}
	}

	if (inv)
		f *= -1.0f;
	
	out = f;
	return c;
}

// ------------------------------------------------------------------------------------
// Special version of the function, providing higher accuracy and safety
// It is mainly used by fast_atof to prevent ugly and unwanted integer overflows.
// ------------------------------------------------------------------------------------
inline double strtoul10_64( const char* in, const char** out=0, unsigned int* max_inout=0)
{
	unsigned int cur = 0;
	double value = 0;

	if ( *in < '0' || *in > '9' ) return 0;
//			throw std::invalid_argument(std::string("The string \"") + in + "\" cannot be converted into a value.");

	bool running = true;
	while ( running )
	{
		if ( *in < '0' || *in > '9' )
			break;

		const double new_value = ( value * 10 ) + ( *in - '0' );
		
		if (new_value < value) return 0;/* numeric overflow, we rely on you */
//			throw std::overflow_error(std::string("Converting the string \"") + in + "\" into a value resulted in overflow.");

		value = new_value;

		++in;
		++cur;

		if (max_inout && *max_inout == cur) {
					
			if (out) { /* skip to end */
				while (*in >= '0' && *in <= '9')
					++in;
				*out = in;
			}

			return value;
		}
	}
	if (out)
		*out = in;

	if (max_inout)
		*max_inout = cur;

	return value;
}

inline float fast_atof(const char* c)
{
	float ret;
	fast_atof_move(c, ret);
	return ret;
}

// ------------------------------------------------------------------------------------
// Convert just one hex digit
// Return value is UINT_MAX if the input character is not a hex digit.
// ------------------------------------------------------------------------------------
inline unsigned int HexDigitToDecimal(char in)
{
	unsigned int out = 0xffffffffUL;//UINT_MAX;
	if (in >= '0' && in <= '9')
		out = in - '0';

	else if (in >= 'a' && in <= 'f')
		out = 10u + in - 'a';

	else if (in >= 'A' && in <= 'F')
		out = 10u + in - 'A';

	// return value is UINT_MAX if the input is not a hex digit
	return out;
}
// ------------------------------------------------------------------------------------
// Convert a hex-encoded octet (2 characters, i.e. df or 1a).
// ------------------------------------------------------------------------------------
inline u8 HexOctetToDecimal(const char* in)
{
	u8 b = HexDigitToDecimal(in[0]);
	b = b << 4;
	u8 a = HexDigitToDecimal(in[1]);
	
	return b+a;
	//return ((u8)HexDigitToDecimal(in[0])<<4)+(u8)HexDigitToDecimal(in[1]);
}

// Number of relevant decimals for floating-point parsing.
#define AI_FAST_ATOF_RELAVANT_DECIMALS 15
template <typename Real>
inline const char* fast_atoreal_move( const char* c, Real& out, bool check_comma = true)
{
	Real f;

	bool inv = (*c=='-');
	if (inv || *c=='+') {
		++c;
	}

	f = static_cast<Real>( strtoul10_64 ( c, &c) );
	if (*c == '.' || (check_comma && c[0] == ',' && c[1] >= '0' && c[1] <= '9')) // allow for commas, too
	{
		++c;

		// NOTE: The original implementation is highly inaccurate here. The precision of a single
		// IEEE 754 float is not high enough, everything behind the 6th digit tends to be more 
		// inaccurate than it would need to be. Casting to double seems to solve the problem.
		// strtol_64 is used to prevent integer overflow.

		// Another fix: this tends to become 0 for long numbers if we don't limit the maximum 
		// number of digits to be read. AI_FAST_ATOF_RELAVANT_DECIMALS can be a value between
		// 1 and 15.
		unsigned int diff = AI_FAST_ATOF_RELAVANT_DECIMALS;
		double pl = static_cast<double>( strtoul10_64 ( c, &c, &diff ));

		pl *= fast_atof_table[diff];
		f += static_cast<Real>( pl );
	}

	// A major 'E' must be allowed. Necessary for proper reading of some DXF files.
	// Thanks to Zhao Lei to point out that this if() must be outside the if (*c == '.' ..)
	if (*c == 'e' || *c == 'E')	{

		++c;
		const bool einv = (*c=='-');
		if (einv || *c=='+') {
			++c;
		}

		// The reason float constants are used here is that we've seen cases where compilers
		// would perform such casts on compile-time constants at runtime, which would be
		// bad considering how frequently fast_atoreal_move<float> is called in Assimp.
		Real exp = static_cast<Real>( strtoul10_64(c, &c) );
		if (einv) {
			exp = -exp;
		}
		f *= pow(static_cast<Real>(10.0), exp);
	}

	if (inv) {
		f = -f;
	}
	out = f;
	return c;
}


#endif

