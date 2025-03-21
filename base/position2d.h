#ifndef __POSITION_H_INCLUDED__
#define __POSITION_H_INCLUDED__

#include "xdtypes.h"
#include "dimension2d.h"

// Simple class for holding 2d coordinates.
// Not supposed for doing geometric calculations. 
// use vector2d instead for things like that. 

template <class T>
class position2d
{
	public:
		position2d(T px, T py)	: x(x), y(y) {};
		position2d() : x(0), y(0) {};
		position2d(const position2d<T>& other)	: x(other.x), y(other.y) {};
		
		bool operator == (const position2d<T>& other) const
		{
			return x == other.x && y == other.y;
		}
		
		bool operator != (const position2d<T>& other) const
		{
			return x != other.x || y != other.y;
		}
		
		const position2d<T>& operator+=(const position2d<T>& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}
		
		const position2d<T>& operator-=(const position2d<T>& other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		
		const position2d<T>& operator+=(const dimension2d<T>& other)
		{
			x += other.Width;
			y += other.Height;
			return *this;
		}
		
		const position2d<T>& operator-=(const dimension2d<T>& other)
		{
			x -= other.Width;
			y -= other.Height;
			return *this;
		}
		
		position2d<T> operator-(const position2d<T>& other) const
		{
			return position2d<T>(x-other.x, y-other.y);
		}
		
		position2d<T> operator+(const position2d<T>& other) const
		{
			return position2d<T>(x+other.x, y+other.y);
		}
		
		position2d<T> operator*(const position2d<T>& other) const
		{
			return position2d<T>(x*other.x, y*other.y);
		}
		
		position2d<T> operator*(const T& scalar) const
		{
			return position2d<T>(x*scalar, y*scalar);
		}
		
		position2d<T> operator+(const dimension2d<T>& other) const
		{
			return position2d<T>(x+other.Width, y+other.Height);
		}
		
		position2d<T> operator-(const dimension2d<T>& other) const
		{
			return position2d<T>(x-other.Width, y-other.Height);
		}
		
		const position2d<T>& operator=(const position2d<T>& other) 
		{
			x = other.x;
			y = other.y;
			return *this;
		}
		
		T x, y;
};

typedef position2d<f32> position2df;// Typedef for a f32 position.
typedef position2d<s32> position2di;// Typedef for an integer position.


#endif

