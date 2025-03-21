#ifndef __MATRIX_H_INCLUDED__
#define __MATRIX_H_INCLUDED__
#include <stdio.h>
#include <string.h>
#include <vector>
#include "aabbox3d.h"
#include "xdtypes.h"
#include "vector3d.h"
#include "vector2d.h"
#include "plane3d.h"

//#include "rect.h"

//4x4 matrix. Mostly used as transformation matrix for 3d calculations.
//Matrix4 is mainly used by the xd engine for doing transformations.
//The matrix is a D3D style matrix, row major with translations in the 4th row.
class matrix4
{
public:
	//Matrix data, stored in row-major order
	f32 M[16];
	bool definitelyIdentityMatrix;
	/*				 directx version
	0  1  2  3      _11 _12 _13 _14
	4  5  6  7		_21 _22 _23 _24
	8  9  10 11		_31 _32 _33 _34
	12 13 14 15		_41 _42 _43 _44	
	*/
public:
	//Constructor Flags
	enum eConstructor
	{
		EM4CONST_NOTHING = 0,
		EM4CONST_COPY,
		EM4CONST_IDENTITY,
		EM4CONST_TRANSPOSED,
		EM4CONST_INVERSE,
		EM4CONST_INVERSE_TRANSPOSED
	};
	
	matrix4( eConstructor constructor = EM4CONST_IDENTITY );
	matrix4( const matrix4& other,eConstructor constructor = EM4CONST_COPY);
	matrix4(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44);
	
	//행렬의 element를 행,열 방식으로 접근할 수 있게 하는 오퍼레이터.
	f32& operator()(const s32 row, const s32 col) { definitelyIdentityMatrix=false; return M[ row * 4 + col ]; }
	const f32& operator()(const s32 row, const s32 col) const { return M[row * 4 + col]; }
	//행렬의 element를 선형적으로 접근할 수 있게 하는 오퍼레이터.
	f32& operator[](u32 index) { definitelyIdentityMatrix=false; return M[index]; }
	const f32& operator[](u32 index) const { return M[index]; }
	
	inline matrix4& operator=(const matrix4 &other); //행렬 대입
	
	const f32* pointer() const { return M; } //행렬 배열의 포인터를 반환
	f32* pointer() { definitelyIdentityMatrix=false; return M; }
	
	bool operator==(const matrix4 &other) const;
	bool operator!=(const matrix4 &other) const;
	
	matrix4 operator+(const matrix4& other) const;//행렬 더하기
	matrix4& operator+=(const matrix4& other);
	
	matrix4 operator-(const matrix4& other) const;//빼기
	matrix4& operator-=(const matrix4& other);
	
	// set this matrix to the product of two matrices 두 행렬 곱하기===============
	inline void setbyproduct(const matrix4& other_a,const matrix4& other_b );
	// set this matrix to the product of two matrices, no logical optimation
	// use it if you know you never have a identity matrix
	void setbyproduct_nocheck(const matrix4& other_a,const matrix4& other_b );
	
	matrix4 operator*(const matrix4& other) const; //행렬 곱하기
	matrix4& operator*=(const matrix4& other);
	
	matrix4 operator*(const f32 scalar) const; //스칼라 곱하기
	matrix4& operator*=(const f32 scalar);
	
	inline void makeIdentity(); // 단위행렬 만들기
	inline bool isIdentity() const; // 단위 행렬인지 검사
	bool isIdentity_integer_base () const; // 정수베이스 단위 행렬인지 검사
	
	// Set the translation of the current matrix. Will erase any previous values.
	void setTranslation( const vector3df& translation );//이동 행렬 함수
	void setTranslation( const vector3dd& translation );//이동 행렬 함수 jsyun
	vector3df getTranslation() const;
	
	// 스케일 행렬 세팅
	void setScale( const vector3df& scale );
	vector3df getScale() const; 

	// 회전 행렬 세팅 axis: 회전 축 벡터  degree : 회전각도
	void SetRotationRadian( const vector3df& axis ,const f32 radian);
	void SetRotationRadian( const vector3dd& axis ,const f32 radian);//jsyun
	void SetRotationDegree( const vector3df& axis ,const f32 degree);
	void SetRotationDegree( const vector3dd& axis ,const f32 degree);//jsyun

	void SetRotation( const vector3df& axis ,const f32 degree);
	
	// Rotate a vector by the inverse of the rotation part of this matrix.
	void inverseRotateVect( vector3df& vect ) const; //행렬에 의한 3D 점 회전 변환
	// Rotate a vector by the rotation part of this matrix.
	void rotateVect( vector3df& vect ) const;
	void rotateVect(f32 *out,const vector3df &in) const;

	// Transforms the vector by this matrix 행렬에 의해 3차원 좌표 변환
	void transformVect( vector3df& vect) const;
	void transformVect( vector3df& out, const vector3df& in ) const; //결과값 좌표로
	void transformVect(f32 *out,const vector3df &in) const; //결과값 행렬로
	
	// Translate a vector by the translation part of this matrix. //행렬에 의한 이동 변환
	void translateVect( vector3df& vect ) const;
	
	// Transforms a plane by this matrix 행렬에 의한 평면 변환
	void transformPlane( plane3df &plane) const;
	void transformPlane( const plane3df &in, plane3df &out) const;
	
	// Transforms a axis aligned bounding box 행렬에 의한 박스 평면 변환
	// The result box of this operation may not be very accurate. For accurate results, use transformBoxEx() 
	void transformBox(aabbox3df& box) const;
	void transformBoxEx(aabbox3df& box) const; //정확한데 속도 느림
	
	// Multiplies this matrix by a 1x4 matrix
	void multiplyWith1x4Matrix(f32* matrix) const;
	
	// Calculates inverse of matrix. Slow.
	// \return Returns false if there is no inverse matrix.
	bool makeInverse(); // 역행렬 계산 
	
	// Inverts a primitive matrix which only contains a translation and a rotation
	// \param out: where result matrix is written to.
	bool getInversePrimitive ( matrix4& out ) const;
	
	// returns the inversed matrix of this one
	// \param out: where result matrix is written to.
	// \return Returns false if there is no inverse matrix.
	bool getInverse(matrix4& out) const;
	
	// Builds a right-handed perspective projection matrix based on a field of view
	void buildProjectionMatrixPerspectiveFovRH(f32 fieldOfViewRadians, f32 aspectRatio, f32 zNear, f32 zFar);
	
	// Builds a left-handed perspective projection matrix based on a field of view
	void buildProjectionMatrixPerspectiveFovLH(f32 fieldOfViewRadians, f32 aspectRatio, f32 zNear, f32 zFar);
	
	// Builds a right-handed perspective projection matrix.
	void buildProjectionMatrixPerspectiveRH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar);
	
	// Builds a left-handed perspective projection matrix.
	void buildProjectionMatrixPerspectiveLH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar);
	
	// Builds a left-handed orthogonal projection matrix.
	void buildProjectionMatrixOrthoLH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar);
	
	// Builds a right-handed orthogonal projection matrix.
	void buildProjectionMatrixOrthoRH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar);
	
	// Builds a left-handed look-at matrix.
	void buildCameraLookAtMatrixLH(const vector3df& position, const vector3df& target, const vector3df& upVector);
	
	// Builds a right-handed look-at matrix.
	void buildCameraLookAtMatrixRH(const vector3df& position, const vector3df& target, const vector3df& upVector);
	
	// Builds a matrix that flattens geometry into a plane.
	// \param light: light source
	// \param plane: plane into which the geometry if flattened into
	// \param point: value between 0 and 1, describing the light source.
	// If this is 1, it is a point light, if it is 0, it is a directional light.
	void buildShadowMatrix(const vector3df& light, plane3df plane, f32 point=1.0f);
	
	// Builds a matrix which transforms a normalized Device Coordinate to Device Coordinates.
	// Used to scale <-1,-1><1,1> to viewport, for example from von <-1,-1> <1,1> to the viewport <0,0><0,640> */
//	void buildNDCToDCMatrix( const rect<s32>& area, f32 zScale);
	
	// creates a new matrix as interpolated matrix from two other ones.
	// \param b: other matrix to interpolate with
	// \param time: Must be a value between 0 and 1.
	matrix4 interpolate(const matrix4& b, f32 time) const;

	matrix4 getTransposed() const; // 전치행렬 구하기
	inline void getTransposed( matrix4& dest ) const; 
	
	// construct 2D Texture transformations rotate about center, scale, and transform.
	void setTextureScale ( f32 sx, f32 sy );
	
	void setTextureRotationCenter( f32 radAngle );
	void setTextureScaleCenter( f32 sx, f32 sy );
	
	void setTextureTranslate ( f32 x, f32 y );
	
	void buildTextureTransform( f32 rotateRad,	const vector2df &rotatecenter,
								const vector2df &translate,const vector2df &scale);
	
};
	
inline matrix4::matrix4( matrix4::eConstructor constructor ) : definitelyIdentityMatrix(false)
{
	switch ( constructor )
	{
	case EM4CONST_NOTHING:
	case EM4CONST_COPY:
		break;
	case EM4CONST_IDENTITY:
	case EM4CONST_INVERSE:
	default:
		makeIdentity();
		break;
	}
}

inline matrix4::matrix4( const matrix4& other,matrix4::eConstructor constructor) : definitelyIdentityMatrix(false)
{
	switch ( constructor )
	{
	case EM4CONST_IDENTITY:
		makeIdentity();
		break;
	case EM4CONST_NOTHING:
		break;
	case EM4CONST_COPY:
		*this = other;
		break;
	case EM4CONST_TRANSPOSED:
		other.getTransposed(*this);//행과 열을 전치시킴
		break;
	case EM4CONST_INVERSE:
		if (!other.getInverse(*this)) //역행렬 구하기
			memset(M, 0, 16*sizeof(f32));
		break;
	case EM4CONST_INVERSE_TRANSPOSED: //역행렬 구하기 성공이면 여겨행렬 실패이면 전치행렬
		if (!other.getInverse(*this))
			memset(M, 0, 16*sizeof(f32));
		else
			*this=getTransposed();
		break;
	}
}

inline matrix4::matrix4(float _11, float _12, float _13, float _14,
	float _21, float _22, float _23, float _24,
	float _31, float _32, float _33, float _34,
	float _41, float _42, float _43, float _44) : definitelyIdentityMatrix(false)
{
	M[0] = _11;		M[1] = _12;		M[2] = _13;		M[3] = _14;
	M[4] = _21;		M[5] = _22;		M[6] = _23;		M[7] = _24;
	M[8] = _31;		M[9] = _32;		M[10] = _33;	M[11] = _34;
	M[12] = _41;	M[13] = _42;	M[14] = _43;	M[15] = _44;
}
//행렬 더하기
inline matrix4 matrix4::operator+(const matrix4& other) const {
	matrix4 temp ( EM4CONST_NOTHING );
	for(u8 i=0;i<16;i++) temp[i] = M[i]+other[i];
	return temp;
}
inline matrix4& matrix4::operator+=(const matrix4& other) {
	for(u8 i=0;i<16;i++) M[i] += other[i];
	return *this;
}

//Subtract another matrix.행렬 빼기
inline matrix4 matrix4::operator-(const matrix4& other) const	{
	matrix4 temp ( EM4CONST_NOTHING );
	for(u8 i=0;i<16;i++) temp[i] = M[i]-other[i];
	return temp;
}
inline matrix4& matrix4::operator-=(const matrix4& other) {
	for(u8 i=0;i<16;i++) M[i] -= other[i];
	return *this;
}

// Multiply by scalar.
inline matrix4 matrix4::operator*(const f32 scalar) const {
	matrix4 temp ( EM4CONST_NOTHING );
	for(u8 i=0;i<16;i++) temp[i] = M[i]*scalar;
	return temp;
}
inline matrix4& matrix4::operator*=(const f32 scalar)	{
	for(u8 i=0;i<16;i++) M[i]*=scalar;
	return *this;
}

// Multiply by another matrix.행렬 곱하기
inline matrix4& matrix4::operator*=(const matrix4& other) {
	matrix4 temp ( *this );
	setbyproduct ( temp, other );
	return *this;
}

// multiply by another matrix
// set this matrix to the product of two other matrices
// goal is to reduce stack use and copy
inline void matrix4::setbyproduct_nocheck(const matrix4& other_a,const matrix4& other_b )
{
	const f32 *m1 = other_a.M;
	const f32 *m2 = other_b.M;
	/*
	0  1  2  3      _11 _12 _13 _14
	4  5  6  7		_21 _22 _23 _24
	8  9  10 11		_31 _32 _33 _34
	12 13 14 15		_41 _42 _43 _44	
	*/
	
	M[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
	M[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
	M[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
	M[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
	
	M[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
	M[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
	M[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
	M[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
	
	M[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
	M[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
	M[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
	M[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];
	
	M[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
	M[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
	M[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	M[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
	definitelyIdentityMatrix=false;
}

// multiply by another matrix 두 행렬 곱하기
// set this matrix to the product of two other matrices
// goal is to reduce stack use and copy
inline void matrix4::setbyproduct(const matrix4& other_a,const matrix4& other_b )
{
	if ( other_a.isIdentity () ){
		*this = other_b;
		return;
	}
	else if ( other_b.isIdentity () ){
		*this = other_a;
		return;
	}
	setbyproduct_nocheck(other_a,other_b);
}

// multiply by another matrix
inline matrix4 matrix4::operator*(const matrix4& m2) const
{
	// Testing purpose.. 
	if ( this->isIdentity() )
		return m2;
	if ( m2.isIdentity() )
		return *this;
	
	matrix4 m3 ( EM4CONST_NOTHING );
	
	const f32 *m1 = M;
	
	m3[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
	m3[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
	m3[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
	m3[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
	
	m3[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
	m3[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
	m3[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
	m3[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
	
	m3[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
	m3[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
	m3[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
	m3[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];
	
	m3[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
	m3[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
	m3[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	m3[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

	return m3;
}

inline vector3df matrix4::getTranslation() const	{ 
	return vector3df(M[12], M[13], M[14]);
}

inline void matrix4::setTranslation( const vector3df& translation )
{
	M[12] = translation.x;
	M[13] = translation.y;
	M[14] = translation.z;
	definitelyIdentityMatrix=false;
}
inline void matrix4::setTranslation( const vector3dd& translation )
{
	M[12] = translation.x;
	M[13] = translation.y;
	M[14] = translation.z;
	definitelyIdentityMatrix=false;
}
inline void matrix4::setScale( const vector3df& scale )
{
	M[0] = scale.x;
	M[5] = scale.y;
	M[10] = scale.z;
	definitelyIdentityMatrix=false;
}
inline vector3df matrix4::getScale() const {	return vector3df(M[0],M[5],M[10]);}

inline void matrix4::SetRotationRadian( const vector3df& axis ,const f32 radian)
{
    f32			cr = (f32)cos( radian );
    f32			sr = (f32)sin( radian );
//	double		cr = cos( radian );
//	double		sr = sin( radian );
    vector3df	v  = axis;	v.normalize();

	makeIdentity();
    // X 좌표 변환값
    M[0] = ( v.x * v.x ) * ( 1.0f - cr ) + cr;
    M[4] = ( v.x * v.y ) * ( 1.0f - cr ) - (v.z * sr);
    M[8] = ( v.x * v.z ) * ( 1.0f - cr ) + (v.y * sr);
    // Y 좌표 변환값
    M[1] = ( v.y * v.x ) * ( 1.0f - cr ) + (v.z * sr);
    M[5] = ( v.y * v.y ) * ( 1.0f - cr ) + cr ;
    M[9] = ( v.y * v.z ) * ( 1.0f - cr ) - (v.x * sr);
    // Z 좌표 변환값
    M[2] = ( v.z * v.x ) * ( 1.0f - cr ) - (v.y * sr);
    M[6] = ( v.z * v.y ) * ( 1.0f - cr ) + (v.x * sr);
    M[10]= ( v.z * v.z ) * ( 1.0f - cr ) + cr;
	definitelyIdentityMatrix=false;

}

inline void matrix4::SetRotationRadian( const vector3dd& axis ,const f32 radian)
{
    f64			cr = (f64)cos( radian );
    f64			sr = (f64)sin( radian );
//	double		cr = cos( radian );
//	double		sr = sin( radian );
    vector3dd	v  = axis;	v.normalize();

	makeIdentity();
    // X 좌표 변환값
    M[0] = ( v.x * v.x ) * ( 1.0f - cr ) + cr;
    M[4] = ( v.x * v.y ) * ( 1.0f - cr ) - (v.z * sr);
    M[8] = ( v.x * v.z ) * ( 1.0f - cr ) + (v.y * sr);
    // Y 좌표 변환값
    M[1] = ( v.y * v.x ) * ( 1.0f - cr ) + (v.z * sr);
    M[5] = ( v.y * v.y ) * ( 1.0f - cr ) + cr ;
    M[9] = ( v.y * v.z ) * ( 1.0f - cr ) - (v.x * sr);
    // Z 좌표 변환값
    M[2] = ( v.z * v.x ) * ( 1.0f - cr ) - (v.y * sr);
    M[6] = ( v.z * v.y ) * ( 1.0f - cr ) + (v.x * sr);
    M[10]= ( v.z * v.z ) * ( 1.0f - cr ) + cr;
	definitelyIdentityMatrix=false;

}
// Angular displacement = (임의의 회전축을 나타내는 벡터) + (그 축에 대한 회전각)
// 회전 행렬 세팅 axis: 회전 축 벡터  degree : 회전각도
inline void matrix4::SetRotationDegree( const vector3df& axis ,const f32 degree)
{
	SetRotationRadian( axis , degree * DEGTORAD );
}
inline void matrix4::SetRotationDegree( const vector3dd& axis ,const f32 degree)
{
	SetRotationRadian( axis , degree * DEGTORAD );
}

inline void matrix4::makeIdentity()
{
	memset(M, 0, 16*sizeof(f32));
	M[0] = M[5] = M[10] = M[15] = 1.0f;
	definitelyIdentityMatrix=true;
}

//check identity with epsilon solve floating range problems..
inline bool matrix4::isIdentity() const
{
	if (definitelyIdentityMatrix)
		return true;
	if (!equal ( M[ 0], 1.f ) ||
		!equal ( M[ 5], 1.f ) ||
		!equal ( M[10], 1.f ) ||
		!equal ( M[15], 1.f ) )
		return false;
	
	for (s32 i=0; i<4; ++i)
		for (s32 j=0; j<4; ++j)
			if (j != i)
			{
				if ((*this)(i,j) < -ROUNDING_ERROR_32||
					(*this)(i,j) > ROUNDING_ERROR_32)
					return false;
			}
			return true;
}

//doesn't solve floating range problems.but takes care on +/- 0 on translation because we are changing it..
//reducing floating point branches but it need's the floats in memory..
inline bool matrix4::isIdentity_integer_base() const
{
	if (definitelyIdentityMatrix) return true;
	
	if(IR(M[0])!=F32_VALUE_1)	return false;
	if(IR(M[1])!=0)			return false;
	if(IR(M[2])!=0)			return false;
	if(IR(M[3])!=0)			return false;
	
	if(IR(M[4])!=0)			return false;
	if(IR(M[5])!=F32_VALUE_1)	return false;
	if(IR(M[6])!=0)			return false;
	if(IR(M[7])!=0)			return false;
	
	if(IR(M[8])!=0)			return false;
	if(IR(M[9])!=0)			return false;
	if(IR(M[10])!=F32_VALUE_1)	return false;
	if(IR(M[11])!=0)		return false;
	
	if(IR(M[12])!=0)		return false;
	if(IR(M[13])!=0)		return false;
	if(IR(M[13])!=0)		return false;
	if(IR(M[15])!=F32_VALUE_1)	return false;
	return true;
}



inline void matrix4::rotateVect( vector3df& vect ) const
{
	vector3df tmp = vect;
	vect.x = tmp.x*M[0] + tmp.y*M[4] + tmp.z*M[8];
	vect.y = tmp.x*M[1] + tmp.y*M[5] + tmp.z*M[9];
	vect.z = tmp.x*M[2] + tmp.y*M[6] + tmp.z*M[10];
}
// An alternate transform vector method, writing into an array of 3 floats
inline void matrix4::rotateVect(f32 *out,const vector3df &in) const
{
	out[0] = in.x*M[0] + in.y*M[4] + in.z*M[8];
	out[1] = in.x*M[1] + in.y*M[5] + in.z*M[9];
	out[2] = in.x*M[2] + in.y*M[6] + in.z*M[10];
}
inline void matrix4::inverseRotateVect( vector3df& vect ) const
{
	vector3df tmp = vect;
	vect.x = tmp.x*M[0] + tmp.y*M[1] + tmp.z*M[2];
	vect.y = tmp.x*M[4] + tmp.y*M[5] + tmp.z*M[6];
	vect.z = tmp.x*M[8] + tmp.y*M[9] + tmp.z*M[10];
}


inline void matrix4::transformVect( vector3df& vect) const
{
	f32 vector[4];
	
	vector[0] = vect.x*M[0] + vect.y*M[4] + vect.z*M[8] + M[12];
	vector[1] = vect.x*M[1] + vect.y*M[5] + vect.z*M[9] + M[13];
	vector[2] = vect.x*M[2] + vect.y*M[6] + vect.z*M[10] + M[14];
	vector[3] = vect.x*M[3] + vect.y*M[7] + vect.z*M[11] + M[15];
	
	if(fabs(vector[3])<ROUNDING_ERROR_32){// 분모가 0에 근사한지 확인 
		vect.x = vector[0];
		vect.y = vector[1];
		vect.z = vector[2];
		return;
	}
	vect.x = vector[0]/vector[3];
	vect.y = vector[1]/vector[3];
	vect.z = vector[2]/vector[3];
}

inline void matrix4::transformVect( vector3df& out, const vector3df& in) const
{
	out.x = in.x*M[0] + in.y*M[4] + in.z*M[8] + M[12];
	out.y = in.x*M[1] + in.y*M[5] + in.z*M[9] + M[13];
	out.z = in.x*M[2] + in.y*M[6] + in.z*M[10] + M[14];
}
// transform (x,y,z,1)
inline void matrix4::transformVect(f32 *out,const vector3df &in) const
{
	out[0] = in.x*M[0] + in.y*M[4] + in.z*M[8] + M[12];
	out[1] = in.x*M[1] + in.y*M[5] + in.z*M[9] + M[13];
	out[2] = in.x*M[2] + in.y*M[6] + in.z*M[10] + M[14];
	out[3] = in.x*M[3] + in.y*M[7] + in.z*M[11] + M[15];
}


// Transforms a plane by this matrix 행렬에 의한 평면 변환======================
inline void matrix4::transformPlane( plane3df &plane) const
{
	vector3df member;
	transformVect(member, plane.getMemberPoint());
	
	vector3df origin(0,0,0);
	transformVect(plane.Normal);
	transformVect(origin);
	
	plane.Normal -= origin;
	plane.D = - member.dotProduct(plane.Normal);
}
// Transforms a plane by this matrix
inline void matrix4::transformPlane( const plane3df &in, plane3df &out) const
{
	out = in;
	transformPlane( out );
}

// Transforms a axis aligned bounding box
inline void matrix4::transformBox(aabbox3df& box) const
{
	if (isIdentity() )
		return;
	
	transformVect(box.Min);
	transformVect(box.Max);
	box.repair();
}

// Transforms a axis aligned bounding box more accurately than transformBox() 정확하지만 속도는 느림
inline void matrix4::transformBoxEx(aabbox3df& box) const 
{
	f32 Amin[3];
	f32 Amax[3];
	f32 Bmin[3];
	f32 Bmax[3];
	
	Amin[0] = box.Min.x;
	Amin[1] = box.Min.y;
	Amin[2] = box.Min.z;
	
	Amax[0] = box.Max.x;
	Amax[1] = box.Max.y;
	Amax[2] = box.Max.z;
	
	Bmin[0] = Bmax[0] = M[12];
	Bmin[1] = Bmax[1] = M[13];
	Bmin[2] = Bmax[2] = M[14];
	
	u32 i, j;
	const matrix4 &m = *this;
	
	for (i = 0; i < 3; ++i) 
	{
		for (j = 0; j < 3; ++j) 
		{
			f32 a = m(j,i) * Amin[j];
			f32 b = m(j,i) * Amax[j];
			
			if (a < b)
			{
				Bmin[i] += a;
				Bmax[i] += b;
			}
			else
			{
				Bmin[i] += b;
				Bmax[i] += a;
			}
		}
	}
	
	box.Min.x = Bmin[0];
	box.Min.y = Bmin[1];
	box.Min.z = Bmin[2];
	
	box.Max.x = Bmax[0];
	box.Max.y = Bmax[1];
	box.Max.z = Bmax[2];
}


// Multiplies this matrix by a 1x4 matrix
inline void matrix4::multiplyWith1x4Matrix(f32* matrix) const
{
	/*
	0  1  2  3
	4  5  6  7
	8  9  10 11
	12 13 14 15	
	*/
	f32 mat[4];
	mat[0] = matrix[0];
	mat[1] = matrix[1];
	mat[2] = matrix[2];
	mat[3] = matrix[3];
	
	matrix[0] = M[0]*mat[0] + M[4]*mat[1] + M[8]*mat[2] + M[12]*mat[3];
	matrix[1] = M[1]*mat[0] + M[5]*mat[1] + M[9]*mat[2] + M[13]*mat[3];
	matrix[2] = M[2]*mat[0] + M[6]*mat[1] + M[10]*mat[2] + M[14]*mat[3];
	matrix[3] = M[3]*mat[0] + M[7]*mat[1] + M[11]*mat[2] + M[15]*mat[3];
}

inline void matrix4::translateVect( vector3df& vect ) const
{
	vect.x = vect.x+M[12];
	vect.y = vect.y+M[13];
	vect.z = vect.z+M[14];
}

// Calculates the inverse of this Matrix
// The inverse is calculated using Cramers rule.
// If no inverse exists then 'false' is returned.
inline bool matrix4::getInverse(matrix4& out) const
{
	const matrix4 &m = *this;
	
	f32 d = (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0)) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)) -
		(m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)) +
		(m(0, 0) * m(1, 3) - m(0, 3) * m(1, 0)) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)) +
		(m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)) -
		(m(0, 1) * m(1, 3) - m(0, 3) * m(1, 1)) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)) +
		(m(0, 2) * m(1, 3) - m(0, 3) * m(1, 2)) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0));
	
	if( iszero ( d ) )	return false;
	
	d = reciprocal ( d );
	
	out(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)) + m(1, 2) * (m(2, 3) * m(3, 1) - m(2, 1) * m(3, 3)) + m(1, 3) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1)));
	out(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(0, 3) * m(3, 2)) + m(2, 2) * (m(0, 3) * m(3, 1) - m(0, 1) * m(3, 3)) + m(2, 3) * (m(0, 1) * m(3, 2) - m(0, 2) * m(3, 1)));
	out(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(0, 3) * m(1, 2)) + m(3, 2) * (m(0, 3) * m(1, 1) - m(0, 1) * m(1, 3)) + m(3, 3) * (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)));
	out(0, 3) = d * (m(0, 1) * (m(1, 3) * m(2, 2) - m(1, 2) * m(2, 3)) + m(0, 2) * (m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1)) + m(0, 3) * (m(1, 2) * m(2, 1) - m(1, 1) * m(2, 2)));
	out(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0)) + m(1, 3) * (m(2, 2) * m(3, 0) - m(2, 0) * m(3, 2)) + m(1, 0) * (m(2, 3) * m(3, 2) - m(2, 2) * m(3, 3)));
	out(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(0, 3) * m(3, 0)) + m(2, 3) * (m(0, 2) * m(3, 0) - m(0, 0) * m(3, 2)) + m(2, 0) * (m(0, 3) * m(3, 2) - m(0, 2) * m(3, 3)));
	out(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(0, 3) * m(1, 0)) + m(3, 3) * (m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2)) + m(3, 0) * (m(0, 3) * m(1, 2) - m(0, 2) * m(1, 3)));
	out(1, 3) = d * (m(0, 2) * (m(1, 3) * m(2, 0) - m(1, 0) * m(2, 3)) + m(0, 3) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) + m(0, 0) * (m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2)));
	out(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0)) + m(1, 0) * (m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1)) + m(1, 1) * (m(2, 3) * m(3, 0) - m(2, 0) * m(3, 3)));
	out(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(0, 1) * m(3, 0)) + m(2, 0) * (m(0, 1) * m(3, 3) - m(0, 3) * m(3, 1)) + m(2, 1) * (m(0, 3) * m(3, 0) - m(0, 0) * m(3, 3)));
	out(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0)) + m(3, 0) * (m(0, 1) * m(1, 3) - m(0, 3) * m(1, 1)) + m(3, 1) * (m(0, 3) * m(1, 0) - m(0, 0) * m(1, 3)));
	out(2, 3) = d * (m(0, 3) * (m(1, 1) * m(2, 0) - m(1, 0) * m(2, 1)) + m(0, 0) * (m(1, 3) * m(2, 1) - m(1, 1) * m(2, 3)) + m(0, 1) * (m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0)));
	out(3, 0) = d * (m(1, 0) * (m(2, 2) * m(3, 1) - m(2, 1) * m(3, 2)) + m(1, 1) * (m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0)) + m(1, 2) * (m(2, 1) * m(3, 0) - m(2, 0) * m(3, 1)));
	out(3, 1) = d * (m(2, 0) * (m(0, 2) * m(3, 1) - m(0, 1) * m(3, 2)) + m(2, 1) * (m(0, 0) * m(3, 2) - m(0, 2) * m(3, 0)) + m(2, 2) * (m(0, 1) * m(3, 0) - m(0, 0) * m(3, 1)));
	out(3, 2) = d * (m(3, 0) * (m(0, 2) * m(1, 1) - m(0, 1) * m(1, 2)) + m(3, 1) * (m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) + m(3, 2) * (m(0, 1) * m(1, 0) - m(0, 0) * m(1, 1)));
	out(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) + m(0, 1) * (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) + m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)));
	out.definitelyIdentityMatrix = definitelyIdentityMatrix;
	return true;
}

// 이동과 회전에 대한 아이템만 역행렬을 구함
// Inverts a primitive matrix which only contains a translation and a rotation
// param out: where result matrix is written to.
inline bool matrix4::getInversePrimitive ( matrix4& out ) const
{
	out.M[0 ]	= M[0];
	out.M[1 ]	= M[4];
	out.M[2 ]	= M[8];
	out.M[3 ]	= 0.0f;
	
	out.M[4 ]	= M[1];
	out.M[5 ]	= M[5];
	out.M[6 ]	= M[9];
	out.M[7 ]	= 0.0f;
	
	out.M[8 ]	= M[2];
	out.M[9 ]	= M[6];
	out.M[10]	= M[10];
	out.M[11]	= 0.0f;
	
	out.M[12]	= -(M[12]*M[0] + M[13]*M[1] + M[14]*M[2]);
	out.M[13]	= -(M[12]*M[4] + M[13]*M[5] + M[14]*M[6]);
	out.M[14]	= -(M[12]*M[8] + M[13]*M[9] + M[14]*M[10]);
	out.M[15]	= 1.0f;
	out.definitelyIdentityMatrix = definitelyIdentityMatrix;
	return true;
}

inline bool matrix4::makeInverse()
{
	matrix4 temp ( EM4CONST_NOTHING );
	
	if (getInverse(temp))
	{
		*this = temp;
		return true;
	}
	
	return false;
}

inline matrix4& matrix4::operator=(const matrix4 &other)
{
	if (this==&other)
		return *this;
	memcpy(M, other.M, 16*sizeof(f32));
	definitelyIdentityMatrix=other.definitelyIdentityMatrix;
	return *this;
}

inline bool matrix4::operator==(const matrix4 &other) const
{
	if (definitelyIdentityMatrix && other.definitelyIdentityMatrix)
		return true;
	for (s32 i = 0; i < 16; ++i)
		if (M[i] != other.M[i])
			return false;
		
		return true;
}

inline bool matrix4::operator!=(const matrix4 &other) const
{
	return !(*this == other);
}



//! Builds a right-handed perspective projection matrix based on a field of view
inline void matrix4::buildProjectionMatrixPerspectiveFovRH(f32 fieldOfViewRadians, f32 aspectRatio, f32 zNear, f32 zFar)
{
	f32 h = (f32)(1.0/tan(fieldOfViewRadians/2.0));
	f32 w = h / aspectRatio;
	
	M[0] = w;
	M[1] = 0.0f;
	M[2] = 0.0f;
	M[3] = 0.0f;
	
	M[4] = 0.0f;
	M[5] = h;
	M[6] = 0.0f;
	M[7] = 0.0f;
	
	M[8] = 0.0f;
	M[9] = 0.0f;
	M[10] = zFar/(zNear-zFar); // DirectX version
	//		M[10] = zFar+zNear/(zNear-zFar); // OpenGL version
	M[11] = -1.0f;
	
	M[12] = 0.0f;
	M[13] = 0.0f;
	M[14] = zNear*zFar/(zNear-zFar); // DirectX version
	//		M[14] = 2.0f*zNear*zFar/(zNear-zFar); // OpenGL version
	M[15] = 0.0f;
	definitelyIdentityMatrix=false;
}


// Builds a left-handed perspective projection matrix based on a field of view
inline void matrix4::buildProjectionMatrixPerspectiveFovLH(f32 fieldOfViewRadians, f32 aspectRatio, f32 zNear, f32 zFar)
{
	f32 h = (f32)(1.0/tan(fieldOfViewRadians/2.0));
	f32 w = h / aspectRatio;
	
	M[0] = w;
	M[1] = 0.0f;
	M[2] = 0.0f;
	M[3] = 0.0f;
	
	M[4] = 0.0f;
	M[5] = h;
	M[6] = 0.0f;
	M[7] = 0.0f;
	
	M[8] = 0.0f;
	M[9] = 0.0f;
	M[10] = zFar/(zFar-zNear);
//	M[10] = 1.0f;
	M[11] = 1.0f;
	
	M[12] = 0.0f;
	M[13] = 0.0f;
	M[14] = -zNear*zFar/(zFar-zNear);
	M[15] = 0.0f;

	definitelyIdentityMatrix=false;
}
	/*				 directx version
	0  1  2  3      _11 _12 _13 _14
	4  5  6  7		_21 _22 _23 _24
	8  9  10 11		_31 _32 _33 _34
	12 13 14 15		_41 _42 _43 _44	
	*/
// Builds a left-handed orthogonal projection matrix.
inline void matrix4::buildProjectionMatrixOrthoLH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar)
{
	M[0] = 2/widthOfViewVolume;
	M[1] = 0;
	M[2] = 0;
	M[3] = 0;
	
	M[4] = 0;
	M[5] = 2/heightOfViewVolume;
	M[6] = 0;
	M[7] = 0;
	
	M[8] = 0;
	M[9] = 0;
	M[10] = 1/(zFar-zNear);
	M[11] = 0;
	
	M[12] = 0;
	M[13] = 0;
	M[14] = zNear/(zNear-zFar);
	M[15] = 1;
	definitelyIdentityMatrix=false;
}



//! Builds a right-handed orthogonal projection matrix.
inline void matrix4::buildProjectionMatrixOrthoRH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar)
{
	M[0] = 2/widthOfViewVolume;
	M[1] = 0;
	M[2] = 0;
	M[3] = 0;
	
	M[4] = 0;
	M[5] = 2/heightOfViewVolume;
	M[6] = 0;
	M[7] = 0;
	
	M[8] = 0;
	M[9] = 0;
	M[10] = 1/(zNear-zFar);
	M[11] = 0;
	
	M[12] = 0;
	M[13] = 0;
	M[14] = zNear/(zNear-zFar);
	M[15] = -1;
	definitelyIdentityMatrix=false;
}


// Builds a right-handed perspective projection matrix.
inline void matrix4::buildProjectionMatrixPerspectiveRH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar)
{
	M[0] = 2*zNear/widthOfViewVolume;
	M[1] = 0;
	M[2] = 0;
	M[3] = 0;
	
	M[4] = 0;
	M[5] = 2*zNear/heightOfViewVolume;
	M[6] = 0;
	M[7] = 0;
	
	M[8] = 0;
	M[9] = 0;
	M[10] = zFar/(zNear-zFar);
	M[11] = -1;
	
	M[12] = 0;
	M[13] = 0;
	M[14] = zNear*zFar/(zNear-zFar);
	M[15] = 0;
	definitelyIdentityMatrix=false;
}


// Builds a left-handed perspective projection matrix.
inline void matrix4::buildProjectionMatrixPerspectiveLH(f32 widthOfViewVolume, f32 heightOfViewVolume, f32 zNear, f32 zFar)
{
	M[0] = 2*zNear/widthOfViewVolume;
	M[1] = 0;
	M[2] = 0;
	M[3] = 0;
	
	M[4] = 0;
	M[5] = 2*zNear/heightOfViewVolume;
	M[6] = 0;
	M[7] = 0;
	
	M[8] = 0;
	M[9] = 0;
	M[10] = zFar/(zFar-zNear);
	M[11] = 1;
	
	M[12] = 0;
	M[13] = 0;
	M[14] = zNear*zFar/(zNear-zFar);
	M[15] = 0;
	definitelyIdentityMatrix=false;
}


// Builds a matrix that flattens geometry into a plane.
inline void matrix4::buildShadowMatrix(const vector3df& light, plane3df plane, f32 point)
{
	plane.Normal.normalize();
	f32 d = plane.Normal.dotProduct(light);
	
	M[ 0] = -plane.Normal.x * light.x + d;
	M[ 1] = -plane.Normal.x * light.y;
	M[ 2] = -plane.Normal.x * light.z;
	M[ 3] = -plane.Normal.x * point;
	
	M[ 4] = -plane.Normal.y * light.x;
	M[ 5] = -plane.Normal.y * light.y + d;
	M[ 6] = -plane.Normal.y * light.z;
	M[ 7] = -plane.Normal.y * point;
	
	M[ 8] = -plane.Normal.z * light.x;
	M[ 9] = -plane.Normal.z * light.y;
	M[10] = -plane.Normal.z * light.z + d;
	M[11] = -plane.Normal.z * point;
	
	M[12] = -plane.D * light.x;
	M[13] = -plane.D * light.y;
	M[14] = -plane.D * light.z;
	M[15] = -plane.D * point + d; 
	definitelyIdentityMatrix=false;
}

// Builds a left-handed look-at matrix.
inline void matrix4::buildCameraLookAtMatrixLH(	const vector3df& position,
								const vector3df& target,  const vector3df& upVector)
{
	vector3df zaxis = target - position;
	zaxis.normalize();
	
	vector3df xaxis = upVector.crossProduct(zaxis);
	xaxis.normalize();
	
	vector3df yaxis = zaxis.crossProduct(xaxis);
	
	M[0] = xaxis.x;
	M[1] = yaxis.x;
	M[2] = zaxis.x;
	M[3] = 0;
	
	M[4] = xaxis.y;
	M[5] = yaxis.y;
	M[6] = zaxis.y;
	M[7] = 0;
	
	M[8] = xaxis.z;
	M[9] = yaxis.z;
	M[10] = zaxis.z;
	M[11] = 0;
	
	M[12] = -xaxis.dotProduct(position);
	M[13] = -yaxis.dotProduct(position);
	M[14] = -zaxis.dotProduct(position);
	M[15] = 1.0f;
	definitelyIdentityMatrix=false;
}



// Builds a right-handed look-at matrix.
inline void matrix4::buildCameraLookAtMatrixRH(	const vector3df& position,
								  const vector3df& target, const vector3df& upVector)
{
	vector3df zaxis = position - target;
	zaxis.normalize();
	
	vector3df xaxis = upVector.crossProduct(zaxis);
	xaxis.normalize();
	
	vector3df yaxis = zaxis.crossProduct(xaxis);
	
	M[0] = xaxis.x;
	M[1] = yaxis.x;
	M[2] = zaxis.x;
	M[3] = 0;
	
	M[4] = xaxis.y;
	M[5] = yaxis.y;
	M[6] = zaxis.y;
	M[7] = 0;
	
	M[8] = xaxis.z;
	M[9] = yaxis.z;
	M[10] = zaxis.z;
	M[11] = 0;
	
	M[12] = -xaxis.dotProduct(position);
	M[13] = -yaxis.dotProduct(position);
	M[14] = -zaxis.dotProduct(position);
	M[15] = 1.0f;
	definitelyIdentityMatrix=false;
}


// creates a new matrix as interpolated matrix from to other ones.
// \param time: Must be a value between 0 and 1.
inline matrix4 matrix4::interpolate(const matrix4& b, f32 time) const
{
	matrix4 mat ( EM4CONST_NOTHING );
	
	for (s32 i=0; i < 16; i += 4)
	{
		mat.M[i+0] = M[i+0] + ( b.M[i+0] - M[i+0] ) * time;
		mat.M[i+1] = M[i+1] + ( b.M[i+1] - M[i+1] ) * time;
		mat.M[i+2] = M[i+2] + ( b.M[i+2] - M[i+2] ) * time;
		mat.M[i+3] = M[i+3] + ( b.M[i+3] - M[i+3] ) * time;
	}
	return mat;
}

//전치 행렬 구하기(행과 열 바꿈)
inline matrix4 matrix4::getTransposed() const
{
	matrix4 t ( EM4CONST_NOTHING );
	getTransposed ( t );
	return t;
}
inline void matrix4::getTransposed( matrix4& o ) const
{
	o[ 0] = M[ 0];	o[ 1] = M[ 4];	o[ 2] = M[ 8];	o[ 3] = M[12];
	o[ 4] = M[ 1];	o[ 5] = M[ 5];	o[ 6] = M[ 9];	o[ 7] = M[13];
	o[ 8] = M[ 2];	o[ 9] = M[ 6];	o[10] = M[10];	o[11] = M[14];
	o[12] = M[ 3];	o[13] = M[ 7];	o[14] = M[11];	o[15] = M[15];
	o.definitelyIdentityMatrix=definitelyIdentityMatrix;
}


// used to scale <-1,-1><1,1> to viewport
/*	inline void matrix4::buildNDCToDCMatrix( const rect<s32>& viewport, f32 zScale)
{
f32 scaleX = (viewport.getWidth() - 0.75f ) / 2.f;
f32 scaleY = -(viewport.getHeight() - 0.75f ) / 2.f;

		f32 dx = -0.5f + ( (viewport.UpperLeftCorner.x + viewport.LowerRightCorner.x ) / 2.f );
		f32 dy = -0.5f + ( (viewport.UpperLeftCorner.y + viewport.LowerRightCorner.y ) / 2.f );
		
		  makeIdentity();
		  M[0] = scaleX;
		  M[5] = scaleY;
		  M[10] = zScale;
		  M[12] = dx;
		  M[13] = dy;
		  definitelyIdentityMatrix=false;
		  }
*/	
/*!
Generate texture coordinates as linear functions so that:
u = Ux*x + Uy*y + Uz*z + Uw 
v = Vx*x + Vy*y + Vz*z + Vw
The matrix M for this case is:
Ux  Vx  0  0 
Uy  Vy  0  0 
Uz  Vz  0  0 
Uw  Vw  0  0 
*/

inline void matrix4::buildTextureTransform( f32 rotateRad,
										   const vector2df &rotatecenter,
										   const vector2df &translate,
										   const vector2df &scale)
{
	f32 c = cosf(rotateRad);
	f32 s = sinf(rotateRad);
	
	M[0] = c * scale.x;
	M[1] = s * scale.y;
	M[2] = 0.f;
	M[3] = 0.f;
	
	M[4] = -s * scale.x;
	M[5] = c * scale.y;
	M[6] = 0.f;
	M[7] = 0.f;
	
	M[8] = c * scale.x * rotatecenter.x + -s * rotatecenter.y + translate.x;
	M[9] = s * scale.y * rotatecenter.x +  c * rotatecenter.y + translate.y;
	M[10] = 1.f;
	M[11] = 0.f;
	
	M[12] = 0.f;
	M[13] = 0.f;
	M[14] = 0.f;
	M[15] = 1.f;
	definitelyIdentityMatrix=false;
}

//! rotate about z axis, center ( 0.5, 0.5 )
inline void matrix4::setTextureRotationCenter( f32 rotateRad )
{
	f32 c = cosf(rotateRad);
	f32 s = sinf(rotateRad);
	M[0] = c;
	M[1] = s;
	M[2] = -0.5f * ( c + s) + 0.5f;
	
	M[4] = -s;
	M[5] = c;
	M[6] = -0.5f * (-s + c) + 0.5f;
	definitelyIdentityMatrix=false;
}

inline void matrix4::setTextureTranslate ( f32 x, f32 y )
{
	M[2] = x;
	M[6] = y;
	definitelyIdentityMatrix=false;
}

inline void matrix4::setTextureScale ( f32 sx, f32 sy )
{
	M[0] = sx;
	M[5] = sy;
	definitelyIdentityMatrix=false;
}

inline void matrix4::setTextureScaleCenter( f32 sx, f32 sy )
{
	M[0] = sx;
	M[2] = -0.5f * sx + 0.5f;
	M[5] = sy;
	M[6] = -0.5f * sy + 0.5f;
	definitelyIdentityMatrix=false;
}

//Multiply by scalar.
inline matrix4 operator*(const f32 scalar, const matrix4& mat)
{
	return mat*scalar;
}

const matrix4 IdentityMatrix(matrix4::EM4CONST_IDENTITY);


#endif

/*
Euler Angles는 3차원 공간에서 물체가 취할 수 있는 방향을 나타내는데 사용되는 세개의 각도 값의 조합을 말합니다. 

18세기 수학자 오일러가 착안한 것으로 3차원 공간에서는 3차원 직교 좌표계의 좌표축인 x, y, z 축에 대한 회전을 적당히 

조합하면 임의의 방향을 나타낼 수 있다는 사실입니다.

2차원 평면에서 물체의 방향을 지정해주려면 단지 하나의 각도 값이면 충분하지만 3차원 공간에서는 최소한 세개의 각도값이 있어야 하죠.
유의할 점은 오일러 앵글은 특정 회전축의 조합 하나를 말하는게 아니라 3차원 공간에서 임의의 방향을 나타낼 수 있는 조합이라면 어떤 조합도 다 오일러 앵글입니다.

회전을 물체의 회전으로 보느냐? 아니면 좌표계의 회전으로 보느냐? 에 따라서 도 달라질 수 있고, 기준축을 어느것으로 할지에 따라서도 달라질 수 있으므로 사용하려는 목적에 맞는 가장 편리하고 직관적인 조합을 사용하는 것이 좋죠.
보통의 경우 많이 쓰이는 조합이 있는데 3차원 그래픽의 경우 X, Y, Z축 회전을 차례대로 조합한 걸 많이 사용하고, 물리의 강체역학에서는 Z, X, Z축 회전 조합을 많이 사용합니다.
저는 개인적으로 Y, X, Z축 회전 조합을 좋아합니다.

각설하고 임의의 3차원 방향을 나타내는데 문제가 없는 조합이면 다 오일러 앵글입니다. 
그런데 문제는 수학적인 견지에서 볼때 이러한 임의의 세 각도 값이 서로 독립적이지 않다는 겁니다. 
그래서 실제 로보트 팔을 제어하거나 할때 어떤 축의 회전이 하나마나 한 경우가 생기는데 이걸 Gimbal Lock이라고 합니다. 
그리고 임의의 방향을 설정할때 세축의 회전을 조합해야 하는데 이게 얼른 직관적으로 되질 않고, 현재의 특정 방향에서 다른 방향으로 바꾸려고 할때 각 회전 값을 얼만큼 변화시키면 되는지가 애매합니다.
이게 오일러 앵글의 치명적인 약점입니다.
일견 기본 좌표축에 대한 회전의 조합이라서 편하기도 하지만.. 

위치의 경우 : 
　임의의 위치 = X축 위치성분 + Y축 위치성분 + Z축 위치성분
<<x, y, z 성분은 서로 완전히 독립적>> 

방향의 경우 : 
　임의의 방향 = X축 회전행렬*Y축 회전행렬*Z축 회전행렬(오일러 앵글)
<<위치와는 달리 서로 독립적이지 않다는 것이죠.>> 

그래서 이런 문제를 피하기 위해서 오일러 앵글 대신에.. 

Angular displacement 
= (임의의 회전축을 나타내는 벡터) + (그 축에 대한 회전각) 
을 사용하기도 합니다.

Angular displacement는 각도성분 하나, 벡터성분 세개 해서 (Theta, Vx, Vy, Vz) 식으로 네개의 값으로 구성되고, 
이 네 값은 다행히도 서로 독립적입니다(OpenGL 의 경우도 이 방법을 씁니다).
그래서 오일러 앵글과는 달리 애매한 경우는 생기지 않지만 키프레임 에니메이션을 구현하려 할 때 문제가 있습니다. 
키프레임으로 지정된 두개의 방향을 서로 보간 하려고 할때 단순히 네 값의 차이를 서로 선형 보간하면 될 것 같지만 
그렇지가 않다는 것이죠.

이건 Quaternion이란 복소수(실수와 허수)를 확장한 수학적 양을 도입해서 해결하는데 쿼터니온에 대해서는 따로 적지요. 

아래 소스는 Euler Angles 를 이용해 구현한 것...==================================================================================  
	// Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
	inline void setRotationRadians( const vector3df& rotation );
	// Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
	void setRotationDegrees( const vector3df& rotation );
	// Returns the rotation, as set by setRotation(). This code was orginally written by by Chev.
	vector3df getRotationDegrees() const;
	// Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	inline void setInverseRotationRadians( const vector3df& rotation );
	// Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	void setInverseRotationDegrees( const vector3df& rotation );

inline void matrix4::setRotationDegrees( const vector3df& rotation )
{
	setRotationRadians( rotation * DEGTORAD );
}

inline void matrix4::setInverseRotationDegrees( const vector3df& rotation )
{
	setInverseRotationRadians( rotation * DEGTORAD );
}

inline void matrix4::setRotationRadians( const vector3df& rotation )
{
	f64 cr = cos( rotation.x );
	f64 sr = sin( rotation.x );
	f64 cp = cos( rotation.y );
	f64 sp = sin( rotation.y );
	f64 cy = cos( rotation.z );
	f64 sy = sin( rotation.z );
	
	M[0] = (f32)( cp*cy );
	M[1] = (f32)( cp*sy );
	M[2] = (f32)( -sp );
	
	f64 srsp = sr*sp;
	f64 crsp = cr*sp;
	
	M[4] = (f32)( srsp*cy-cr*sy );
	M[5] = (f32)( srsp*sy+cr*cy );
	M[6] = (f32)( sr*cp );
	
	M[8] = (f32)( crsp*cy+sr*sy );
	M[9] = (f32)( crsp*sy-sr*cy );
	M[10] = (f32)( cr*cp );
	definitelyIdentityMatrix=false;
}

// Returns the rotation, as set by setRotation(). 
inline vector3df matrix4::getRotationDegrees() const
{
	const matrix4 &mat = *this;
	
	f64 Y = -asin(mat(0,2));
	f64 C = cos(Y);
	Y *= RADTODEG64;
	
	f64 rotx, roty, X, Z;
	
	if (fabs(C)>0.0005f){
		rotx = mat(2,2) / C;
		roty = mat(1,2) / C;
		X = atan2( roty, rotx ) * RADTODEG64;
		rotx = mat(0,0) / C;
		roty = mat(0,1) / C;
		Z = atan2( roty, rotx ) * RADTODEG64;
	}else{
		X = 0.0f;
		rotx = mat(1,1);
		roty = -mat(1,0);
		Z = atan2( roty, rotx ) * RADTODEG64;
	}
	
	// fix values that get below zero
	// before it would set (!) values to 360
	// that where above 360:
	if (X < 0.0) X += 360.0;
	if (Y < 0.0) Y += 360.0;
	if (Z < 0.0) Z += 360.0;
	
	return vector3df((f32)X,(f32)Y,(f32)Z);
}

inline void matrix4::setInverseRotationRadians( const vector3df& rotation )
{
	f64 cr = cos( rotation.x );
	f64 sr = sin( rotation.x );
	f64 cp = cos( rotation.y );
	f64 sp = sin( rotation.y );
	f64 cy = cos( rotation.z );
	f64 sy = sin( rotation.z );
	
	M[0] = (f32)( cp*cy );
	M[4] = (f32)( cp*sy );
	M[8] = (f32)( -sp );
	
	f64 srsp = sr*sp;
	f64 crsp = cr*sp;
	
	M[1] = (f32)( srsp*cy-cr*sy );
	M[5] = (f32)( srsp*sy+cr*cy );
	M[9] = (f32)( sr*cp );
	
	M[2] = (f32)( crsp*cy+sr*sy );
	M[6] = (f32)( crsp*sy-sr*cy );
	M[10] = (f32)( cr*cp );
	definitelyIdentityMatrix=false;
}

*/