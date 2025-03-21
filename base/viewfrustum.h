#ifndef __VIEW_FRUSTUM_H_INCLUDED__
#define __VIEW_FRUSTUM_H_INCLUDED__

#include "plane3d.h"
#include "vector3d.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "line3d.h"

//Defines the view frustum. Thats the space viewed by the camera.
//The view frustum is enclosed by 6 planes. These six planes share
//four points. A bounding box around these four points is also stored in this structure.
struct SViewFrustum
{
	enum VFPLANES
	{
		VF_FAR_PLANE = 0,
		VF_NEAR_PLANE,
		VF_LEFT_PLANE,
		VF_RIGHT_PLANE,
		VF_BOTTOM_PLANE,
		VF_TOP_PLANE,
		VF_PLANE_COUNT
	};
	
	//Default Constructor
	SViewFrustum() {};
	
	//This constructor creates a view frustum based on a projection and/or view matrix.
	SViewFrustum(const matrix4& mat);
	void setFrom(const matrix4& mat);
	void setFromLine( line3df* lines );
	
	//the position of the camera
	vector3df cameraPosition;
	//all planes enclosing the view frustum.
	plane3df planes[VF_PLANE_COUNT];
	
	//transforms the frustum by the matrix
	//param mat: Matrix by which the view frustum is transformed.
	void transform(const matrix4 &mat);
	
	//바라보는 방향에서 먼곳의 네 꼭지점 구하기
	vector3df getFarLeftUp() const; 
	vector3df getFarLeftDown() const;
	vector3df getFarRightUp() const;
	vector3df getFarRightDown() const;
	void getPoints(vector3df *points) const;
	void getFrustumPoints( line3df line, double angle, vector3df* pt) const;
	
	// returns a bounding box enclosing the whole view frustum
	const aabbox3df &getBoundingBox() const;
	// recalculates the bounding box member based on the planes
	void recalculateBoundingBox();

	EIntersectionRelation3D classifyBoxRelation(const aabbox3df& box) const;

	bool classifyPointRelation(const vector3df& point) const;

	bool IntersectSphere(const vector3df& center, float radius )	const;


	// bouding box around the view frustum
	aabbox3df boundingBox;
	
	//! Hold a copy of important transform matrices
//	enum E_TRANSFORMATION_STATE_3
//	{
//		ETS_VIEW_PROJECTION_3 = video::ETS_PROJECTION + 1,
//		ETS_VIEW_MODEL_INVERSE_3,
//		ETS_CURRENT_3,
//		ETS_COUNT_3
//	};
	
//	matrix4 Matrices[ETS_COUNT_3];
//	void setTransformState( video::E_TRANSFORMATION_STATE state);
};


// transforms the frustum by the matrix
// \param Matrix by which the view frustum is transformed.
inline void SViewFrustum::transform(const matrix4 &mat)//행렬에 의한 시야 변환
{
	for (int i=0; i<VF_PLANE_COUNT; ++i)
		mat.transformPlane(planes[i]);
	
	mat.transformVect(cameraPosition);
	recalculateBoundingBox();
}

// returns the point which is on the far left upper corner inside the
inline vector3df SViewFrustum::getFarLeftUp() const{
	vector3df p;
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_TOP_PLANE],planes[VF_LEFT_PLANE], p);
	return p;
}
// returns the point which is on the far left bottom corner inside the
inline vector3df SViewFrustum::getFarLeftDown() const{
	vector3df p;
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_BOTTOM_PLANE],planes[VF_LEFT_PLANE], p);
	return p;
}
// returns the point which is on the far right top corner inside the
inline vector3df SViewFrustum::getFarRightUp() const{
	vector3df p;
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_TOP_PLANE],planes[VF_RIGHT_PLANE], p);
	return p;
}
// returns the point which is on the far right bottom corner inside the
inline vector3df SViewFrustum::getFarRightDown() const{
	vector3df p;
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_BOTTOM_PLANE],planes[VF_RIGHT_PLANE], p);
	return p;
}
// returns a bounding box enclosing the whole view frustum
inline const aabbox3df &SViewFrustum::getBoundingBox() const{//시야전체 박스 영역
	return boundingBox;
}
// recalculates the bounding box member based on the planes
inline void SViewFrustum::recalculateBoundingBox()
{
	boundingBox.reset ( cameraPosition );
	
	boundingBox.addInternalPoint(getFarLeftUp());
	boundingBox.addInternalPoint(getFarRightUp());
	boundingBox.addInternalPoint(getFarLeftDown());
	boundingBox.addInternalPoint(getFarRightDown());
}

// This constructor creates a view frustum based on a projection and/or view matrix.
inline SViewFrustum::SViewFrustum(const matrix4& mat){setFrom ( mat );}

//프로젝션과 뷰 메트릭스 곱하기.
//SViewFrustum frustum(TransformationMatrix[ETS_PROJECTION] * TransformationMatrix[ETS_VIEW]);
//This constructor creates a view frustum based on a projection and/or view matrix.
inline void SViewFrustum::setFrom(const matrix4& mat)
{
	// left clipping plane
	planes[VF_LEFT_PLANE].Normal.x	= mat[3 ] + mat[0];
	planes[VF_LEFT_PLANE].Normal.y	= mat[7 ] + mat[4];
	planes[VF_LEFT_PLANE].Normal.z	= mat[11] + mat[8];
	planes[VF_LEFT_PLANE].D		= mat[15] + mat[12];
	
	// right clipping plane
	planes[VF_RIGHT_PLANE].Normal.x = mat[3 ] - mat[0];
	planes[VF_RIGHT_PLANE].Normal.y = mat[7 ] - mat[4];
	planes[VF_RIGHT_PLANE].Normal.z = mat[11] - mat[8];
	planes[VF_RIGHT_PLANE].D =        mat[15] - mat[12];
	
	// top clipping plane
	planes[VF_TOP_PLANE].Normal.x = mat[3 ] - mat[1];
	planes[VF_TOP_PLANE].Normal.y = mat[7 ] - mat[5];
	planes[VF_TOP_PLANE].Normal.z = mat[11] - mat[9];
	planes[VF_TOP_PLANE].D =        mat[15] - mat[13];
	
	// bottom clipping plane
	planes[VF_BOTTOM_PLANE].Normal.x = mat[3 ] + mat[1];
	planes[VF_BOTTOM_PLANE].Normal.y = mat[7 ] + mat[5];
	planes[VF_BOTTOM_PLANE].Normal.z = mat[11] + mat[9];
	planes[VF_BOTTOM_PLANE].D =        mat[15] + mat[13];
	
	// far clipping plane
	planes[VF_FAR_PLANE].Normal.x = mat[3 ] - mat[2];
	planes[VF_FAR_PLANE].Normal.y = mat[7 ] - mat[6];
	planes[VF_FAR_PLANE].Normal.z = mat[11] - mat[10];
	planes[VF_FAR_PLANE].D =        mat[15] - mat[14];
/*
CString str;
str.Format("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f\n%f",
		   mat[3 ], mat[7 ],mat[11],   mat[15],
		   mat[2 ], mat[6 ],mat[10],   mat[14],
		   planes[VF_FAR_PLANE].Normal.x,
		   planes[VF_FAR_PLANE].Normal.y,
		   planes[VF_FAR_PLANE].Normal.z,
		   planes[VF_FAR_PLANE].D,
		   planes[VF_FAR_PLANE].Normal.getLengthSQ(),
		   - reciprocal_squareroot ( planes[VF_FAR_PLANE].Normal.getLengthSQ()) );
//AfxMessageBox(str);
*/	
	// near clipping plane
	planes[VF_NEAR_PLANE].Normal.x = mat[2];
	planes[VF_NEAR_PLANE].Normal.y = mat[6];
	planes[VF_NEAR_PLANE].Normal.z = mat[10];
	planes[VF_NEAR_PLANE].D =        mat[14];
	
	u32 i;// normalize normals
	for ( i=0; i != 6; ++i)
	{
		const f32 len = - reciprocal_squareroot ( planes[i].Normal.getLengthSQ() );
		planes[i].Normal *= len;
		planes[i].D *= len;
	}
	
	recalculateBoundingBox();// make bounding box
}

inline void SViewFrustum::setFromLine( line3df* lines )
{
			/*	frustum 저장 방식
                  /7--------/6
                 /  |      / |
                /   |     /  |
                3---------2  |
                |   5- - -| -4
                |  /      |  /
                |/        | /
                1---------0/ 
			*/
	if(lines==0) return;
	int i;
	vector3df normal[4],v1,v2,n,pt[8];

	for( i=0;i<4;i++ )	pt[i]=lines[i].start + lines[i].getVector().normalize();
	for( i=4;i<8;i++ )	pt[i]=lines[i-4].end;

	n = (pt[1] - pt[0]).crossProduct(pt[2] - pt[0]).normalize();
	planes[VF_NEAR_PLANE].Normal = n;
	planes[VF_NEAR_PLANE].D = -(n.x * pt[0].x + n.y * pt[0].y + n.z * pt[0].z);

	n = (pt[6] - pt[4]).crossProduct(pt[5] - pt[4]).normalize();
	planes[VF_FAR_PLANE].Normal = n;
	planes[VF_FAR_PLANE].D = -(n.x * pt[4].x + n.y * pt[4].y + n.z * pt[4].z);

	n = (pt[3] - pt[2]).crossProduct(pt[6] - pt[2]).normalize();
	planes[VF_TOP_PLANE].Normal = n;
	planes[VF_TOP_PLANE].D = -(n.x * pt[2].x + n.y * pt[2].y + n.z * pt[2].z);

	n = (pt[4] - pt[0]).crossProduct(pt[1] - pt[0]).normalize();
	planes[VF_BOTTOM_PLANE].Normal = n;
	planes[VF_BOTTOM_PLANE].D = -(n.x * pt[0].x + n.y * pt[0].y + n.z * pt[0].z);

	n = (pt[5] - pt[1]).crossProduct(pt[3] - pt[1]).normalize();
	planes[VF_LEFT_PLANE].Normal = n;
	planes[VF_LEFT_PLANE].D = -(n.x * pt[1].x + n.y * pt[1].y + n.z * pt[1].z);

	n = (pt[2] - pt[0]).crossProduct(pt[4] - pt[0]).normalize();
	planes[VF_RIGHT_PLANE].Normal = n;
	planes[VF_RIGHT_PLANE].D = -(n.x * pt[0].x + n.y * pt[0].y + n.z * pt[0].z);

	recalculateBoundingBox();// make bounding box

}

inline void SViewFrustum::getPoints(vector3df *points) const
{
			/*	frustum 저장 방식
                  /7--------/6
                 /  |      / |
                /   |     /  |
                3---------2  |
                |   5- - -| -4
                |  /      |  /
                |/        | /
                1---------0/ 
			*/
	planes[VF_NEAR_PLANE].getIntersectionWithPlanes( planes[VF_BOTTOM_PLANE],planes[VF_RIGHT_PLANE], points[0]);//getFarRightDown() 
	planes[VF_NEAR_PLANE].getIntersectionWithPlanes( planes[VF_BOTTOM_PLANE],planes[VF_LEFT_PLANE], points[1]);//getFarLeftDown() 
	planes[VF_NEAR_PLANE].getIntersectionWithPlanes( planes[VF_TOP_PLANE],planes[VF_RIGHT_PLANE], points[2]);//getFarRightUp() 
	planes[VF_NEAR_PLANE].getIntersectionWithPlanes( planes[VF_TOP_PLANE],planes[VF_LEFT_PLANE], points[3]);//getFarLeftUp()
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_BOTTOM_PLANE],planes[VF_RIGHT_PLANE], points[4]);//getFarRightDown() 
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_BOTTOM_PLANE],planes[VF_LEFT_PLANE], points[5]);//getFarLeftDown() 
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_TOP_PLANE],planes[VF_RIGHT_PLANE], points[6]);//getFarRightUp() 
	planes[VF_FAR_PLANE].getIntersectionWithPlanes(	planes[VF_TOP_PLANE],planes[VF_LEFT_PLANE], points[7]);//getFarLeftUp()

}

inline void SViewFrustum::getFrustumPoints( line3df line, double angle, vector3df* pt) const
{
	float dist,t;
	double radian = angle*DEGTORAD64;
	vector3df cross,upvec;	// 눈 방향 벡터
	vector3df axis= line.getVector();//to - from; 
	axis.normalize();
	vector3df rot_axis = axis.crossProduct(vector3df(0.0f,1.0f,0.0f));
	rot_axis.normalize();

	upvec=axis.crossProduct(rot_axis);
	cross=upvec.crossProduct(axis).normalize();

	double length = line.getVector().getLength();
	for(int i = 0; i < 8; i++)	{
		//dist = (i & 0x4) ? 1000 : 1;
		dist = (i & 0x4) ? (float)length : 1;
		pt[i] = axis * dist;
		t = float(dist * tan(radian/2));
		t = (i & 0x2) ? t : -t;
		pt[i] += upvec * t;
		t = float(dist * tan(radian/2)); // take into account screen proportions
		t = (i & 0x1) ? -t : t;
		pt[i] += cross * t;
		pt[i] = line.start + pt[i];
	}
}

inline EIntersectionRelation3D SViewFrustum::classifyBoxRelation(const aabbox3df& box) const
{
	vector3df edges[8];
	box.getEdges(edges);
			
	int	OrCodes = 0 , AndCodes = ~0 ;
	DWORD zones[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	int i,j;
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 6; j++)
		{
			if (planes[j].Normal.dotProduct(edges[i]) + planes[j].D > 0) zones[i] |= (DWORD)pow(2.0,j);
		}
	}
	OrCodes=zones[0] | zones[1] | zones[2] | zones[3] | zones[4] | zones[5] | zones[6] | zones[7];
	// if all of the corners are outside of the boundaries
	// this node is excluded, so stop traversing
	AndCodes=zones[0] & zones[1] & zones[2] & zones[3] & zones[4] & zones[5] & zones[6] & zones[7];

	if (AndCodes) // All the points are outside one of the frustum planes.
		return ISREL3D_FRONT;//시야에서 완전히 벗어남
	else if (OrCodes == 0) 
		return ISREL3D_BACK;//시야내부에 완전히 들어옴
	
	return ISREL3D_CLIPPED;//시야 박스와 교차됨


/*	u32 i;
	for ( i=0; i < 6; i++)	{
		if(box.classifyPlaneRelation(planes[i])==ISREL3D_FRONT)   
			return ISREL3D_FRONT;
		if(box.classifyPlaneRelation(planes[i])==ISREL3D_CLIPPED) 
			return ISREL3D_CLIPPED;
	}
	return ISREL3D_BACK;
*/
}

inline bool SViewFrustum::classifyPointRelation(const vector3df& point) const // soo 20100105
{
	for(int i = 0; i < 6; i++)
	{
		// 점이 평면의 외부에 있으면 안보임
		if (planes[i].Normal.dotProduct(point) + planes[i].D > 0) return false;
	}

	return true;
}

/// Test if a sphere intersects or is completely inside the frustum.
inline bool SViewFrustum::IntersectSphere(const vector3df& center, float radius )	const
{
	for(int i = 0; i < 6; i++)
	{
//		if(planes[i].Normal.dotProduct(center) + planes[i].D < -radius)// More than 1 radius outside the plane = outside
		if(planes[i].Normal.dotProduct(center) + planes[i].D > radius) return false;
	}

	return true;
}

/*
inline void SViewFrustum::setTransformState( video::E_TRANSFORMATION_STATE state)
{
	switch ( state )
	{
	case video::ETS_VIEW:
		Matrices[ETS_VIEW_PROJECTION_3].setbyproduct_nocheck (	Matrices[ video::ETS_PROJECTION],
			Matrices[ video::ETS_VIEW]
			);
		Matrices[ETS_VIEW_MODEL_INVERSE_3] = Matrices[ video::ETS_VIEW];
		Matrices[ETS_VIEW_MODEL_INVERSE_3].makeInverse();
		break;
		
	case video::ETS_WORLD:
		Matrices[ETS_CURRENT_3].setbyproduct (  Matrices[ ETS_VIEW_PROJECTION_3 ],
			Matrices[ video::ETS_WORLD]	);
		break;
	default:
		break;
	}
}
*/

#endif

