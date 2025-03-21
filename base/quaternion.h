
#ifndef __QUATERNION_H_INCLUDED__
#define __QUATERNION_H_INCLUDED__

#include "xdtypes.h"
#include "matrix4.h"
#include "vector3d.h"

// Quaternion class.
class quaternion
{
	public:
		// Constructor
		quaternion();
		quaternion(f32 x, f32 y, f32 z, f32 w);
		quaternion(f32 x, f32 y, f32 z);
		quaternion(const matrix4& mat);

		bool operator==(const quaternion& other) const;
		// assignment operator
		inline quaternion& operator=(const quaternion& other);
		// matrix assignment operator
		inline quaternion& operator=(const matrix4& other);

		// add operator
		quaternion operator+(const quaternion& other) const;

		// multiplication operator
		quaternion operator*(const quaternion& other) const;

		// multiplication operator
		quaternion operator*(f32 s) const;

		// multiplication operator
		quaternion& operator*=(f32 s);

		// multiplication operator
		vector3df operator* (const vector3df& v) const;

		// multiplication operator
		quaternion& operator*=(const quaternion& other);

		// calculates the dot product
		inline f32 getDotProduct(const quaternion& other) const;

		// sets new quaternion
		inline void set(f32 x, f32 y, f32 z, f32 w);

		// sets new quaternion based on euler angles
		inline void set(f32 x, f32 y, f32 z);

		// normalizes the quaternion
		inline quaternion& normalize();

		// Creates a matrix from this quaternion
		matrix4 getMatrix() const;

		// Creates a matrix from this quaternion
		void getMatrix( matrix4 &dest ) const;

		// Creates a matrix from this quaternion
		void getMatrix_transposed( matrix4 &dest ) const;

		// Inverts this quaternion
		void makeInverse();

		// set this quaternion to the result of the inpolation between two quaternions based
		void slerp( quaternion q1, const quaternion q2, f32 interpolate );

		// axis must be unit length
		// The quaternion representing the rotation is
		// q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
		void fromAngleAxis (f32 angle, const vector3df& axis);

		void toEuler(vector3df& euler) const;

		// set quaternion to identity
		void makeIdentity();

		f32 x, y, z, w;
};


// Default Constructor
inline quaternion::quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

// Constructor
inline quaternion::quaternion(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
{
}

// Constructor which converts euler angles to a quaternion
inline quaternion::quaternion(f32 x, f32 y, f32 z)
{
	set(x,y,z);
}

// Constructor which converts a matrix to a quaternion
inline quaternion::quaternion(const matrix4& mat)
{
	(*this) = mat;
}

// equal operator
inline bool quaternion::operator==(const quaternion& other) const
{
	if(x != other.x)
		return false;
	if(y != other.y)
		return false;
	if(z != other.z)
		return false;
	if(w != other.w)
		return false;

	return true;
}

// assignment operator
inline quaternion& quaternion::operator=(const quaternion& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

// matrix assignment operator
inline quaternion& quaternion::operator=(const matrix4& m)
{
	f32 diag = m(0,0) + m(1,1) + m(2,2) + 1;
	f32 scale = 0.0f;

	if( diag > 0.0f )
	{
		scale = sqrtf(diag) * 2.0f; // get scale from diagonal

		// TODO: speed this up
		x = ( m(2,1) - m(1,2)) / scale;
		y = ( m(0,2) - m(2,0)) / scale;
		z = ( m(1,0) - m(0,1)) / scale;
		w = 0.25f * scale;
	}
	else
	{
		if ( m(0,0) > m(1,1) && m(0,0) > m(2,2))
		{
			// 1st element of diag is greatest value
			// find scale according to 1st element, and double it
			scale = sqrtf( 1.0f + m(0,0) - m(1,1) - m(2,2)) * 2.0f;

			// TODO: speed this up
			x = 0.25f * scale;
			y = (m(0,1) + m(1,0)) / scale;
			z = (m(2,0) + m(0,2)) / scale;
			w = (m(2,1) - m(1,2)) / scale;
		}
		else if ( m(1,1) > m(2,2))
		{
			// 2nd element of diag is greatest value
			// find scale according to 2nd element, and double it
			scale = sqrtf( 1.0f + m(1,1) - m(0,0) - m(2,2)) * 2.0f;

			// TODO: speed this up
			x = (m(0,1) + m(1,0) ) / scale;
			y = 0.25f * scale;
			z = (m(1,2) + m(2,1) ) / scale;
			w = (m(0,2) - m(2,0) ) / scale;
		}
		else
		{
			// 3rd element of diag is greatest value
			// find scale according to 3rd element, and double it
			scale  = sqrtf( 1.0f + m(2,2) - m(0,0) - m(1,1)) * 2.0f;

			// TODO: speed this up
			x = (m(0,2) + m(2,0)) / scale;
			y = (m(1,2) + m(2,1)) / scale;
			z = 0.25f * scale;
			w = (m(1,0) - m(0,1)) / scale;
		}
	}

	normalize();
	return *this;
}


// multiplication operator
inline quaternion quaternion::operator*(const quaternion& other) const
{
	quaternion tmp;

	tmp.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	tmp.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	tmp.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	tmp.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);

	return tmp;
}


// multiplication operator
inline quaternion quaternion::operator*(f32 s) const
{
	return quaternion(s*x, s*y, s*z, s*w);
}

// multiplication operator
inline quaternion& quaternion::operator*=(f32 s)
{
	x *= s; y*=s; z*=s; w*=s;
	return *this;
}

// multiplication operator
inline quaternion& quaternion::operator*=(const quaternion& other)
{
	*this = other * (*this);
	return *this;
}

// add operator
inline quaternion quaternion::operator+(const quaternion& b) const
{
	return quaternion(x+b.x, y+b.y, z+b.z, w+b.w);
}


// Creates a matrix from this quaternion
inline matrix4 quaternion::getMatrix() const
{
	matrix4 m;

	m(0,0) = 1.0f - 2.0f*y*y - 2.0f*z*z;
	m(1,0) = 2.0f*x*y + 2.0f*z*w;
	m(2,0) = 2.0f*x*z - 2.0f*y*w;
	m(3,0) = 0.0f;

	m(0,1) = 2.0f*x*y - 2.0f*z*w;
	m(1,1) = 1.0f - 2.0f*x*x - 2.0f*z*z;
	m(2,1) = 2.0f*z*y + 2.0f*x*w;
	m(3,1) = 0.0f;

	m(0,2) = 2.0f*x*z + 2.0f*y*w;
	m(1,2) = 2.0f*z*y - 2.0f*x*w;
	m(2,2) = 1.0f - 2.0f*x*x - 2.0f*y*y;
	m(3,2) = 0.0f;

	m(0,3) = 0.0f;
	m(1,3) = 0.0f;
	m(2,3) = 0.0f;
	m(3,3) = 1.0f;

	return m;
}


// Creates a matrix from this quaternion
inline void quaternion::getMatrix( matrix4 &dest ) const
{
	dest[0] = 1.0f - 2.0f*y*y - 2.0f*z*z;
	dest[1] = 2.0f*x*y + 2.0f*z*w;
	dest[2] = 2.0f*x*z - 2.0f*y*w;
	dest[3] = 0.0f;

	dest[4] = 2.0f*x*y - 2.0f*z*w;
	dest[5] = 1.0f - 2.0f*x*x - 2.0f*z*z;
	dest[6] = 2.0f*z*y + 2.0f*x*w;
	dest[7] = 0.0f;

	dest[8] = 2.0f*x*z + 2.0f*y*w;
	dest[9] = 2.0f*z*y - 2.0f*x*w;
	dest[10] = 1.0f - 2.0f*x*x - 2.0f*y*y;
	dest[11] = 0.0f;

	dest[12] = 0.f;
	dest[13] = 0.f;
	dest[14] = 0.f;
	dest[15] = 1.f;
}

// Creates a matrix from this quaternion
inline void quaternion::getMatrix_transposed( matrix4 &dest ) const
{
	dest[0] = 1.0f - 2.0f*y*y - 2.0f*z*z;
	dest[4] = 2.0f*x*y + 2.0f*z*w;
	dest[8] = 2.0f*x*z - 2.0f*y*w;
	dest[12] = 0.0f;

	dest[1] = 2.0f*x*y - 2.0f*z*w;
	dest[5] = 1.0f - 2.0f*x*x - 2.0f*z*z;
	dest[9] = 2.0f*z*y + 2.0f*x*w;
	dest[13] = 0.0f;

	dest[2] = 2.0f*x*z + 2.0f*y*w;
	dest[6] = 2.0f*z*y - 2.0f*x*w;
	dest[10] = 1.0f - 2.0f*x*x - 2.0f*y*y;
	dest[14] = 0.0f;

	dest[3] = 0.f;
	dest[7] = 0.f;
	dest[11] = 0.f;
	dest[15] = 1.f;
}


// Inverts this quaternion
inline void quaternion::makeInverse()
{
	x = -x; y = -y; z = -z;
}

// sets new quaternion
inline void quaternion::set(f32 x, f32 y, f32 z, f32 w)
{
	x = x;
	y = y;
	z = z;
	w = w;
}


// sets new quaternion based on euler angles
inline void quaternion::set(f32 x, f32 y, f32 z)
{
	f64 angle;

	angle = x * 0.5;
	f64 sr = (f32)sin(angle);
	f64 cr = (f32)cos(angle);

	angle = y * 0.5;
	f64 sp = (f32)sin(angle);
	f64 cp = (f32)cos(angle);

	angle = z * 0.5;
	f64 sy = (f32)sin(angle);
	f64 cy = (f32)cos(angle);

	f64 cpcy = cp * cy;
	f64 spcy = sp * cy;
	f64 cpsy = cp * sy;
	f64 spsy = sp * sy;

	x = (f32)(sr * cpcy - cr * spsy);
	y = (f32)(cr * spcy + sr * cpsy);
	z = (f32)(cr * cpsy - sr * spcy);
	w = (f32)(cr * cpcy + sr * spsy);

	normalize();
}

// normalizes the quaternion
inline quaternion& quaternion::normalize()
{
	f32 n = x*x + y*y + z*z + w*w;

	if (n == 1)
		return *this;

	//n = 1.0f / sqrtf(n);
	n = reciprocal_squareroot ( n );
	x *= n;
	y *= n;
	z *= n;
	w *= n;

	return *this;
}


// set this quaternion to the result of the inpolation between two quaternions based
inline void quaternion::slerp( quaternion q1, quaternion q2, f32 time)
{
	f32 angle = q1.getDotProduct(q2);

	if (angle < 0.0f)
	{
		q1 *= -1.0f;
		angle *= -1.0f;
	}

	f32 scale;
	f32 invscale;

	if ((angle + 1.0f) > 0.05f)
	{
		if ((1.0f - angle) >= 0.05f)  // spherical interpolation
		{
			f32 theta = (f32)acos(angle);
			f32 invsintheta = 1.0f / (f32)sin(theta);
			scale = (f32)sin(theta * (1.0f-time)) * invsintheta;
			invscale = (f32)sin(theta * time) * invsintheta;
		}
		else // linear interploation
		{
			scale = 1.0f - time;
			invscale = time;
		}
	}
	else
	{
		q2 = quaternion(-q1.y, q1.x, -q1.w, q1.z);
		scale = (f32)sin(PI * (0.5f - time));
		invscale = (f32)sin(PI * time);
	}

	*this = (q1*scale) + (q2*invscale);
}


// calculates the dot product
inline f32 quaternion::getDotProduct(const quaternion& q2) const
{
	return (x * q2.x) + (y * q2.y) + (z * q2.z) + (w * q2.w);
}


inline void quaternion::fromAngleAxis(f32 angle, const vector3df& axis)
{
	f32 fHalfAngle = 0.5f*angle;
	f32 fSin = (f32)sin(fHalfAngle);
	w = (f32)cos(fHalfAngle);
	x = fSin*axis.x;
	y = fSin*axis.y;
	z = fSin*axis.z;
}

inline void quaternion::toEuler(vector3df& euler) const
{
	double sqw = w*w;
	double sqx = x*x;
	double sqy = y*y;
	double sqz = z*z;

	// heading = rotation about z-axis
	euler.z = (f32) (atan2(2.0 * (x*y +z*w),(sqx - sqy - sqz + sqw)));

	// bank = rotation about x-axis
	euler.x = (f32) (atan2(2.0 * (y*z +x*w),(-sqx - sqy + sqz + sqw)));

	// attitude = rotation about y-axis
	euler.y = (f32) (asin(-2.0 * (x*z - y*w)));
}

inline vector3df quaternion::operator* (const vector3df& v) const
{
	// nVidia SDK implementation
	vector3df uv, uuv;
	vector3df qvec(x, y, z);
	uv = qvec.crossProduct(v);
	uuv = qvec.crossProduct(uv);
	uv *= (2.0f * w);
	uuv *= 2.0f;

	return v + uv + uuv;
}

// set quaterion to identity
inline void quaternion::makeIdentity()
{
	w = 1.f;
	x = 0.f;
	y = 0.f;
	z = 0.f;
}


#endif

