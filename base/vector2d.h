#ifndef __VECTOR_2D_H_INCLUDED__
#define __VECTOR_2D_H_INCLUDED__

#include "xdmath.h"

template <class T>
class vector2d
{
public:
	T x, y;	// member variables=====================================

	vector2d() : x(0), y(0) {};
	vector2d(T nx, T ny) : x(nx), y(ny) {};
	vector2d(const vector2d<T>& other) : x(other.x), y(other.y) {};

	// operators=========================================================
	vector2d<T> operator-() const { return vector2d<T>(-x, -y);   }

	vector2d<T>& operator=(const vector2d<T>& other)	{ x = other.x; y = other.y; return *this; }

	vector2d<T> operator+(const vector2d<T>& other) const { return vector2d<T>(x + other.x, y + other.y);	}
	vector2d<T>& operator+=(const vector2d<T>& other)	{ x+=other.x; y+=other.y; return *this; }

	vector2d<T> operator-(const vector2d<T>& other) const { return vector2d<T>(x - other.x, y - other.y);	}
	vector2d<T>& operator-=(const vector2d<T>& other)	{ x-=other.x; y-=other.y; return *this; }

	vector2d<T> operator*(const vector2d<T>& other) const { return vector2d(x * other.x, y * other.y);	}
	vector2d<T>& operator*=(const vector2d<T>& other)	{ x*=other.x; y*=other.y; return *this; }
	vector2d<T> operator*(const T v) const { return vector2d<T>(x * v, y * v);	}
	vector2d<T>& operator*=(const T v) { x*=v; y*=v; return *this; }

	vector2d<T> operator/(const vector2d<T>& other) const { return vector2d<T>(x / other.x, y / other.y);	}
	vector2d<T>& operator/=(const vector2d<T>& other)	{ x/=other.x; y/=other.y; return *this; }
	vector2d<T> operator/(const T v) const { return vector2d<T>(x / v, y / v);	}
	vector2d<T>& operator/=(const T v) { x/=v; y/=v; return *this; }

	bool operator<=(const vector2d<T>& other) const { return x<=other.x && y<=other.y; }
	bool operator>=(const vector2d<T>& other) const { return x>=other.x && y>=other.y; }

	bool operator<(const vector2d<T>& other) const { return x<other.x && y<other.y; }
	bool operator>(const vector2d<T>& other) const { return x>other.x && y>other.y; }

	bool operator==(const vector2d<T>& other) const { return other.x==x && other.y==y; }
	bool operator!=(const vector2d<T>& other) const { return other.x!=x || other.y!=y; }

	// functions=======================================================================
	// returns if this vector equals the other one, taking floating point rounding errors into account
	bool equals(const vector2d<T>& other) const{
		return equal(x, other.x) &&  equal(y, other.y);
	}

	void set(T nx, T ny) {x=nx; y=ny; }
	void set(const vector2d<T>& p) { x=p.x; y=p.y;}

	
	f64 getLength() const { return sqrt(x*x + y*y); }//벡터 길이
	T getLengthSQ() const { return x*x + y*y; }//길이 제곱 (빠른 계산 속도)
	//Returns the dot product of this vector with another.
	T dotProduct(const vector2d<T>& other) const	{ return x*other.x + y*other.y;	}
	f64 getDistanceFrom(const vector2d<T>& other) const{//특정 점으로부터의 거리
		return vector2d<T>(x - other.x, y - other.y).getLength();
	}
	T getDistanceFromSQ(const vector2d& other) const	{//거리의 제곱
		return vector2d(x - other.x, y - other.y).getLengthSQ();
	}
	void rotateBy(f64 degrees, const vector2d<T>& center){//점을 기준으로 회전//rotates the point around a center by an amount of degrees.
		degrees *= DEGTORAD64;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);

		x -= center.x;
		y -= center.y;

		set(x*cs - y*sn, x*sn + y*cs);

		x += center.x;
		y += center.y;
	}

	vector2d<T>& normalize()//normalizes the vector.
	{
		T l = (T)reciprocal_squareroot ( x*x + y*y );
/*
		f32 l = (f32)getLength();
		if (l == 0)
			return *this;

		l = (f32)1.0 / l;
*/
		x *= l;
		y *= l;
		return *this;
	}

	//벡터의 각도(between 0 and 360.)
	f64 getAngleTrig() const
	{
		if (x == 0.0)
			return y < 0.0 ? 270.0 : 90.0;
		else
		if (y == 0)
			return x < 0.0 ? 180.0 : 0.0;

		if ( y > 0.0)
			if (x > 0.0)
				return atan(y/x) * RADTODEG64;
			else
				return 180.0-atan(y/-x) * RADTODEG64;
		else
			if (x > 0.0)
				return 360.0-atan(-y/x) * RADTODEG64;
			else
				return 180.0+atan(-y/-x) * RADTODEG64;
	} 

	//! Calculates the angle of this vector in grad in the counter trigonometric sense.
	//! \return Returns a value between 0 and 360.
	inline f64 getAngle() const
	{
		if (y == 0.0)  // corrected thanks to a suggestion by Jox
			return x < 0.0 ? 180.0 : 0.0; 
		else if (x == 0.0) 
			return y < 0.0 ? 90.0 : 270.0;

		f64 tmp = y / getLength();
		tmp = atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;

		if (x>0.0 && y>0.0)	return tmp + 270;
		else if (x>0.0 && y<0.0) return tmp + 90;
		else if (x<0.0 && y<0.0) return 90 - tmp;
		else if (x<0.0 && y>0.0) return 270 - tmp;
		return tmp;
	}

	//Calculates the angle between this vector and another one in grad.
	//return Returns a value between 0 and 90.
	inline f64 getAngleWith(const vector2d<T>& b) const
	{
/*		f64 tmp = x*b.x + y*b.y;
		if (tmp == 0.0) return 0.0;
		tmp = tmp / sqrt((x*x + y*y) * (b.x*b.x + b.y*b.y));
		if (tmp < 0.0) tmp = -tmp;
		return atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;
*/
		bool left=false;//기준 벡터에 대해 상대벡터가 오른 쪽에 있는 지 왼쪽에 있는지 검사.
		if(b.x*y-x*b.y<0) left = true;
		//두벡터 사이의 각도 구하기
		double dotProduct =  x*b.x + y*b.y;		
		double vectorsMagnitude = getLength()*b.getLength();
		double angle = acos( dotProduct / vectorsMagnitude );
		if(isnan(angle)) angle=0.0f;
		double turn_angle = angle* RADTODEG64;
		if(left) turn_angle*=(-1);
		return turn_angle;

	}

	//Returns if this vector interpreted as a point is on a line between two other points.
	//It is assumed that the point is on the line. 
	//param begin: Beginning vector to compare between.
	//param end: Ending vector to compare between.
	//return True if this vector is between begin and end.  False if not.
	bool isBetweenPoints(const vector2d<T>& begin, const vector2d<T>& end) const
	{
		T f = (end - begin).getLengthSQ();
		return getDistanceFromSQ(begin) < f && 
			getDistanceFromSQ(end) < f;
	}

	//returns interpolated vector
	//param other: other vector to interpolate between
	//param d: value between 0.0f and 1.0f.
	vector2d<T> getInterpolated(const vector2d<T>& other, f32 d) const
	{
		T inv = (f32) 1.0 - d;
		return vector2d<T>(other.x*inv + x*d, other.y*inv + y*d);
	}

	//Returns interpolated vector. ( quadratic )
	//param other0: other vector to interpolate between
	//param other1: other vector to interpolate between
	//param factor: value between 0.0f and 1.0f. 
	vector2d<T> getInterpolated_quadratic(const vector2d<T>& v2, const vector2d<T>& v3, const T d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const f32 inv = (f32) 1.0 - d;
		const f32 mul0 = inv * inv;
		const f32 mul1 = (f32) 2.0 * d * inv;
		const f32 mul2 = d * d;

		return vector2d( x * mul0 + v2.x * mul1 + v3.x * mul2,
						 y * mul0 + v2.y * mul1 + v3.y * mul2 );
	}
	//sets this vector to the interpolated vector between a and b. 
	void interpolate(const vector2d<T>& a, const vector2d<T>& b, const f32 t)
	{
		x = b.x + ( ( a.x - b.x ) * t );
		y = b.y + ( ( a.y - b.y ) * t );
	}

};
	
typedef vector2d<f32> vector2df;// Typedef for f32 2d vector.
typedef vector2d<f64> vector2dd;// Typedef for f64 2d vector.
typedef vector2d<s32> vector2di;// Typedef for integer 2d vector.

#endif

