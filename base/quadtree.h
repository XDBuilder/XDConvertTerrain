#ifndef __QUADTREE_H_INCLUDED__
#define __QUADTREE_H_INCLUDED__
#include "rect2d.h"
#include "vector2d.h"
#include "xdtypes.h"
#include <vector>
#include <map>
#include <math.h>


#include "polygon.h"
#define SAFE_DELETE(p)		{ if(p) { delete (p);     (p)=NULL; } }
using namespace std;
const double    TLooseK = 1.2;//원래 2 였는데 너무 많은 쿼드 요청..
class QuadData
{
	//SHPHandle m_shp;	// 데이터가 들어가 있는 파일 핸들
public:
	QuadData() {
		m_shpIndex=-1;
		m_entityPos=-1;
		m_dust=-1;
		m_speed=-1;
		m_rect = rect2dd(0,0,0,0);
	}
	virtual ~QuadData() {} //(42)
	short m_shpIndex; // QTREE 데이터가 들어가 있는 파일 핸들을 가리키는 인덱스(2)
	int		  m_entityPos;// shape 파일에서 객체 위치(4)
    float	  m_dust;	// 미세 먼지
	float     m_speed; // 차량 속독
	rect2dd	  m_rect; //(32)
	//int		  m_id; // 객체 id
};

class QNODE
{
public:
	unsigned long	m_nodeid;		//quad node아이디.
	QNODE*			m_parent;		//상위 노드(사각형 ) 
	QNODE*			m_child[2][2];//하위 노드들  
	double			m_cx, m_cy;		//노드 중점 
	unsigned char	m_depth;		//노드의 단계 
	bool			m_isData;		//데이타가 존재유무


	map<int,QuadData*> m_objects;//노드에 포함되는 오브젝트를 가리키는 인덱스 리스트
	vector<QuadData*> m_objs;

	QNODE(unsigned long id,QNODE* p, double x, double y, int d) { //초기화 
		m_nodeid=id;
		m_parent = p; //상위
		m_depth = d;	//현재의 깊이 
		m_cx = x;		//사각형의 중점
		m_cy = y; 
		m_isData=false;
		m_parent = 0;
		m_objects.clear();
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				if (m_child[i][j]) m_child[i][j] = 0;
			}
		}
	}
	
	~QNODE() { //메모리 제거 
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				m_objects.clear();
				if (m_child[i][j]) delete m_child[i][j];
			}
		}
	}
	
};
class QTREE
{
public:

	short m_shpFileCount; // 파일 갯수 , 256 넘으면 오류
	QNODE	*m_root;		//쿼드트리 루트
	QNODE	*m_rt;		//쿼드트리 루트
	rect2dd	m_rect;
	double m_worldSize; //쿼드트리 영역가로길이
	unsigned char m_treeDepth; //256 레벨 까지
	int m_tot_count;
	unsigned long m_tot_quad;
	char 			m_valueKind; 	// 먼지 : 0, 차량속도 : 1
public:
	QTREE() {
		m_shpFileCount = 0;
		m_root = NULL;
		m_worldSize=0.0;
		m_treeDepth=0;
		m_tot_count=0;
		m_tot_quad=0;
		m_valueKind = 0; //먼지 
	}
	~QTREE(){
		if(m_root) {
			delete m_root;
			m_root=NULL;
		}
	}
	unsigned long calcQuad(int nLevel, int nColumn, int nRow);
	unsigned long GetQuadNodeID(double cx,double cz,unsigned char depth);
	void calQuadBox();
	// Insert the given object into the tree given by qnode.
	// Returns the depth of the node the object was placed in.
	void QuadInsert(QNODE* q,QuadData *qData);
	void makeRootAndQuadTreeDepth(double x1,double y1,double x2, double y2);
	void MakeQuadTreeLandUse(char* oriShpName,char *destPath,double minx1,double miny1,double maxx1,double maxy1);
	void MakeQuadTreeBuildings(char* oriShpName,char *destPath,double minx1,double miny1,double maxx1,double maxy1);
	rect2dd GetQNodeRect(QNODE* node);
	float QuadSelectPoint(QNODE* q,double xx,double yy);
	float pickObject(double xx,double yy,QuadData* qdata);
	void quadPrintf(QNODE *q,FILE *f);
	void setQuadValue(char kind);

};
#endif
