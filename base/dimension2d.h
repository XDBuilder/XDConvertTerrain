#ifndef __DIMENSION2D_H_INCLUDED__
#define __DIMENSION2D_H_INCLUDED__

#include "xdtypes.h"

//! Specifies a 2 dimensional size.
template <class T>
class dimension2d
{
	public:
		
		dimension2d() : Width(0), Height(0) {};
		
		dimension2d(T width, T height) : Width(width), Height(height) {};
		
		dimension2d(const dimension2d<T>& other)
			: Width(other.Width), Height(other.Height) {};
		
		
		bool operator == (const dimension2d<T>& other) const
		{
			return Width == other.Width && Height == other.Height;
		}
		
		
		bool operator != (const dimension2d<T>& other) const
		{
			return Width != other.Width || Height != other.Height;
		}
		
		const dimension2d<T>& operator=(const dimension2d<T>& other) 
		{
			Width = other.Width;
			Height = other.Height;
			return *this;
		}
		
		dimension2d<T> operator/(T scale) 
		{
			return dimension2d<T>(Width/scale, Height/scale);
		}
		
		dimension2d<T> operator*(T scale) 
		{
			return dimension2d<T>(Width*scale, Height*scale);
		}
		
		T Width, Height;
};

typedef dimension2d<f32> dimension2df;// Typedef for a f32 dimension.
typedef dimension2d<s32> dimension2di;// Typedef for an integer dimension.

#endif

