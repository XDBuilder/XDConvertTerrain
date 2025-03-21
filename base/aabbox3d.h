#ifndef __AABBOX_3D_H_INCLUDED__
#define __AABBOX_3D_H_INCLUDED__

#include "xdmath.h"
#include "plane3d.h"
#include "line3d.h"

//Axis aligned bounding box in 3d dimensional space.
//Has some useful methods used with occlusion culling or clipping.
template <class T>
class aabbox3d
{
	public:
		// member variables
		vector3d<T>	Min;
		vector3d<T>	Max;

		// Constructors==============================================================================
		aabbox3d(): Min(-1,-1,-1), Max(1,1,1) {};
		aabbox3d(const aabbox3d<T>& other): Min(other.Min), Max(other.Max) {};
		aabbox3d(const vector3d<T>& min, const vector3d<T>& max): Min(min), Max(max) {};
		aabbox3d(const vector3d<T>& init): Min(init), Max(init) {};
		aabbox3d(T minx, T miny, T minz, T maxx, T maxy, T maxz)
						: Min(minx, miny, minz), Max(maxx, maxy, maxz) {};

		// operators==============================================================================
		inline bool operator==(const aabbox3d<T>& other) const { return (Min == other.Min && other.Max == Max);};
		inline bool operator!=(const aabbox3d<T>& other) const { return !(Min == other.Min && other.Max == Max);};

		// functions==============================================================================
		// 박스에 점을 더한다. 박스 바깥 점이면 박스가 더 커지겠지?
		void addInternalPoint(const vector3d<T>& p) {	addInternalPoint(p.x, p.y, p.z); }

		// Adds an other bounding box to the bounding box, causing it to grow bigger,
		// if the box is outside of the box
		// param b: Other bounding box to add into this box.
		void addInternalBox(const aabbox3d<T>& b){
			addInternalPoint(b.Max);
			addInternalPoint(b.Min);
		}
		// Resets the bounding box.
		void reset(T x, T y, T z)	{//박스 초기화
			Max.set(x,y,z);
			Min = Max;
		}
		// Resets the bounding box.
		void reset(const aabbox3d<T>& initValue) {	*this = initValue;	}

		// Resets the bounding box.
		void reset(const vector3d<T>& initValue){
			Max = initValue;
			Min = initValue;
		}
		// 박스 바깥 점이면 박스가 더 커지겠지...
		// Adds a point to the bounding box, causing it to grow bigger, if point is outside of the box
		void addInternalPoint(T x, T y, T z)
		{
			if (x>Max.x) Max.x = x;
			if (y>Max.y) Max.y = y;
			if (z>Max.z) Max.z = z;

			if (x<Min.x) Min.x = x;
			if (y<Min.y) Min.y = y;
			if (z<Min.z) Min.z = z;
		}

		bool isPointInside(const vector3d<T>& p) const //3차원 점이 박스 안에 있는지 검사
		{
			return (p.x >= Min.x && p.x <= Max.x &&
					p.y >= Min.y && p.y <= Max.y &&
					p.z >= Min.z && p.z <= Max.z);
		};

		bool isPointTotalInside(const vector3d<T>& p) const //3차원 점이 완전히 박스 안에 있는지 검사
		{
			return (p.x > Min.x && p.x < Max.x &&
				p.y > Min.y && p.y < Max.y &&
				p.z > Min.z && p.z < Max.z);
		};

		bool intersectsWithBox(const aabbox3d<T>& other) const//다른 박스와 교차하는지
		{
			return (Min <= other.Max && Max >= other.Min);
		}

		bool isFullInside(const aabbox3d<T>& other) const//다른 박스가 포한되는지
		{
			return Min >= other.Min && Max <= other.Max;
		}

		bool intersectsWithLine(const line3d<T>& line) const//라인 과 교차하는지 검사
		{
//			return intersectsWithLine(line.getMiddle(), line.getVector().normalize(), (T)(line.getLength() * 0.5));//이게 잘 안된다..
			return intersectsWithLine(line.start, line.getVector().normalize(), (T)(line.getLength() ));
		}

		//Tests if the box intersects with a line //위 intersectsWithLine 함수를 위한 함수. 라인과 교차.
		//return Returns true if there is an intersection and false if not.
		bool intersectsWithLine(const vector3d<T>& linemiddle, const vector3d<T>& linevect, T halflength) const
		{
			const vector3d<T> e = getExtent() * (T)0.5;
			const vector3d<T> t = getCenter() - linemiddle;
			T r;

			if ((fabs(t.x) > e.x + halflength * fabs(linevect.x)) || 
				(fabs(t.y) > e.y + halflength * fabs(linevect.y)) ||
				(fabs(t.z) > e.z + halflength * fabs(linevect.z)) )
				return false;

			r = e.y * (T)fabs(linevect.z) + e.z * (T)fabs(linevect.y);
			if (fabs(t.y*linevect.z - t.z*linevect.y) > r )
				return false;

			r = e.x * (T)fabs(linevect.z) + e.z * (T)fabs(linevect.x);
			if (fabs(t.z*linevect.x - t.x*linevect.z) > r )
				return false;

			r = e.x * (T)fabs(linevect.y) + e.y * (T)fabs(linevect.x);
			if (fabs(t.x*linevect.y - t.y*linevect.x) > r)
				return false;

			return true;
		}

		// 평면과의 관계를 분류하는 것(앞에 있는지 뒤에 있는지 걸치는지) Classifies a relation with a plane.
		// return Returns ISREL3D_FRONT if the box is in front of the plane,
		// ISREL3D_BACK if the box is back of the plane, and
		// ISREL3D_CLIPPED if is on both sides of the plane.
		EIntersectionRelation3D classifyPlaneRelation(const plane3d<T>& plane) const
		{
			vector3d<T> nearPoint(Max);
			vector3d<T> farPoint(Min);

			if (plane.Normal.x > (T)0)
			{
				nearPoint.x = Min.x;
				farPoint.x = Max.x;
			}

			if (plane.Normal.y > (T)0)
			{
				nearPoint.y = Min.y;
				farPoint.y = Max.y;
			}

			if (plane.Normal.z > (T)0)
			{
				nearPoint.z = Min.z;
				farPoint.z = Max.z;
			}

			if (plane.Normal.dotProduct(nearPoint) + plane.D > (T)0)
				return ISREL3D_FRONT;

			if (plane.Normal.dotProduct(farPoint) + plane.D > (T)0)
				return ISREL3D_CLIPPED;

			return ISREL3D_BACK;

		}

		// returns center of the bounding box
		vector3d<T> getCenter() const	{ return (Min + Max) / 2; }//중점
		// returns extend of the box
		vector3d<T> getExtent() const	{ return Max - Min;	}//가로지르는 벡터

		// stores all 8 edges of the box into a array
		// param edges: Pointer to array of 8 edges
		void getEdges(vector3d<T> *edges) const
		{
			const vector3d<T> middle = getCenter();
			const vector3d<T> diag = middle - Max;

			/*
			Edges are stored in this way:
			Hey, am I an ascii artist, or what? :) niko.
                  /4--------/0
                 /  |      / |
                /   |     /  |
                6---------2  |
                |   5- - -| -1
                |  /      |  /
                |/        | /
                7---------3/ 
			*/

			edges[0].set(middle.x + diag.x, middle.y + diag.y, middle.z + diag.z);
			edges[1].set(middle.x + diag.x, middle.y - diag.y, middle.z + diag.z);
			edges[2].set(middle.x + diag.x, middle.y + diag.y, middle.z - diag.z);
			edges[3].set(middle.x + diag.x, middle.y - diag.y, middle.z - diag.z);
			edges[4].set(middle.x - diag.x, middle.y + diag.y, middle.z + diag.z);
			edges[5].set(middle.x - diag.x, middle.y - diag.y, middle.z + diag.z);
			edges[6].set(middle.x - diag.x, middle.y + diag.y, middle.z - diag.z);
			edges[7].set(middle.x - diag.x, middle.y - diag.y, middle.z - diag.z);
		}


		bool isEmpty() const {	return Min.equals ( Max );	}//박스크기가 거의 0 이다.

		// repairs the box, if for example Min and Max are swapped.
		void repair(){//박스 좌표가 뒤 바뀌었을 경우 다시 고침
			T t;
			if (Min.x > Max.x)	{ t=Min.x; Min.x = Max.x; Max.x=t; }
			if (Min.y > Max.y)	{ t=Min.y; Min.y = Max.y; Max.y=t; }
			if (Min.z > Max.z)	{ t=Min.z; Min.z = Max.z; Max.z=t; }
		}

		// Calculates a new interpolated bounding box.
		// param other: other box to interpolate between
		// param d: value between 0.0f and 1.0f.
		aabbox3d<T> getInterpolated(const aabbox3d<T>& other, T d) const
		{
			T inv = 1.0f - d;
			return aabbox3d<T>((other.Min*inv) + (Min*d),(other.Max*inv) + (Max*d));
		}

		aabbox3d<T> getmovebox( const vector3d<T> move ) const
		{
			return aabbox3d<T>( Min+move , Max+move );
		}
		
		aabbox3d<T>& expand(const T& size ){//size 만큼 확장
			vector3d<T> sizeV = vector3d<T>(size,size,size);
			Min -= sizeV;
			Max += sizeV;
			return *this;
		}

		void getAs6Values(T* array) const
		{
			if(!array) return;
			array[0] = Min.x; 
			array[1] = Min.y; 
			array[2] = Min.z;
			array[3] = Max.x; 
			array[4] = Max.y; 
			array[5] = Max.z;
		}


};

typedef aabbox3d<f32> aabbox3df;// Typedef for a f32 3d bounding box.
typedef aabbox3d<s32> aabbox3di;// Typedef for an integer 3d bounding box.
typedef aabbox3d<f64> aabbox3dd;// Typedef for an double 3d bounding box.

#endif

