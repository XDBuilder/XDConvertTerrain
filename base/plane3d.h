#ifndef __PLANE_3D_H_INCLUDED__
#define __PLANE_3D_H_INCLUDED__

#include "xdmath.h"
#include "vector3d.h"


//Enumeration for intersection relations of 3d objects
enum EIntersectionRelation3D 
{
	ISREL3D_FRONT = 0,
	ISREL3D_BACK,
	ISREL3D_PLANAR,
	ISREL3D_SPANNING,
	ISREL3D_CLIPPED
};

template <class T>
class plane3d
{
	public:
		// member variables
		vector3d<T> Normal;		// normal vector
		T		 D;				// distance from origin

		// Constructors
		plane3d(): Normal(0,1,0) { recalculateD(vector3d<T>(0,0,0)); };
		plane3d(const vector3d<T>& MPoint, const vector3d<T>& Normal) : Normal(Normal) { recalculateD(MPoint); };
		plane3d(T px, T py, T pz, T nx, T ny, T nz) : Normal(nx, ny, nz) { recalculateD(vector3d<T>(px, py, pz)); };
		plane3d(const plane3d<T>& other) : Normal(other.Normal), D(other.D) {};
		plane3d(const vector3d<T>& point1, const vector3d<T>& point2, const vector3d<T>& point3) { setPlane(point1, point2, point3); };

		// operators
		inline bool operator==(const plane3d<T>& other) const { return (D==other.D && Normal==other.Normal);};
		inline bool operator!=(const plane3d<T>& other) const { return !(D==other.D && Normal==other.Normal);};

		// functions
		void setPlane(const vector3d<T>& point, const vector3d<T>& nvector)
		{
			Normal = nvector;
			Normal.normalize();
			recalculateD(point);
		}

		void setPlane(const vector3d<T>& nvect, T d)
		{
			Normal = nvect;
			D = d;
		}

		void setPlane(const vector3d<T>& point1, const vector3d<T>& point2, const vector3d<T>& point3)
		{
			// creates the plane from 3 memberpoints
			Normal = (point2 - point1).crossProduct(point3 - point1);
			Normal.normalize();

			recalculateD(point1);
		}

		// Returns an intersection with a 3d line.점에서 특정방향으로 쐇을때 평면에 교차하는지
		// \param lineVect: Vector of the line to intersect with.
		// \param linePoint: Point of the line to intersect with.
		// \param outIntersection: Place to store the intersection point, if there is one.
		// \return Returns true if there was an intersection, false if there was not.
		bool getIntersectionWithLine(const vector3d<T>& linePoint, const vector3d<T>& lineVect,
							  				vector3d<T>& outIntersection) const
		{
			T t2 = Normal.dotProduct(lineVect);

			if (t2 == 0) return false;

			T t =- (Normal.dotProduct(linePoint) + D) / t2;			
			outIntersection = linePoint + (lineVect * t);
			return true;
		}

		//Returns where on a line between two points an intersection with this plane happened. 라인과 평면이 교차했을때 라인 길이 대 교차잠점 비율..
		//Only useful if known that there is an intersection.
		//\param linePoint1: Point1 of the line to intersect with.
		//\param linePoint2: Point2 of the line to intersect with.
		//\return Returns where on a line between two points an intersection with this plane happened.
		//For example, 0.5 is returned if the intersection happened exactly in the middle of the two points. 리턴값이 0.5이면 교차점이 라인 중간..
		f32 getKnownIntersectionWithLine(const vector3d<T>& linePoint1,
										 const vector3d<T>& linePoint2) const
		{
			vector3d<T> vect = linePoint2 - linePoint1;
			f32 t2 = (f32)Normal.dotProduct(vect);
			return (f32)-((Normal.dotProduct(linePoint1) + D) / t2);
		}
		// Returns an intersection with a 3d line, limited between two 3d points.
		// \param linePoint1: Point 1 of the line.교차하는 점이 두점 사이 값인지 아닌지.
		// \param linePoint2: Point 2 of the line.
		// \param outIntersection: Place to store the intersection point, if there is one.
		// \return Returns true if there was an intersection, false if there was not.
		bool getIntersectionWithLimitedLine(const vector3d<T>& linePoint1, 
					const vector3d<T>& linePoint2, vector3d<T>& outIntersection) const
		{
			return (getIntersectionWithLine(linePoint1, linePoint2 - linePoint1, outIntersection) &&
					outIntersection.isBetweenPoints(linePoint1, linePoint2));
		}

		//Classifies the relation of a point to this plane. 점이 어느 위치에 있는지 분류
		//ISREL3D_FRONT	: 점이 평면의 앞쪽에 있다.
		//ISREL3D_BACK	: 점이 평며의 뒤쪽에 있다.
		//ISREL3D_PLANAR: 점이 평면 위에 있다.
		EIntersectionRelation3D classifyPointRelation(const vector3d<T>& point) const
		{
			const T d = Normal.dotProduct(point) + D;

			if (d < -ROUNDING_ERROR_32)	return ISREL3D_FRONT;

			if (d > ROUNDING_ERROR_32)	return ISREL3D_BACK;

			return ISREL3D_PLANAR;
		}

		// Recalculates the distance from origin by applying a new member point to the plane.
		void recalculateD(const vector3d<T>& MPoint){	D = - MPoint.dotProduct(Normal); }

		// Returns a member point of the plane.
		vector3d<T> getMemberPoint() const	{	return Normal * -D;	}

		// Tests if there is a intersection between this plane and another
		// \return Returns true if there is a intersection.
		bool existsInterSection(const plane3d<T>& other) const	{//다른 평면과 교차하는지.
			vector3d<T> cross = other.Normal.crossProduct(Normal);
			return cross.getLength() > ROUNDING_ERROR_32;
		}

		// Intersects this plane with another.다른 두 평면이 교차했을때 점벡터와 방향 벡터(라인).
		// \return Returns true if there is a intersection, false if not.
		bool getIntersectionWithPlane(const plane3d<T>& other, vector3d<T>& outLinePoint,
				vector3d<T>& outLineVect) const
		{
			f64 fn00 = Normal.getLength();
			f64 fn01 = Normal.dotProduct(other.Normal);
			f64 fn11 = other.Normal.getLength();
			f64 det = fn00*fn11 - fn01*fn01;

			if (fabs(det) < ROUNDING_ERROR_64 )	return false;

			det = 1.0 / det;
			f64 fc0 = (fn11*-D + fn01*other.D) * det;
			f64 fc1 = (fn00*-other.D + fn01*D) * det;

			outLineVect = Normal.crossProduct(other.Normal);
			outLinePoint = Normal*(f32)fc0 + other.Normal*(f32)fc1;
			return true;
		}

		// Returns the intersection point with two other planes if there is one.세 평면이 교차하는 점
		bool getIntersectionWithPlanes(const plane3d<T>& o1, 
				const plane3d<T>& o2, vector3d<T>& outPoint) const
		{
			vector3d<T> linePoint, lineVect;
			if (getIntersectionWithPlane(o1, linePoint, lineVect))
				return o2.getIntersectionWithLine(linePoint, lineVect, outPoint);

			return false;
		}

		// Returns if the plane is front of backfacing. Note that this only
		// works if the normal is Normalized.//점이 평면 앞에 있는지 뒤에 있는지.
		// \param lookDirection: Look direction.
		// \return Returns true if the plane is front facing, which mean it would be visible, and false if it is backfacing.
		bool isFrontFacing(const vector3d<T>& lookDirection) const 
		{
			const f32 d = Normal.dotProduct(lookDirection);
			return F32_LOWER_EQUAL_0 ( d );
		}

		// Returns the distance to a point.  Note that this only works if the normal is Normalized.
		// 점과 가장 가까운 거리
		T getDistanceTo(const vector3d<T>& point) const
		{
			return point.dotProduct(Normal) + D;
		}
	
};
	
typedef plane3d<f32> plane3df;// Typedef for f32 3d plane.
typedef plane3d<f64> plane3dd;// Typedef for f64 3d plane.

#endif

