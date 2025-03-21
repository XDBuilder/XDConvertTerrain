#ifndef __LINE_2D_H_INCLUDED__
#define __LINE_2D_H_INCLUDED__

#include "xdtypes.h"
#include "vector2d.h"

template <class T>
class line2d
{
	public:
		// member variables==============================================================================
		vector2d<T> start;
		vector2d<T> end;

		line2d() : start(0,0), end(1,1) {};
		line2d(f32 xa, f32 ya, f32 xb, f32 yb) : start(xa, ya), end(xb, yb) {};
		line2d(const vector2d<T>& start, const vector2d<T>& end) : start(start), end(end) {};
		line2d(const line2d<T>& other) : start(other.start), end(other.end) {};

		// operators==============================================================================
		line2d<T> operator+(const vector2d<T>& point) const { return line2d<T>(start + point, end + point); };
		line2d<T>& operator+=(const vector2d<T>& point) { start += point; end += point; return *this; };

		line2d<T> operator-(const vector2d<T>& point) const { return line2d<T>(start - point, end - point); };
		line2d<T>& operator-=(const vector2d<T>& point) { start -= point; end -= point; return *this; };

		bool operator==(const line2d<T>& other) const { return (start==other.start && end==other.end) || (end==other.start && start==other.end);};
		bool operator!=(const line2d<T>& other) const { return !(start==other.start && end==other.end) || (end==other.start && start==other.end);};

		// functions==============================================================================
		void setLine(const T& xa, const T& ya, const T& xb, const f32& yb){start.set(xa, ya); end.set(xb, yb);}
		void setLine(const vector2d<T>& nstart, const vector2d<T>& nend){start.set(nstart); end.set(nend);}
		void setLine(const line2d<T>& line){start.set(line.start); end.set(line.end);}

		f64 getLength() const { return start.getDistanceFrom(end); };//라인 길이
		f32 getLengthSQ() const { return start.getDistanceFromSQ(end); };//길이 제곱
		vector2d<T> getMiddle() const	{	return (start + end) * (f32)0.5; }//중간점
		vector2d<T> getVector() const { return vector2d<T>(start.x - end.x, start.y - end.y); };//방향벡터

		// Tests if this line intersects with another line. 다른 라인과 교차되는지 테스트
		//! \param l: Other line to test intersection with.
		//! \param out: If there is an intersection, the location of the intersection will be stored in this vector.
		//! \return Returns true if there is an intersection, false if not.
		bool intersectWith(const line2d<T>& l, vector2d<T>& out) const
		{
			bool found=false;

/*			// 이거 잘 안된다.(라인이 90 도로 직교하는 경우 안됨)
			T a1,a2,b1,b2,bb;

			// calculate slopes, deal with infinity
			if (end.x-start.x == 0) 	b1 = (f32)1e+10;
			else b1 = (end.y-start.y)/(end.x-start.x);

			if (l.end.x-l.start.x == 0)	b2 = (f32)1e+10;
			else b2 = (l.end.y-l.start.y)/(l.end.x-l.start.x);

			// calculate position
			a1 = start.y   - b1 *  start.x;
			a2 = l.start.y - b2 * l.start.x;
			bb = (b1-b2); if (bb == 0)	bb = (f32)1e+10;
//			out.x = - (a1-a2)/(b1-b2);//0으로 나누면 안돼..
			out.x = - (a1-a2)/bb;
			out.y = a1 + b1*out.x;

*/			// RTT 라인 끊김 현상 수정 soo 20090619
			double r;
			double denominator, numerator;
			
			if(end.x == l.start.x && end.y == l.start.y) out=end;
			
			numerator = ((start.y-l.start.y)*(l.end.x-l.start.x) - (start.x-l.start.x)*(l.end.y-l.start.y));  
			denominator = ((end.x-start.x)*(l.end.y-l.start.y) - (end.y-start.y)*(l.end.x-l.start.x)); 
			if(denominator==0) denominator=1e+10;
			
			r = numerator/denominator;
			
			out.x = (start.x + r*(end.x-start.x));
			out.y = (start.y + r*(end.y-start.y));
			
			// did the lines cross?
			if ((start.x-out.x) *(out.x-end.x)	 >= -ROUNDING_ERROR_32 &&
				(l.start.x-out.x)*(out.x-l.end.x)>= -ROUNDING_ERROR_32 &&
				(start.y-out.y)  *(out.y-end.y)  >= -ROUNDING_ERROR_32 &&
				(l.start.y-out.y)*(out.y-l.end.y)>= -ROUNDING_ERROR_32 )
			{
				found = true;
			}
			return found;
		}

		// Returns unit vector of the line.단위 벡터 구함
		vector2d<T> getUnitVector()
		{
			T len = (f32)(1.0 / getLength());
			return vector2d<T>((end.x - start.x) * len, (end.y - start.y) * len);
		}

		f64 getAngleWith(const line2d<T>& l)
		{
			vector2d<T> vect = getVector();
			vector2d<T> vect2 = l.getVector();
			return vect.getAngleWith(vect2);
		}

		// Tells us if the given point lies to the left, right, or on the direction of the line
		// return Returns 0 if the point is on the line	 <0 if to the left, or >0 if to the right.
		T getPointOrientation(const vector2d<T>& point)
		{
			return ( (end.x   - start.x) * (point.y - start.y) - 
					 (point.x - start.x) * (end.y   - start.y) );
		}

		bool isPointOnLine(const vector2d<T>& point)//라인 선상에 있는지 검사
		{
			T d = getPointOrientation(point);
			return (d == 0 && point.isBetweenPoints(start, end));
		}

		// Returns if the given point is between start and end of the line
		// Assumes that the point is already somewhere on the line.
		bool isPointBetweenStartAndEnd(const vector2d<T>& point) const
		{
			return point.isBetweenPoints(start, end);
		}

		// Returns the closest point on this line to a point
		vector2d<T> getClosestPoint(const vector2d<T>& point) const
		{
			vector2d<T> c = point - start;
			vector2d<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			T t = v.dotProduct(c);

			if (t < (T)0.0) return start;
			if (t > d) return end;

			v *= t;
			return start + v;
		}

};
	
typedef line2d<f32> line2df;// Typedef for a f32 line.
typedef line2d<f64> line2dd;// Typedef for a f64 line.
typedef line2d<s32> line2di;// Typedef for an integer line.

#endif

/* 라인 교차점 구하기 xdworld v 2.0
VECTOR2D getLineIntersection(VECTOR2D a, VECTOR2D b, VECTOR2D c, VECTOR2D d)
{
  VECTOR2D p;
  double r;
  double denominator, numerator;

  if(b.x == c.x && b.y == c.y) return(b);

  numerator = ((a.y-c.y)*(d.x-c.x) - (a.x-c.x)*(d.y-c.y));  
  denominator = ((b.x-a.x)*(d.y-c.y) - (b.y-a.y)*(d.x-c.x));  

  r = numerator/denominator;

  p.x = (a.x + r*(b.x-a.x));
  p.y = (a.y + r*(b.y-a.y));

  return(p);
}
*/
