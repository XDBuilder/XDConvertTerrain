#ifndef __TRIANGLE_3D_H_INCLUDED__
#define __TRIANGLE_3D_H_INCLUDED__

#include "vector3d.h"
#include "line3d.h"
#include "plane3d.h"
#include "aabbox3d.h"
#include "tri_tri_intersection.h"

template <class T>
class triangle3d  
{
public:
	//the three points of the triangle
	vector3d<T> pointA; 
	vector3d<T> pointB; 
	vector3d<T> pointC; 

	//Determines if the triangle is totally inside a bounding box.
	//param box : Box to check.
	//return Returns true if the triangle is withing the box, and false otherwise.
	bool isTotalInsideBox(const aabbox3d<T>& box) const
	{
		return (box.isPointInside(pointA) && 
				box.isPointInside(pointB) &&
				box.isPointInside(pointC));
	}
	
	bool operator==(const triangle3d<T>& other) const { return other.pointA==pointA && other.pointB==pointB && other.pointC==pointC; }
	bool operator!=(const triangle3d<T>& other) const { return other.pointA!=pointA || other.pointB!=pointB || other.pointC!=pointC; }
	
	//Returns the closest point on a triangle to a point on the same plane.
	//param p: Point which must be on the same plane as the triangle.
	vector3d<T> closestPointOnTriangle(const vector3d<T>& p) const
	{
		
		vector3d<T> rab = line3df(pointA, pointB).getClosestPoint(p);
		vector3d<T> rbc = line3df(pointB, pointC).getClosestPoint(p);
		vector3d<T> rca = line3df(pointC, pointA).getClosestPoint(p);
		
		T d1 = (T)rab.getDistanceFrom(p);
		T d2 = (T)rbc.getDistanceFrom(p);
		T d3 = (T)rca.getDistanceFrom(p);
		
		if (d1 < d2) return d1 < d3 ? rab : rca;
		
		return d2 < d3 ? rbc : rca;
	}
	
	//점이 삼각형 내부에 있는지 검사한다.
	bool isPointInside(const vector3d<T>& p) const
	{
		return (isOnSameSide(p, pointA, pointB, pointC) &&
				isOnSameSide(p, pointB, pointA, pointC) &&
				isOnSameSide(p, pointC, pointA, pointB));
	}
	
	// Returns if a point is inside the triangle. This method is an implementation
	// of the example used in a paper by Kasper Fauerby original written by Keidy from Mr-Gamemaker.
	// \param p: Point to test. Assumes that this point is already on the plane of the triangle.
	// \return Returns true if the point is inside the triangle, otherwise false.
	bool isPointInsideFast(const vector3d<T>& p) const
	{
		vector3d<T> f = pointB - pointA;
		vector3d<T> g = pointC - pointA;
		
		f32 a = f.dotProduct(f);
		f32 b = f.dotProduct(g);
		f32 c = g.dotProduct(g);
		
		f32 ac_bb = (a*c)-(b*b);
		vector3d<T> vp = p - pointA;
		
		f32 d = vp.dotProduct(f);
		f32 e = vp.dotProduct(g);
		f32 x = (d*c)-(e*b);
		f32 y = (e*a)-(d*b);
		f32 z = x+y-ac_bb;
		
		return (( ((u32&)z)& ~(((u32&)x)|((u32&)y))) & 0x80000000)!=0;
	}
	
	bool isOnSameSide(const vector3d<T>& p1, const vector3d<T>& p2, const vector3d<T>& a, const vector3d<T>& b) const
	{
		vector3d<T> bminusa = b - a;
		vector3d<T> cp1 = bminusa.crossProduct(p1 - a);
		vector3d<T> cp2 = bminusa.crossProduct(p2 - a);
		return (cp1.dotProduct(cp2) >= ROUNDING_ERROR_32);
	}

	//아래 getIntersectionWithLine 함수에서 교차점이 라인 사이에 있으면  true 아니면 false
	bool getIntersectionWithLimitedLine(const line3d<T>& line,
								vector3d<T>& outIntersection) const
	{
		return getIntersectionWithLine(line.start, line.getVector(), outIntersection) &&
			   outIntersection.isBetweenPoints(line.start, line.end);
	}
	
	//아래 getIntersectionOfPlaneWithLine 함수에서 교차점이 삼각형에 포함되면 true 아니면 false
	bool getIntersectionWithLine(const vector3d<T>& linePoint,
		const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
	{
		if (getIntersectionOfPlaneWithLine(linePoint, lineVect, outIntersection))
			return isPointInside(outIntersection);
		
		return false;			
	}
	
	//3차원 라인과 삼각형을 이루는 평면(무한평면) 사이의 교차점을 계산한다. 
	//교차점이 있으면 true 를 평면과 나란하면 false
	bool getIntersectionOfPlaneWithLine(const vector3d<T>& linePoint,
		const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
	{
		const vector3d<T> normal = getNormal().normalize();
		T t2;
		
		if ( iszero ( t2 = normal.dotProduct(lineVect) ) )
			return false;
		
		T d = pointA.dotProduct(normal);
		T t = -(normal.dotProduct(linePoint) - d) / t2;
		outIntersection = linePoint + (lineVect * t);
		return true;
	}

	//삼각형의 법선벡터를 구한다. Normalize 되지 않았다.
	vector3d<T> getNormal() const
	{
		return (pointB - pointA).crossProduct(pointC - pointA);
	}
	
	//점이 삼각형의 앞면에 있는지 검사한다. Returns if the triangle is front of backfacing.
	//param lookDirection: Look direction.
	//return Returns true if the plane is front facing, which mean it would
	//be visible, and false if it is backfacing.
	bool isFrontFacing(const vector3d<T>& lookDirection) const
	{
		vector3d<T> n = getNormal();
		n.normalize();
		return n.dotProduct(lookDirection) <= 0.0f;
	}
	
	plane3d<T> getPlane() const {//삼각형을 이루는 무한 평면을 계산한다.
		return plane3d<T>(pointA, pointB, pointC);
	}
	
	f64 getArea() const	{//삼각형 면적
		return (pointB - pointA).crossProduct(pointC - pointA).getLength() * 0.5;
	}
	
	void set(const vector3d<T>& a, const vector3d<T>& b, const vector3d<T>& c){//삼각형 점 설정
		pointA = a;
		pointB = b;
		pointC = c;
	}
	
};
	
typedef triangle3d<f32> triangle3df;// Typedef for a f32 3d triangle.
typedef triangle3d<f64> triangle3dd;// Typedef for a f64 3d triangle.

#endif

