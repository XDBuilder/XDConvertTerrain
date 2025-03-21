#ifndef __RECT2D_H_INCLUDED__
#define __RECT2D_H_INCLUDED__

#include "xdtypes.h"
#include "line2d.h"
#include "vector3d.h"
#include "dimension2d.h"
#include "position2d.h"
#include "aabbox3d.h"

template <class T>
class rect2d
{
	public:
		T ix,iy,ax,ay;

		rect2d() : ix(0), iy(0), ax(0), ay(0){};

		rect2d(T x, T y, T x2, T y2)
				: ix(x), iy(y), ax(x2), ay(y2) {};

		rect2d(const position2d<T>& upperLeft, const position2d<T>& lowerRight)
			: ix(upperLeft.x), iy(upperLeft.y), ax(lowerRight.x), ay(lowerRight.y) {};

		rect2d(const position2d<T>& pos, const dimension2d<T>& size)
			: ix(pos.x), iy(pos.y), ax(pos.x + size.Width), ay(pos.y +  + size.Height) {};

		rect2d(const vector2d<T>& v, const vector2d<T>& v2)
				: ix(v.x), iy(v.y), ax(v2.x), ay(v2.y) {};
		rect2d(const vector3d<T>& v, const vector3d<T>& v2)
				: ix(v.x), iy(v.z), ax(v2.x), ay(v2.z) {};

		rect2d(const rect2d<T>& other)
				: ix(other.ix), iy(other.iy), ax(other.ax), ay(other.ay) {};

		rect2d(const vector2d<T>& pos, const T& sx, const T& sy)
				: ix(pos.x), iy(pos.y), ax(pos.x+sx), ay(pos.y+sy) {};

		rect2d(const aabbox3d<T>& box)
			: ix(box.Min.x), iy(box.Min.z), ax(box.Max.x), ay(box.Max.z) {};


		rect2d<T> operator+(const vector2d<T>& pos) const	{//이동
			rect2d<T> ret(*this);
			ret.ix += pos.x; ret.iy += pos.y;
			ret.ax += pos.x; ret.ay += pos.y;
			return ret;
		}
		const rect2d<T>& operator+=(const vector2d<T>& pos) {
			ix += pos.x; iy += pos.y;
			ax += pos.x; ay += pos.y;
			return *this;
		}

		rect2d<T> operator-(const vector2d<T>& pos) const	{
			rect2d<T> ret(*this);
			ret.ix -= pos.x; ret.iy -= pos.y;
			ret.ax -= pos.x; ret.ay -= pos.y;
			return ret;
		}
		const rect2d<T>& operator-=(const vector2d<T>& pos) {//이동
			ix -= pos.x; iy -= pos.y;
			ax -= pos.x; ay -= pos.y;
			return *this;
		}
		const rect2d<T>& expand(const T& size ){//size 만큼 확장
			ix-=size; iy-=size; ax+=size; ay+=size;
			return *this;
		}

		bool operator == (const rect2d<T>& other) const {
			return (ix == other.ix && ax == other.ax && iy == other.iy && ay == other.ay);
		}
		bool operator != (const rect2d<T>& other) const {
			return (ix != other.ix || ax != other.ax || iy != other.iy || ay != other.ay);
		}

		const rect2d<T>& operator = (const rect2d<T>& other)
		{
			ix = other.ix; ax = other.ax;
			iy = other.iy; ay = other.ay;
			return *this;
		}

		bool isPointInside(const T& x,const T& y) const {//점이 내부에 있는지 검사
			return (ix <= x &&	iy<= y && ax >= x && ay >= y);
		}
		bool isPointInside2(const T& x,const T& y,const T& delta) const {//점이 내부에 있는지 검사
			return ( (ix+delta) < x &&	(iy+delta)< y && (ax-delta) > x && (ay-delta) > y);
		}
		bool isPointInside(const vector2d<T>& pos) const	{
			return isPointInside(pos.x,pos.y);
		}
		// soo 20100226
		bool isRectCollided(const rect2d<T>& other) const //두 영역이 교차하는지 검사(intersect)
		{
			bool bx=true,by=true;
			if(ix <= other.ix) bx = ax >= other.ix;
			if(ix >= other.ix) bx = ix <= other.ax;
			if(iy <= other.iy) by = ay >= other.iy;
			if(iy >= other.iy) by = iy <= other.ay;
			return bx && by;
		}

		bool isRectContained(const rect2d<T>& other) const //영역을 포함하는지 검사(contain)
		{
			return ( ix<=other.ix && iy<=other.iy && ax>=other.ax && ay>=other.ay); 
		}
		bool contain(const rect2d<T>& mr){
			return (this->ix<=mr.ix)&&(this->iy<=mr.iy)&&(this->ax>=mr.ax)&&(this->ay>=mr.ay); 
		}
		bool intersect(const rect2d<T>& mr)
		{
			bool bx=true,by=true;
			if(this->ix < mr.ix) bx = this->ax > mr.ix;
			if(this->ix > mr.ix) bx = this->ix < mr.ax;
			if(this->iy < mr.iy) by = this->ay > mr.iy;
			if(this->iy > mr.iy) by = this->iy < mr.ay;
			return bx && by;
		}
		void repair(){// 영역 좌표가 뒤바뀐 경우 재조정
			if (ax < ix){ T t = ix;	ix = ax;  ax = t;	}
			if (ay < iy){ T t = iy;	iy = ay;  ay = t;	}
		}

		void clipAgainst(const rect2d<T>& other) //교차된 영역
		{
			if (other.ax < ax)	ax = other.ax;
			if (other.ay < ay)	ay = other.ay;
			if (other.ix > ix)	ix = other.ix;
			if (other.iy > iy)	iy = other.iy;
			// correct possible invalid rect2d resulting from clipping
			repair();
		}

		T getWidth() const { return ax-ix;}
		T getHeight() const { return ay-iy;}

		bool isValid() const{ //영역 좌표가 안 맞다..
			T xd = ax - ix;
			T yd = ay - iy;
			return !(xd < 0 || yd < 0 || (xd == 0 && yd == 0));
		}
		vector2d<T> getCenter() const	{//중심점
			return vector2d<T>((ix + ax) / 2, (iy + ay) / 2);
		}
		// returns extend of the box
		vector2d<T> getExtent() const	{ return vector2d<T>( ax - ix , ay - iy );	}//가로지르는 벡터 // soo 20090323

		void addInternalRect(const rect2d<T>& b){
			addInternalPoint(b.ax,b.ay);
			addInternalPoint(b.ix,b.iy);
		}

		// Adds a point to the rectangle, causing it to grow bigger, 
		void addInternalPoint(const vector2d<T>& p){ addInternalPoint(p.x, p.y);}
		void addInternalPoint(const vector3d<T>& p){ addInternalPoint(p.x, p.z);}

		void addInternalPoint(T x, T y)
		{
			if (x<ix) ix = x;	if (y<iy) iy = y;
			if (x>ax) ax = x;	if (y>ay) ay = y;
		}
		// Returns the dimensions of the rectangle
		dimension2d<T> getSize() const
		{
			return dimension2d<T>(getWidth(), getHeight());
		}

		int intersectWithLine(const line2d<T>& l, vector2d<T> out[])
		{
			int i=0;
			vector2d<T> vec;
			line2d<T> line;
			line=line2d<T>(vector2d<T>(ix,iy),vector2d<T>(ix,ay));
			if(l.intersectWith( line, vec)) out[i++]=vec;
			line=line2d<T>(vector2d<T>(ix,ay),vector2d<T>(ax,ay));
			if(l.intersectWith( line, vec)) out[i++]=vec;
			line=line2d<T>(vector2d<T>(ax,ay),vector2d<T>(ax,iy));
			if(l.intersectWith( line, vec)) out[i++]=vec;
			line=line2d<T>(vector2d<T>(ax,iy),vector2d<T>(ax,iy));
			if(l.intersectWith( line, vec)) out[i++]=vec;
			return i;
		}


};

typedef rect2d<f32> rect2df;// Typedef for a f32 rect.
typedef rect2d<f64> rect2dd;// Typedef for a f64 rect.
typedef rect2d<s32> rect2di;// Typedef for an integer rect.

#endif
