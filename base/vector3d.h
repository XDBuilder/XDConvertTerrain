#ifndef __VECTOR_3D_H_INCLUDED__
#define __VECTOR_3D_H_INCLUDED__

#include "xdmath.h"
#include <float.h>

template <class T>
class vector3d  
{
public:
	T x, y, z; // member variables==========================================
	
	vector3d() {}; //fast than
//	vector3d() : x(0), y(0), z(0) {};
	
	vector3d(T nx, T ny, T nz) : x(nx), y(ny), z(nz) {};
	vector3d(const vector3d<T>& other) : x(other.x), y(other.y), z(other.z) {};
	
	// operators==========================================================
	vector3d<T> operator-() const { return vector3d<T>(-x, -y, -z);   }
	
	vector3d<T>& operator=(const vector3d<T>& other)	{ x = other.x; y = other.y; z = other.z; return *this; }
	
	vector3d<T> operator+(const vector3d<T>& other) const { return vector3d<T>(x + other.x, y + other.y, z + other.z);	}
	vector3d<T>& operator+=(const vector3d<T>& other)	{ x+=other.x; y+=other.y; z+=other.z; return *this; }
	
	vector3d<T> operator-(const vector3d<T>& other) const { return vector3d<T>(x - other.x, y - other.y, z - other.z);	}
	vector3d<T>& operator-=(const vector3d<T>& other)	{ x-=other.x; y-=other.y; z-=other.z; return *this; }
	
	vector3d<T> operator*(const vector3d<T>& other) const { return vector3d<T>(x * other.x, y * other.y, z * other.z);	}
	vector3d<T>& operator*=(const vector3d<T>& other)	{ x*=other.x; y*=other.y; z*=other.z; return *this; }
	vector3d<T> operator*(const T v) const { return vector3d<T>(x * v, y * v, z * v);	}
	vector3d<T>& operator*=(const T v) { x*=v; y*=v; z*=v; return *this; }
	
	vector3d<T> operator/(const vector3d<T>& other) const { return vector3d<T>(x / other.x, y / other.y, z / other.z);	}
	vector3d<T>& operator/=(const vector3d<T>& other)	{ x/=other.x; y/=other.y; z/=other.z; return *this; }
	vector3d<T> operator/(const T v) const { T i=(T)1.0/v; return vector3d<T>(x * i, y * i, z * i);	}
	vector3d<T>& operator/=(const T v) { T i=(T)1.0/v; x*=i; y*=i; z*=i; return *this; }
	
	bool operator<=(const vector3d<T>& other) const { return x<=other.x && y<=other.y && z<=other.z;};
	bool operator>=(const vector3d<T>& other) const { return x>=other.x && y>=other.y && z>=other.z;};
	bool operator<(const vector3d<T>& other) const { return x<other.x && y<other.y && z<other.z;};
	bool operator>(const vector3d<T>& other) const { return x>other.x && y>other.y && z>other.z;};
	
	//! use week float compare
	//bool operator==(const vector3d& other) const { return other.x==x && other.y==y && other.z==z; }
	//bool operator!=(const vector3d& other) const { return other.x!=x || other.y!=y || other.z!=z; }

	bool operator==(const vector3d<T>& other) const {//round error
		return equal64(x, other.x) && equal64(y, other.y) && equal64(z, other.z);
	}
	bool operator!=(const vector3d<T>& other) const {
		return !equal(x, other.x) || !equal(y, other.y) || !equal(z, other.z);
	}
	
	// functions========================================================================
	//returns if this vector equals the other one, taking floating point rounding errors into account
	bool equals(const vector3d<T>& other, const f32 tolerance = ROUNDING_ERROR_32 ) const
	{
		return equal(x, other.x, tolerance) &&
			   equal(y, other.y, tolerance) &&
			   equal(z, other.z, tolerance);
	}
	
	void set(const T nx, const T ny, const T nz) { x=nx; y=ny; z=nz; }
	void set(const vector3d<T>& p) { x=p.x; y=p.y; z=p.z;}
	
	T getLength() const { return (T) sqrt(x*x + y*y + z*z); }//벡터길이
	T getLengthSQ() const { return x*x + y*y + z*z; }//벡터 길이 제곱
	T getXZLengthSQ() const { return x*x + z*z; }//벡터 길이 제곱
	
	//Returns the dot product with another vector.
	T dotProduct(const vector3d<T>& other) const	{ 
		return x*other.x + y*other.y + z*other.z;
	}
	//Returns distance from another point.
	//Here, the vector is interpreted as point in 3 dimensional space. 
	f64 getDistanceFrom(const vector3d<T>& other) const
	{
		return vector3d(x - other.x, y - other.y, z - other.z).getLength();
	}
	//Returns squared distance from another point. 
	//Here, the vector is interpreted as point in 3 dimensional space. */
	T getDistanceFromSQ(const vector3d<T>& other) const
	{
		return vector3d<T>(x - other.x, y - other.y, z - other.z).getLengthSQ();
	}
	
	//Calculates the cross product with another vector
	//Return Crossproduct of this vector with p.
	vector3d<T> crossProduct(const vector3d<T>& p) const
	{
		return vector3d<T>(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
	}
	
	//Returns if this vector interpreted as a point is on a line between two other points.
	//It is assumed that the point is on the line. 다른 두점 선상에 있는지 검사
	bool isBetweenPoints(const vector3d<T>& begin, const vector3d<T>& end) const
	{
		T f = (end - begin).getLengthSQ();
		return (getDistanceFromSQ(begin) < f && getDistanceFromSQ(end) < f);
	}
	
	//Normalizes the vector.
	//Todo: 64 Bit template doesnt work.. need specialized template
	vector3d<T>& normalize()
	{
		T l = (T)reciprocal_squareroot ( x*x + y*y + z*z );
		
		x *= l;
		y *= l;
		z *= l;
		return *this;
		
		/*
		f32 l = (f32)getLength();
		if (l == 0)
		return *this;
		
		  l = (f32)1.0 / l;
		  x *= l;
		  y *= l;
		  z *= l;
		  return *this;
		*/
	}
	
	//Sets the length of the vector to a new value
	void setLength(T newlength)
	{
		normalize();
		*this *= newlength;
	}
	void invert(){//Inverts the vector.
		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;
	}
	
	//Rotates the vector by a specified number of degrees around the y 
	//axis and the specified center.
	//param degrees: Number of degrees to rotate around the y axis.
	//param center: The center of the rotation.
	void rotateXZBy(f64 degrees, const vector3d<T>& center)//y축을 중심으로 회전
	{
		degrees *= DEGTORAD64;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);
		x -= center.x;
		z -= center.z;
		set(x*cs - z*sn, y, x*sn + z*cs);
		x += center.x;
		z += center.z;
	}

	//Rotates the vector by a specified number of degrees around the z 
	//axis and the specified center.
	//param degrees: Number of degrees to rotate around the z axis.
	//param center: The center of the rotation.
	void rotateXYBy(f64 degrees, const vector3d<T>& center)//z축을 중심으로 회전
	{
		degrees *= DEGTORAD64;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);
		x -= center.x;
		y -= center.y;
		set(x*cs - y*sn, x*sn + y*cs, z);
		x += center.x;
		y += center.y;
	}
	
	//Rotates the vector by a specified number of degrees around the x
	//axis and the specified center.
	//param degrees: Number of degrees to rotate around the x axis.
	//param center: The center of the rotation.
	void rotateYZBy(f64 degrees, const vector3d<T>& center)//x축을 중심으로 회전
	{
		degrees *= DEGTORAD64;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);
		z -= center.z;
		y -= center.y;
		set(x, y*cs - z*sn, y*sn + z*cs);
		z += center.z;
		y += center.y;
	}
	
	//Returns interpolated vector.
	//param other: other vector to interpolate between
	//param d: value between 0.0f and 1.0f. 
	vector3d<T> getInterpolated(const vector3d<T>& other, const T d) const
	{
		const T inv = (T) 1.0 - d;
		return vector3d<T>(other.x*inv + x*d, other.y*inv + y*d, other.z*inv + z*d);
	}
	
	//Returns interpolated vector. ( quadratic )
	//param other0: other vector to interpolate between
	//param other1: other vector to interpolate between
	//aram factor: value between 0.0f and 1.0f. */
	vector3d<T> getInterpolated_quadratic(const vector3d<T>& v2, const vector3d<T>& v3, const T d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const T inv = (T) 1.0 - d;
		const T mul0 = inv * inv;
		const T mul1 = (T) 2.0 * d * inv;
		const T mul2 = d * d;
		
		return vector3d ( x * mul0 + v2.x * mul1 + v3.x * mul2,
						  y * mul0 + v2.y * mul1 + v3.y * mul2,
						  z * mul0 + v2.z * mul1 + v3.z * mul2);
	}
	
	//Gets the y and z rotations of a vector.
	//return A vector representing the rotation in degrees of this vector. 
	//The z component of the vector will always be 0. 
	vector3d<T> getHorizontalAngle()
	{
		vector3d<T> angle;
		
		angle.y = (T)atan2(x, z); 
		angle.y *= (T)RADTODEG64;
		
		if (angle.y < 0.0f) angle.y += 360.0f; 
		if (angle.y >= 360.0f) angle.y -= 360.0f; 
		
		f32 z1 = (f32)sqrt(x*x + z*z); 
		
		angle.x = (T)atan2(z1, y); 
		angle.x *= (T)RADTODEG64;
		angle.x -= 90.0f; 
		
		if (angle.x < 0.0f) angle.x += 360.0f; 
		if (angle.x >= 360.0f) angle.x -= 360.0f; 
		
		return angle;
	}

	f64 getAngleWith(const vector3d<T>& other)//두벡터 사이의 각도 구하기
	{							
		T dot = dotProduct( other );		
		f64 vectorsMagnitude = getLength() * other.getLength();
		f64 angle = acos( dot / vectorsMagnitude );
		if(isnan(angle)) return 0;
		angle=(angle*RADTODEG64);
		return  angle;
	}

	//Fills an array of 4 values with the vector data (usually floats).
	//Useful for setting in shader constants for example. The fourth value will always be 0. 
	void getAs4Values(f32* array) const
	{
		array[0] = x;
		array[1] = y;
		array[2] = z;
		array[3] = 0;
	}
	
};
	
typedef vector3d<f32> vector3df;// Typedef for f32 3d vector.
typedef vector3d<f64> vector3dd;// Typedef for f64 3d vector.
typedef vector3d<s32> vector3di;// Typedef for integer 3d vector.

#endif

