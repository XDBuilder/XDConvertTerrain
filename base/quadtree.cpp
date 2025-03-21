#include "quadtree.h"
#include <stdio.h>     /* printf, fgets */
#include <string.h>
#include <stdlib.h>     /* atof */
#include <math.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h> 

#ifdef _WIN32
#include <windows.h>
#include <libloaderapi.h> // for dynamic loading
#include <io.h>           // for access() and other POSIX functions
#include <direct.h>       // for getcwd, chdir
#else
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#endif

//#include "../XDEarthAPI.h"
const float NODATA_VALUE=999;
const int STR_LEN=512;

void QTREE::setQuadValue(char kind)
{
	m_valueKind = kind;
}
bool FitsInRect(rect2dd rect, double cx, double cy, double HalfSize)
{
	vector2dd center=rect.getCenter();
	double radius = sqrt(rect.getWidth()*rect.getWidth()+rect.getHeight()*rect.getHeight());

	if (center.x - radius < cx - HalfSize ||
	    center.x + radius > cx + HalfSize ||
	    center.y - radius < cy - HalfSize ||
	    center.y + radius > cy + HalfSize)
	{
		return false;
	} else {
		return true;
	}
}
unsigned long QTREE::calcQuad(int nLevel, int nColumn, int nRow)
{
	if(nLevel < 1) return 0L;
	else{ 
		unsigned long imsi = 1;
		long lNumRows = 1;
		for ( int i = 0; i < nLevel ; i ++ ){
			imsi = imsi * 4;
			lNumRows *= 2;
		}		
		return ( imsi - 1L ) / 3L + nRow*(lNumRows) + nColumn ;
	}
}
unsigned long QTREE::GetQuadNodeID(double cx,double cz,unsigned char depth)
{
	int col= static_cast<int>((cx-m_rect.ix)/(m_worldSize / (2 << depth)));
	int row= static_cast<int>((cz-m_rect.iy)/(m_worldSize / (2 << depth)));
	return calcQuad(depth+1,col,row);
}
void QTREE::calQuadBox()
{	//영역 재계산
	vector2dd c = m_rect.getCenter();
	double subx   = m_rect.getWidth()/2;
	double suby   = m_rect.getHeight()/2;
	double half  = subx > suby ? subx : suby;
	m_rect=rect2dd(c.x-half,c.y-half,c.x+half,c.y+half);
}

void QTREE::makeRootAndQuadTreeDepth(double x1,double y1,double x2, double y2)
{
	/*
	if(m_root){//쿼드가 이미 있으면 모두 지운다.
		delete m_root;
		m_root=NULL;
	}*/
	if(m_root) return; //최초에 한번만 실행
	m_rect = rect2dd( x1, y1, x2,  y2);
	calQuadBox();//정사각형의 박스로 만든다.
	m_worldSize=m_rect.getWidth();

	if(m_worldSize==0) m_worldSize=400/TLooseK;

	double cx = (m_rect.ix+m_rect.ax)/2;
	double cy = (m_rect.iy+m_rect.ay)/2;

	float max=1; 
	max = static_cast<float>(0.002777778);
	m_treeDepth=0;
	while(1){//쿼드 깊이 구하기.(server랑 통일 해야 된다.)
//		double quadSize=pow(2,quadDepth)*(m_maxHidden+m_minHidden)/2;//Looses Quad Tree
		double quadSize=pow(2.0,(double)m_treeDepth);//*max;//Looses Quad Tree
//		double quadSize=pow(2,quadDepth)*4000/TLooseK;//Looses Quad Tree
//		double quadSize=pow(2,quadDepth)*1000/TLooseK;

		if(m_worldSize<=quadSize) break;
		m_treeDepth++;
	}
	//CString str;
	//str.Format("m_treeDepth = %d",m_treeDepth);
	//AfxMessageBox(str);
	m_root=new QNODE(0,NULL, cx,cy,0);
}
// Insert the given object into the tree given by qnode.
// Returns the depth of the node the object was placed in.
void QTREE::QuadInsert(QNODE* q,QuadData  *qData)
{
	if(q==NULL) {
		return ;
	}

	int	i, j;
	// Check child nodes to see if object fits in one of them.
//	TLooseK=2.0;
	if (q->m_depth < m_treeDepth) {
		double	quadSize = TLooseK * m_worldSize / pow(2.0,(double)q->m_depth);
		double	halfSize = quadSize / 2;
		double  offset = halfSize/2;

		vector2dd  ocenter=qData->m_rect.getCenter();
		i = (ocenter.x <= q->m_cx) ? 0 : 1; //LooseK>1일경우 (loose quad) 
		j = (ocenter.y <= q->m_cy) ? 0 : 1;
		rect2dd looseQuad(q->m_cx-(i+1)%2*halfSize,q->m_cy-(j+1)%2*halfSize,q->m_cx+(i)%2*halfSize,q->m_cy+(j)%2*halfSize);
		if(looseQuad.isRectContained(qData->m_rect)) {
			if (q->m_child[i][j] == 0) {
				double	cx = q->m_cx + ((i == 0) ? -offset : offset);
				double	cy = q->m_cy + ((j == 0) ? -offset : offset);
				unsigned long quad_id=GetQuadNodeID(cx,cy,q->m_depth);
				q->m_child[i][j] = new QNODE(quad_id,q, cx, cy, q->m_depth + 1);
				q->m_child[i][j]->m_parent = q;
				m_tot_quad++;
			}
			return QuadInsert(q->m_child[i][j],qData);
		}
		
	}
	pair<int,QuadData *>p(m_tot_count,qData);
	q->m_objects.insert(p);
	m_tot_count++;
	q->m_isData=true;
}

void QTREE::quadPrintf(QNODE *q,FILE *f)
{
	if(q==NULL) return;
	int i,j;
	if(q->m_depth < m_treeDepth+1) {
		if(q->m_isData) fprintf(f,"%d,%d\n",q->m_depth,q->m_objects.size());
		for(j=0;j<2;j++) {
			for(i=0;i<2;i++) {
				if(q->m_child[i][j]!=0) {
					quadPrintf(q->m_child[i][j],f);
				}
			}
		}
	}
	return;
}


rect2dd QTREE::GetQNodeRect(QNODE* node)
{
	if(node==NULL) return rect2dd(0,0,0,0);
	double	HalfSize;
	HalfSize = TLooseK *m_worldSize / (2 << node->m_depth);//TLooseK = 1.2
	rect2dd qRect = rect2dd(node->m_cx-HalfSize,node->m_cy-HalfSize,
						node->m_cx+HalfSize,node->m_cy+HalfSize);
	return qRect;
}
//type : 0 => 먼지, type : 1 => 차량속도
float QTREE::QuadSelectPoint(QNODE* q,double xx,double yy) // 토지피복도 종류 혹은 건물의 층수를 리턴함.
{

	int	j, i;
	//점이 쿼드에 포함되지 않으면 나간다.
	rect2dd qRect=GetQNodeRect(q);
	//검사 위치가 현재 쿼드 안에 있는지 확인한다.
	if(qRect.isPointInside(xx,yy)==false) return NODATA_VALUE;
	float ret;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			if (q->m_child[i][j]) {
				ret = QuadSelectPoint(q->m_child[i][j],xx,yy);
				if(ret!=NODATA_VALUE) return ret;
			}
		}
	}
	//if(q->m_objects.size()==0) return NODATA_VALUE;	
	if(q->m_isData==false) return NODATA_VALUE;	
	// 본인 쿼드를 먼저 검사한다.
	QuadData* obj;
	float contain=NODATA_VALUE;
	map<int,QuadData*>::iterator it=q->m_objects.begin();
	while(it != q->m_objects.end()) {
		obj=it->second;
		contain=NODATA_VALUE;
		if(obj){
			contain=pickObject( xx,yy,obj );//objIdx 해당되는 shape 파일 데이터를 읽어서 xx,yy 좌표로 교차하는지 살펴본다.
			if(contain!=NODATA_VALUE) {
				if(m_valueKind == 0) 
					return obj->m_dust;
				else 
					return obj->m_speed;
			}
		}
		it++;
	}
	return NODATA_VALUE;
}


float QTREE::pickObject(double xx,double yy,QuadData* qdata)
{
	//if(qdata->m_value != 5 ) return -1;
	if(qdata->m_rect.isPointInside(vector2dd(xx,yy))==false) return NODATA_VALUE;//영역에 포함되지 않으면 

	return qdata->m_dust;

}
/*
void QTREE::GetTerrainData()
{
	HANDLE f=CreateFile("f:\\terrain.dat",GENERIC_READ, FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	char msg[256];
	memset(msg,0,256);

	int i,j;
	int row,col,blkcol,blkrow;
	DWORD dwTemp;
	double minx,miny,maxy;
	double cellsize,nodata;
	ReadFile(f,&minx,sizeof(double),&dwTemp,NULL);//좌하단x
	ReadFile(f,&miny,sizeof(double),&dwTemp,NULL);//좌하단y
	ReadFile(f,&cellsize,sizeof(double),&dwTemp,NULL);//해상도
	ReadFile(f,&col,sizeof(int),&dwTemp,NULL);//실제 가로 셀 수
	ReadFile(f,&row,sizeof(int),&dwTemp,NULL);// 실제 세로 셀수
	ReadFile(f,&blkcol,sizeof(int),&dwTemp,NULL);//블럭단위로 계산된 픽셀 가로 수
	ReadFile(f,&blkrow,sizeof(int),&dwTemp,NULL);//블럭단위로 계산된 픽셀 세로 수(실제 세로 셀 수 보다 크다)
	ReadFile(f,msg,256,&dwTemp,NULL);
	//cellsize=5.0;
	col = blkcol;
	row = blkrow;
	maxy = miny + (row-1)*cellsize;

	double BytesPerLine = sizeof(float)*(double)col;

	double minx1,miny1,maxx1,maxy1; //추출 영역

	minx1 = 261098; miny1 = 288554;
	maxx1 = 263288; maxy1 = 290377;

//	minx1 = 161212; miny1 = 288133;
//	maxx1 = 163536; maxy1 = 290238;

	minx1 = 412934; miny1 = 322372;
	maxx1 = 419313; maxy1 = 327276;

	minx1 = 409753; miny1 = 319578;
	maxx1 = 421229; maxy1 = 329824;

	minx1 = 406128; miny1 = 323283;
	maxx1 = 412572; maxy1 = 327985;

	int iCol = (maxx1-minx1)/cellsize;
	int iRow = (maxy1-miny1)/cellsize;
	int stCol = (minx1-minx)/cellsize;
	//int stRow = (maxy-maxy1)/cellsize;
	//int stRow = (maxy1-miny)/cellsize;
	int stRow = (miny1-miny)/cellsize;
	double stRowOffset = BytesPerLine*stRow;
	double stColOffset = stCol*sizeof(float);
	float *dt = new float[iCol];

	FILE *ft=fopen("f:\\height.txt","wt");
	fprintf(ft,"nrows %d\n",iRow);
	fprintf(ft,"ncols %d\n",iCol);
	fprintf(ft,"xllcorner %f\n",minx1);
	fprintf(ft,"yllcorner %f\n",miny1);
	fprintf(ft,"cellsize %f\n",cellsize);
	fprintf(ft,"nodata_value %d\n",999);

//	CProgressWnd wndProgress(NULL, " Progress");
//	wndProgress.GoModal();
//	wndProgress.SetRange(0, iRow );
//	wndProgress.SetText("Shape file get pick data....");

	double offset = 0;

	//for(j=iRow-1;j>=0;j--) {
	//	offset = stRowOffset + stColOffset+ j*BytesPerLine+32;
	for(j=0;j<iRow;j++) {
//		wndProgress.StepIt();
//		wndProgress.PeekAndPump();
		if(wndProgress.Cancelled()==true) break;
		offset = stRowOffset + stColOffset + j*BytesPerLine+296;
		SetBigFilePointer(f,offset,FILE_BEGIN);
		ReadFile(f,dt,sizeof(float)*iCol,&dwTemp,NULL);
		for(i=0;i<iCol;i++) {
			fprintf(ft,"%7.1f",dt[i]);
			//if(dt[i]>0) {
			//	AfxMessageBox("ff");
			//}
		}
		fprintf(ft,"\n");
	}
	SAFE_DELETE(dt);

	fclose(ft);
	CloseHandle(f);
}
void QTREE::SetBigFilePointer(HANDLE fh,double offset,DWORD dwMoveMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = LONGLONG(offset);
	SetFilePointerEx(fh,li,&li,dwMoveMethod);
	//SetFilePointer(fh,li.LowPart,&li.HighPart,dwMoveMethod);
}
*/
